#include "PlayerAnimation.h"
#include "../../Config/DirectionConfig.h"
#include "../../Config/PlayerConfig.h"

using namespace RyoEngine;

namespace App
{
	//別のファイルのアニメーションを足して待機をループで流す
	void PlayerAnimation::Initialize(Model& body)
	{
		body_ = &body;
		//ファイルごとに名前をつけて足すと名前の後ろにglbの中のアニメーションの名前がつく
		body_->LoadAnimation(DirectionConfig::kPlayerJumpFile, "Jump");
		body_->LoadAnimation(DirectionConfig::kPlayerSwingFile, "Swing");
		body_->LoadAnimation(DirectionConfig::kPlayerPoseFile, "Pose");
		PlayIdle();
	}

	//アニメーションを最初から速さを決めて流す
	void PlayerAnimation::Play(const char* clip, bool isLoop, float speed, State state)
	{
		state_ = state;
		isAfterImpact_ = false;
		//見つからないときは流したことにしてすぐ終わらせる
		isClipMissing_ = !body_->HasAnimation(clip);
		if (isClipMissing_)
		{
			return;
		}
		body_->PlayAnimation(clip, isLoop, DirectionConfig::kPlayerAnimBlend, true);
		body_->SetAnimationSpeed(speed);
	}

	//待機をループで流す
	void PlayerAnimation::PlayIdle()
	{
		Play(DirectionConfig::kPlayerIdleClip, true, 1.0f, State::Idle);
	}

	//まっすぐ立った待機の最初の姿勢で止めて、脚を動かすシェーダーが使いやすいようにする
	void PlayerAnimation::Freeze()
	{
		Play(DirectionConfig::kPlayerIdleClip, true, 0.0f, State::Frozen);
	}

	//1回跳ぶ
	void PlayerAnimation::PlayJump()
	{
		//ハンマーを振っている間は振るアニメーションを最後まで見せ、止めている間は止めたまま
		if (IsSwinging() || state_ == State::Frozen)
		{
			return;
		}
		//アニメーションで足が床に着くときにちょうど1マス跳び終わる速さ
		const float speed = DirectionConfig::kPlayerJumpLandTime / PlayerConfig::kJumpDuration;
		Play(DirectionConfig::kPlayerJumpClip, false, speed, State::Jump);
	}

	//ハンマーを振り下ろして首をかしげる
	void PlayerAnimation::PlaySwing()
	{
		Play(DirectionConfig::kPlayerSwingClip, false, DirectionConfig::kPlayerSwingSpeed, State::Swing);
	}

	//クリアのポーズを1回する
	void PlayerAnimation::PlayPose()
	{
		Play(DirectionConfig::kPlayerPoseClip, false, 1.0f, State::Pose);
	}

	//1回だけのアニメーションが終わったら待機へ戻し、ハンマーが当たったら首かしげの速さにする
	void PlayerAnimation::Update()
	{
		//ハンマーが床に当たったら首かしげはゆっくり見せる
		isImpactNow_ = false;
		if (state_ == State::Swing && !isAfterImpact_ && body_->GetAnimationTime() >= DirectionConfig::kPlayerSwingImpactTime)
		{
			isAfterImpact_ = true;
			isImpactNow_ = true;
			body_->SetAnimationSpeed(DirectionConfig::kPlayerSwingAfterSpeed);
		}

		//移動と足場を壊すのは終わったら待機へ、ポーズは最後の姿勢のまま
		const bool isOnce = state_ == State::Jump || state_ == State::Swing;
		if (isOnce && (isClipMissing_ || body_->IsAnimationFinished()))
		{
			PlayIdle();
		}
	}

	//クリアのポーズが終わったか
	bool PlayerAnimation::IsPoseFinished() const
	{
		return state_ == State::Pose && (isClipMissing_ || body_->IsAnimationFinished());
	}
}
