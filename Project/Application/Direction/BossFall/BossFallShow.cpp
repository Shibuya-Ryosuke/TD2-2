#include "BossFallShow.h"
#include "BossFallParticles.h"
#include "BossFallPoses.h"
#include "../../Config/BossFallConfig.h"
#include <algorithm>
#include <cmath>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			const float clamped = std::clamp(t, 0.0f, 1.0f);
			return clamped * clamped * (3.0f - 2.0f * clamped);
		}

		//つぶれてからぽよんと戻るばねで、timeがマイナスなら0
		float Spring(float time, float amount)
		{
			if (time < 0.0f)
			{
				return 0.0f;
			}
			return amount * std::exp(-BossFallConfig::kSlamSpringDamping * time) * std::cos(BossFallConfig::kSlamSpringSpeed * time);
		}

		//fromからtoへ近い回り方でtだけ回した度数
		float LerpAngle(float from, float to, float t)
		{
			float delta = std::fmod(to - from + 540.0f, 360.0f) - 180.0f;
			if (delta < -180.0f)
			{
				delta += 360.0f;
			}
			return from + delta * t;
		}
	}

	//パーツと煙と光の輪を用意する
	void BossFallShow::Initialize()
	{
		rig_.Initialize();
		BossFallParticles::Initialize();
		state_ = State::Hidden;
	}

	//feetの位置でfaceYawDeg度を向いて、足元がなくなって「えっ」となる
	void BossFallShow::Begin(const Vector3& feet, float faceYawDeg)
	{
		feet_ = feet;
		faceYawDeg_ = faceYawDeg;
		yawDeg_ = faceYawDeg;
		state_ = State::Notice;
		timer_ = 0.0f;
		slamTimer_ = -1.0f;
		hitTimer_ = -1.0f;
		flash_ = 0.0f;
		Update(0.0f);
	}

	//じたばた落ち始める
	void BossFallShow::StartFlail()
	{
		noticeAtFlail_ = MakePose();
		state_ = State::Flail;
		timer_ = 0.0f;
	}

	//ドスンと叩きつけられて座りこみ、足元から煙を広げる
	void BossFallShow::Slam()
	{
		flailAtSlam_ = MakePose();
		state_ = State::Hurt;
		timer_ = 0.0f;
		slamTimer_ = 0.0f;
		BossFallParticles::SlamPuff(feet_);
	}

	//足場が当たってsquashだけむぎゅっとつぶれて白く光る
	void BossFallShow::Hit(float squash)
	{
		hitTimer_ = 0.0f;
		hitSquash_ = squash;
		flash_ = BossFallConfig::kHitFlash;
	}

	//yawDeg度の向きへ回りながら起き上がる
	void BossFallShow::BeginGetUp(float yawDeg)
	{
		getUpYawDeg_ = yawDeg;
		state_ = State::GetUp;
		timer_ = 0.0f;
	}

	//起き上がり終わったか
	bool BossFallShow::IsGetUpFinished() const
	{
		return state_ == State::GetUp && timer_ >= BossFallConfig::kGetUpTime;
	}

	//今の場面の姿勢
	BossPose BossFallShow::MakePose() const
	{
		if (state_ == State::Notice)
		{
			//立ち姿から「えっ」へ
			return LerpBossPose(BossFallPoses::StandPose(), BossFallPoses::NoticePose(timer_), Smooth(timer_ / BossFallConfig::kPoseBlendTime));
		}
		if (state_ == State::Flail)
		{
			//「えっ」からすぐにじたばたへ
			return LerpBossPose(noticeAtFlail_, BossFallPoses::FlailPose(timer_), Smooth(timer_ / BossFallConfig::kPoseBlendTime));
		}
		if (state_ == State::Hurt)
		{
			//一瞬で手足が投げ出されてから、頭を押さえて座りこむ
			const float blend = BossFallConfig::kPoseBlendTime;
			if (timer_ < blend)
			{
				return LerpBossPose(flailAtSlam_, BossFallPoses::SlamPose(), Smooth(timer_ / blend));
			}
			return LerpBossPose(BossFallPoses::SlamPose(), BossFallPoses::HurtSitPose(timer_), Smooth((timer_ - blend) / BossFallConfig::kHurtSettleTime));
		}
		//起き上がりは座りこみからいつもの立ち姿へ
		return LerpBossPose(BossFallPoses::HurtSitPose(timer_), BossFallPoses::StandPose(), Smooth(timer_ / BossFallConfig::kGetUpTime));
	}

	//姿勢とつぶれと光を進めて行列を確定する
	void BossFallShow::Update(float deltaTime)
	{
		if (state_ == State::Hidden)
		{
			return;
		}
		timer_ += deltaTime;
		if (slamTimer_ >= 0.0f)
		{
			slamTimer_ += deltaTime;
		}
		if (hitTimer_ >= 0.0f)
		{
			hitTimer_ += deltaTime;
		}
		flash_ *= std::exp(-BossFallConfig::kHitFlashDecay * deltaTime);

		//起き上がるときはいつもの向きへ回る
		if (state_ == State::GetUp)
		{
			yawDeg_ = LerpAngle(faceYawDeg_, getUpYawDeg_, Smooth(timer_ / BossFallConfig::kGetUpTime));
		}

		//ドスンのつぶれと、足場が当たったつぶれを足す
		const float squash = Spring(slamTimer_, BossFallConfig::kSlamSquash) + Spring(hitTimer_, hitSquash_);
		rig_.Apply(MakePose(), feet_, yawDeg_, squash);
		rig_.SetFlash(flash_);
	}

	//見えているときだけパーツを描く
	void BossFallShow::Draw()
	{
		if (state_ == State::Hidden)
		{
			return;
		}
		rig_.Draw();
	}

	//カメラで映すときの胸のあたり
	Vector3 BossFallShow::GetFocus() const
	{
		return feet_ + Vector3{ 0.0f, BossFallConfig::kBossFocusHeight, 0.0f };
	}

	//足場が飛んでいく先
	Vector3 BossFallShow::GetHitTarget() const
	{
		return feet_ + Vector3{ 0.0f, BossFallConfig::kBarrageTargetHeight, 0.0f };
	}
}
