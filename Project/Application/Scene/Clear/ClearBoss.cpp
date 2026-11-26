#include "ClearBoss.h"
#include "../../Config/ClearConfig.h"
#include "../../Object/ModelColor.h"
#include <cmath>

using namespace RyoEngine;

namespace App
{
	//6つのパーツを読みこんで、足元を親にして仰向けに寝かせる
	void ClearBoss::Initialize()
	{
		//正面が上を向くようにXで倒して、頭を左へ向ける
		const float scale = ClearConfig::kBossScale;
		rootRotate_ = { -90.0f, 90.0f, 0.0f };
		const Vector3 feet = ClearConfig::kBossCenter + Vector3{ ClearConfig::kBossLength * 0.5f * scale, ClearConfig::kBossBackDepth * scale, 0.0f };
		root_.SetLocal({ { scale, scale, scale }, rootRotate_, feet });

		//パーツのモデルとまん中と仮の色
		struct PartSetup
		{
			const char* model;
			Vector3 center;
			Vector4 color;
			float radius;
		};
		const std::array<PartSetup, kPartCount> setups = { {
			{ ClearConfig::kBossBody, ClearConfig::kBodyCenter, ClearConfig::kBossBodyColor, 0.4f },
			{ ClearConfig::kBossHead, ClearConfig::kHeadCenter, ClearConfig::kBossHeadColor, 0.5f },
			{ ClearConfig::kBossLeftArm, ClearConfig::kLeftArmCenter, ClearConfig::kBossArmColor, 0.3f },
			{ ClearConfig::kBossRightArm, ClearConfig::kRightArmCenter, ClearConfig::kBossArmColor, 0.3f },
			{ ClearConfig::kBossLeftFeet, ClearConfig::kLeftFeetCenter, ClearConfig::kBossFeetColor, 0.3f },
			{ ClearConfig::kBossRightFeet, ClearConfig::kRightFeetCenter, ClearConfig::kBossFeetColor, 0.3f },
		} };
		for (size_t i = 0; i < kPartCount; ++i)
		{
			Part& part = parts_[i];
			part.center = setups[i].center;
			part.radius = setups[i].radius * scale;
			part.holder.SetParent(&root_);
			part.holder.SetLocal({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, part.center });
			part.model = Model::Create(setups[i].model);
			ModelColor::Apply(*part.model, setups[i].model);
			if (ClearConfig::kUseBossTint)
			{
				part.model->SetColor(setups[i].color);
			}
			part.model->SetParent(&part.holder);
			part.model->SetTranslate({ -part.center.x, -part.center.y, -part.center.z });
		}
		timer_ = 0.0f;
		isScattered_ = false;
	}

	//impactから外へパーツを吹っ飛ばす
	void ClearBoss::Scatter(const Vector3& impact)
	{
		if (isScattered_)
		{
			return;
		}
		isScattered_ = true;
		std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
		const float scale = ClearConfig::kBossScale;
		for (Part& part : parts_)
		{
			//今のワールドの位置から、親を外して自分で飛ぶ
			const Matrix4x4& world = part.holder.GetWorldMatrix();
			part.position = { world.m[3][0], world.m[3][1], world.m[3][2] };
			part.rotate = rootRotate_;
			part.holder.SetParent(nullptr);

			//当たった所から外へ、上へ高く、くるくる回る
			Vector3 away = part.position - impact;
			away.y = 0.0f;
			const float length = std::sqrt(away.x * away.x + away.z * away.z);
			if (length < 0.01f)
			{
				away = { unit(random_), 0.0f, unit(random_) };
			}
			else
			{
				away = away * (1.0f / length);
			}
			const float speed = ClearConfig::kScatterSpeed * (0.7f + 0.3f * unit(random_));
			//カメラの方へは飛ばさず、横と奥へ飛ばす
			part.velocity = { away.x * speed, ClearConfig::kScatterLift * (0.8f + 0.25f * unit(random_)), std::abs(away.z) * speed + 1.5f };
			part.spin = { ClearConfig::kScatterSpin * unit(random_), ClearConfig::kScatterSpin * unit(random_), ClearConfig::kScatterSpin * unit(random_) };
			part.holder.SetLocal({ { scale, scale, scale }, part.rotate, part.position });
		}
	}

	//倒れてふらふらか、バラバラに飛ぶのを1フレーム進める
	void ClearBoss::Update(float deltaTime)
	{
		timer_ += deltaTime;
		if (!isScattered_)
		{
			//目を回して頭がふらふら、体はすうすう
			Part& head = parts_[1];
			head.holder.SetLocal({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 10.0f * std::sin(timer_ * 5.0f), 0.0f }, head.center });
			const float breath = 1.0f + 0.03f * std::sin(timer_ * 2.5f);
			Transform root = root_.GetLocal();
			root.scale = { ClearConfig::kBossScale, ClearConfig::kBossScale, ClearConfig::kBossScale * breath };
			root_.SetLocal(root);
			root_.UpdateMatrix();
		}
		else
		{
			//重力で落ちて、床ではねて止まっていく
			const float scale = ClearConfig::kBossScale;
			for (Part& part : parts_)
			{
				part.velocity.y -= ClearConfig::kScatterGravity * deltaTime;
				part.position = part.position + part.velocity * deltaTime;
				part.rotate = part.rotate + part.spin * deltaTime;
				if (part.position.y < part.radius && part.velocity.y < 0.0f)
				{
					part.position.y = part.radius;
					part.velocity.y = -part.velocity.y * ClearConfig::kScatterBounce;
					part.velocity.x *= 0.6f;
					part.velocity.z *= 0.6f;
					part.spin = part.spin * 0.5f;
				}
				part.holder.SetLocal({ { scale, scale, scale }, part.rotate, part.position });
			}
		}
		for (Part& part : parts_)
		{
			part.holder.UpdateMatrix();
			part.model->TransferMatrix();
		}
	}

	//6つのパーツを描く
	void ClearBoss::Draw()
	{
		for (Part& part : parts_)
		{
			part.model->Draw();
		}
	}
}
