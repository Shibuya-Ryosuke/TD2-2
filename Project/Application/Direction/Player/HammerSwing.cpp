#include "HammerSwing.h"
#include "../../Config/DirectionConfig.h"
#include "../../Object/ModelColor.h"
#include "../../Object/Player/PlayerArms.h"
#include <algorithm>
#include <array>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kPi = 3.14159265f; //円周率
		constexpr float kDegToRad = kPi / 180.0f; //度からラジアンへ
		constexpr float kRadToDeg = 180.0f / kPi; //ラジアンから度へ
		constexpr float kMaxLiftDeg = 170.0f; //床にめり込むときに起こすいちばん大きい度数
		constexpr int32_t kLiftSearchCount = 20; //起こす度数を半分ずつしぼりこむ回数
		constexpr float kMinScale = 0.001f; //大きさが0で向きが取れなくならないための小さい倍率

		//行の長さで割って大きさを取り除いた回転だけの行列からエンジンと同じXYZの順の度数を出す
		Vector3 ToEulerDeg(const Matrix4x4& matrix, float scale)
		{
			const float sinY = std::clamp(-matrix.m[0][2] / scale, -1.0f, 1.0f);
			const float x = std::atan2(matrix.m[1][2], matrix.m[2][2]);
			const float y = std::asin(sinY);
			const float z = std::atan2(matrix.m[0][1], matrix.m[0][0]);
			return { x * kRadToDeg, y * kRadToDeg, z * kRadToDeg };
		}
	}

	//ハンマーを作って、プレイヤーの右手のボーンを大きくした腕の手へ動かした所につける
	void HammerSwing::Initialize(Model& body)
	{
		model_ = Model::Create(DirectionConfig::kHammerModel);
		ModelColor::Apply(*model_, DirectionConfig::kHammerModel);
		hand_ = body.GetJointTransform(DirectionConfig::kHammerJoint);
		if (hand_ != nullptr)
		{
			heldHand_.SetParent(hand_->GetParent());
		}
		model_->SetParent(&heldHand_);
		handGrow_ = 1.0f;
		isShown_ = false;
		scale_ = 0.0f;
	}

	//振り始めにポンっと出す
	void HammerSwing::Start()
	{
		isShown_ = true;
		scale_ = 0.0f;
	}

	//置き直したときなどにすぐ消す
	void HammerSwing::Hide()
	{
		isShown_ = false;
		scale_ = 0.0f;
	}

	//床にめり込むのを直すために柄の持つ所を中心にangle度起こしたときの右手から見たハンマーの行列
	Matrix4x4 HammerSwing::MakeLocalMatrix(float angle, float scale) const
	{
		//大きさを変えて、持つ所を原点に動かしてハンマーの横の軸で起こし、元に戻す
		const Vector3 grip = { 0.0f, DirectionConfig::kHammerGrip * scale, 0.0f };
		const Matrix4x4 scaling = MakeScaleMatrix({ scale, scale, scale });
		const Matrix4x4 toGrip = MakeTranslateMatrix({ -grip.x, -grip.y, -grip.z });
		const Matrix4x4 lift = MakeRotateZMatrix(angle * kDegToRad);
		const Matrix4x4 fromGrip = MakeTranslateMatrix(grip);

		//右手から見た決まった向きと位置に置く
		const Vector3& rotateDeg = DirectionConfig::kHammerRotateDeg;
		const Matrix4x4 hold = MakeRotateMatrix({ rotateDeg.x * kDegToRad, rotateDeg.y * kDegToRad, rotateDeg.z * kDegToRad });
		const Matrix4x4 offset = MakeTranslateMatrix(DirectionConfig::kHammerOffset);
		return scaling * toGrip * lift * fromGrip * hold * offset;
	}

	//その行列のときのハンマーの頭のいちばん低い高さ
	float HammerSwing::FindLowestHeadY(const Matrix4x4& local) const
	{
		const Matrix4x4 world = local * heldHand_.GetWorldMatrix();
		const Vector3& low = DirectionConfig::kHammerHeadMin;
		const Vector3& high = DirectionConfig::kHammerHeadMax;

		//頭の箱の8つの角のうちいちばん低い高さ
		float lowest = 0.0f;
		bool isFirst = true;
		for (int32_t corner = 0; corner < 8; ++corner)
		{
			Vector3 point = low;
			if ((corner & 1) != 0)
			{
				point.x = high.x;
			}
			if ((corner & 2) != 0)
			{
				point.y = high.y;
			}
			if ((corner & 4) != 0)
			{
				point.z = high.z;
			}
			const float y = TransformPoint(point, world).y;
			if (isFirst || y < lowest)
			{
				lowest = y;
				isFirst = false;
			}
		}
		return lowest;
	}

	//頭が床より下に行かないように起こす度数
	float HammerSwing::FindLiftAngle(float floorY) const
	{
		//そのままで床より上ならそのまま
		const float size = 1.0f / handGrow_;
		if (FindLowestHeadY(MakeLocalMatrix(0.0f, size)) >= floorY)
		{
			return 0.0f;
		}

		//少し回して高くなる向きに起こす
		const float probeDeg = 5.0f;
		float direction = 1.0f;
		if (FindLowestHeadY(MakeLocalMatrix(-probeDeg, size)) > FindLowestHeadY(MakeLocalMatrix(probeDeg, size)))
		{
			direction = -1.0f;
		}

		//床にちょうど乗る度数を半分ずつしぼりこむ
		float low = 0.0f;
		float high = kMaxLiftDeg;
		for (int32_t step = 0; step < kLiftSearchCount; ++step)
		{
			const float middle = (low + high) * 0.5f;
			if (FindLowestHeadY(MakeLocalMatrix(middle * direction, size)) >= floorY)
			{
				high = middle;
			}
			else
			{
				low = middle;
			}
		}
		return high * direction;
	}

	//振っている間は大きくした腕の手について動き、終わったらしゅっと消える
	void HammerSwing::Update(bool isSwinging, float floorY, const PlayerArms& arms)
	{
		if (!isShown_ || hand_ == nullptr)
		{
			return;
		}

		//振っている間はポンっと大きくなり、終わったらしゅっと小さくなって消える
		const float deltaTime = GetDeltaTime();
		if (isSwinging)
		{
			scale_ = std::min(scale_ + deltaTime / DirectionConfig::kHammerPopTime, 1.0f);
		}
		else
		{
			scale_ -= deltaTime / DirectionConfig::kHammerHideTime;
			if (scale_ <= 0.0f)
			{
				Hide();
				return;
			}
		}

		//右手のボーンの今のフレームの位置を大きくした腕の手へ動かして、頭が床より下なら起こす
		const Transform hand = hand_->GetLocal();
		const Transform held = arms.ScaleRightHand(hand);
		heldHand_.SetLocal(held);
		heldHand_.UpdateMatrix();
		handGrow_ = 1.0f;
		if (std::abs(hand.scale.x) > kMinScale)
		{
			handGrow_ = held.scale.x / hand.scale.x;
		}
		const float liftDeg = FindLiftAngle(floorY);

		//ポンっと出る大きさをかけた右手から見た行列を位置と向きと大きさに分け、手が大きくなった分は小さくして元の大きさにする
		const float scale = std::max(scale_, kMinScale) / handGrow_;
		const Matrix4x4 local = MakeLocalMatrix(liftDeg, scale);
		model_->SetScale({ scale, scale, scale });
		model_->SetRotate(ToEulerDeg(local, scale));
		model_->SetTranslate({ local.m[3][0], local.m[3][1], local.m[3][2] });
		model_->TransferMatrix();
	}

	//出ている間だけ描く
	void HammerSwing::Draw()
	{
		if (!isShown_)
		{
			return;
		}
		model_->Draw();
	}
}
