#include "TitleBlockPlay.h"
#include "TitlePoses.h"
#include "../../Config/GameConfig.h"
#include "../../Config/ShowcaseConfig.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		constexpr float kRadToDeg = 180.0f / std::numbers::pi_v<float>;
		constexpr int32_t kPlaceSteps = 3; //のせる回数
		constexpr int32_t kCheerStep = 3; //喜ぶ番号
		constexpr int32_t kRestStep = 7; //下ろし終わってひと休みする番号
		constexpr float kRestTime = 0.6f; //ひと休みの秒数

		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			const float c = std::clamp(t, 0.0f, 1.0f);
			return c * c * (3.0f - 2.0f * c);
		}

		//aからbへの区間でtがどれだけ進んだか
		float Segment(float t, float a, float b)
		{
			return Smooth((t - a) / (b - a));
		}
	}

	//積み木の壁と山とプレイヤーを作る
	void TitleBlockPlay::Initialize()
	{
		BuildWall();
		rig_.Initialize();
		hearts_.Initialize();
		timer_ = 0.0f;
		step_ = 0;
		isHolding_ = false;
		isToppling_ = false;
		toppleTimer_ = 0.0f;
	}

	//弧の形の壁と、下ろした積み木の山を作る
	void TitleBlockPlay::BuildWall()
	{
		bricks_.clear();
		const Vector3& size = ShowcaseConfig::kBrickSize;
		const float modelScale = 1.0f / GameConfig::kCubeModelSize;
		const auto addBrick = [&](const Vector3& position, float yawDeg)
		{
			Brick brick;
			brick.model = Model::Create(GameConfig::kCubeModel);
			brick.model->SetScale({ size.z * modelScale, size.y * modelScale, size.x * modelScale });
			const size_t colorIndex = bricks_.size() % ShowcaseConfig::kBlockColors.size();
			brick.model->SetColor(ShowcaseConfig::kBlockColors[colorIndex]);
			brick.position = position;
			brick.rotate = { 0.0f, yawDeg, 0.0f };
			bricks_.push_back(std::move(brick));
		};

		//段ごとに半分ずらして弧に並べ、いちばん上のまん中3本はあけておく
		const int32_t half = ShowcaseConfig::kWallColumns / 2;
		for (int32_t row = 0; row < ShowcaseConfig::kWallRows; ++row)
		{
			const float shift = static_cast<float>(row % 2) * 0.5f - 0.25f;
			for (int32_t column = 0; column < ShowcaseConfig::kWallColumns; ++column)
			{
				const float deg = (static_cast<float>(column - half) + shift) * ShowcaseConfig::kWallArcDeg;
				const float rad = deg / kRadToDeg;
				const Vector3 position = {
					ShowcaseConfig::kWallCenter.x - ShowcaseConfig::kWallRadius * std::cos(rad),
					size.y * (static_cast<float>(row) + 0.5f),
					ShowcaseConfig::kWallCenter.z + ShowcaseConfig::kWallRadius * std::sin(rad) };
				const bool isTop = row == ShowcaseConfig::kWallRows - 1;
				const int32_t slot = column - 1;
				if (isTop && slot >= 0 && slot < static_cast<int32_t>(slots_.size()))
				{
					slots_[static_cast<size_t>(slot)] = position;
					slotYaws_[static_cast<size_t>(slot)] = deg;
					continue;
				}
				addBrick(position, deg);
			}
		}

		//動かす積み木は右前の山に重ねておく
		for (int32_t i = 0; i < ShowcaseConfig::kLooseBlocks; ++i)
		{
			addBrick(GetPileSpot(ShowcaseConfig::kLooseBlocks - 1 - i), 20.0f + static_cast<float>(i) * 12.0f);
		}
	}

	//山の下から数えた位置
	Vector3 TitleBlockPlay::GetPileSpot(int32_t height) const
	{
		const Vector3& size = ShowcaseConfig::kBrickSize;
		return { ShowcaseConfig::kPileCenter.x, size.y * (static_cast<float>(height) + 0.5f), ShowcaseConfig::kPileCenter.z };
	}

	//index回目にのせるか下ろす動き
	TitleBlockPlay::Move TitleBlockPlay::MakeMove(int32_t index) const
	{
		const int32_t looseStart = static_cast<int32_t>(bricks_.size()) - ShowcaseConfig::kLooseBlocks;
		Move move;
		if (index < kPlaceSteps)
		{
			//山の上から順に壁のあいている所へ
			const size_t slot = static_cast<size_t>(index);
			move.brick = looseStart + index;
			move.from = GetPileSpot(ShowcaseConfig::kLooseBlocks - 1 - index);
			move.to = slots_[slot];
			move.fromYaw = 20.0f + static_cast<float>(index) * 12.0f;
			move.toYaw = slotYaws_[slot];
			move.isFromPile = true;
			return move;
		}

		//最後にのせたものから山へ下ろす
		const int32_t loose = kPlaceSteps - 1 - (index - kCheerStep - 1);
		const size_t slot = static_cast<size_t>(loose);
		move.brick = looseStart + loose;
		move.from = slots_[slot];
		move.to = GetPileSpot(ShowcaseConfig::kLooseBlocks - 1 - loose);
		move.fromYaw = slotYaws_[slot];
		move.toYaw = 20.0f + static_cast<float>(loose) * 12.0f;
		move.isFromPile = false;
		return move;
	}

	//遊ぶか崩れるのを1フレーム進める
	void TitleBlockPlay::Update(float deltaTime)
	{
		hearts_.Update();
		if (isToppling_)
		{
			UpdateTopple(deltaTime);
		}
		else
		{
			UpdatePlay(deltaTime);
		}
		for (Brick& brick : bricks_)
		{
			brick.pop = std::max(brick.pop - deltaTime * 4.0f, 0.0f);
			PlaceBrick(brick);
		}
	}

	//のせて喜んで下ろすのをくり返す
	void TitleBlockPlay::UpdatePlay(float deltaTime)
	{
		timer_ += deltaTime;
		const Vector3& feet = ShowcaseConfig::kTitlePlayer;
		const float scale = ShowcaseConfig::kPlayerScale;
		float yaw = ShowcaseConfig::kTitlePlayerYaw;

		//全部のせたらカメラの方を向いて万歳で2回跳ねる
		if (step_ == kCheerStep)
		{
			const float hopTime = ShowcaseConfig::kCheerTime * 0.5f;
			const float phase = std::fmod(timer_, hopTime) / hopTime;
			const float hop = 4.0f * phase * (1.0f - phase);
			const float turn = std::min(Segment(timer_, 0.0f, 0.25f), 1.0f - Segment(timer_, ShowcaseConfig::kCheerTime - 0.25f, ShowcaseConfig::kCheerTime));
			yaw += ShowcaseConfig::kTitleCheerTurn * turn;
			rig_.Apply(LerpPose(TitlePoses::IdlePose(timer_), TitlePoses::CheerPose(hop, timer_), turn), feet, yaw, scale);
			//着地のたびにハート
			if (std::fmod(std::max(timer_ - deltaTime, 0.0f), hopTime) > std::fmod(timer_, hopTime))
			{
				hearts_.Burst(feet, 0.8f);
			}
			if (timer_ >= ShowcaseConfig::kCheerTime)
			{
				hearts_.Burst(feet, 1.2f);
				timer_ = 0.0f;
				step_ += 1;
			}
			return;
		}

		//下ろし終わったら少し休んで、またのせ始める
		if (step_ == kRestStep)
		{
			rig_.Apply(TitlePoses::IdlePose(timer_), feet, yaw, scale);
			if (timer_ >= kRestTime)
			{
				timer_ = 0.0f;
				step_ = 0;
			}
			return;
		}

		//手をのばして取り、胸の前で持って、置く所へのばし、ちょいちょいと直してから戻る
		const float cycle = ShowcaseConfig::kPlaceCycle;
		const float t = timer_ / cycle * 2.2f;
		const Move move = MakeMove(step_);
		RigPose source = TitlePoses::ReachPose();
		RigPose destination = TitlePoses::BendPose();
		if (move.isFromPile)
		{
			source = TitlePoses::BendPose();
			destination = TitlePoses::ReachPose();
		}
		RigPose pose = LerpPose(TitlePoses::IdlePose(timer_), source, Segment(t, 0.0f, 0.45f));
		pose = LerpPose(pose, TitlePoses::CarryPose(), Segment(t, 0.45f, 1.0f));
		pose = LerpPose(pose, destination, Segment(t, 1.0f, 1.45f));
		const float wiggle = Segment(t, 1.45f, 1.5f) * (1.0f - Segment(t, 1.75f, 1.8f));
		pose.armForwardL += 6.0f * std::sin(timer_ * 30.0f) * wiggle;
		pose.armForwardR -= 6.0f * std::sin(timer_ * 30.0f) * wiggle;
		pose = LerpPose(pose, TitlePoses::IdlePose(timer_), Segment(t, 1.8f, 2.2f));
		rig_.Apply(pose, feet, yaw, scale);

		//つかんでから離すまで積み木は両手のまん中について、離したら置く所へすっと入る
		Brick& brick = bricks_[static_cast<size_t>(move.brick)];
		const Vector3 hands = (rig_.GetHandPosition(true) + rig_.GetHandPosition(false)) * 0.5f;
		if (t < 0.45f)
		{
			brick.position = move.from;
			brick.rotate.y = move.fromYaw;
		}
		else if (t < 1.45f)
		{
			isHolding_ = true;
			brick.position = hands;
			brick.rotate.y = move.fromYaw + (move.toYaw - move.fromYaw) * Segment(t, 0.45f, 1.45f);
		}
		else
		{
			if (isHolding_)
			{
				isHolding_ = false;
				brick.pop = 1.0f;
			}
			const float settle = Segment(t, 1.45f, 1.65f);
			brick.position = hands + (move.to - hands) * settle;
			brick.rotate.y = move.toYaw;
		}

		if (timer_ >= cycle)
		{
			brick.position = move.to;
			timer_ = 0.0f;
			step_ += 1;
		}
	}

	//カメラのeyeへ向かって積み木が倒れてきてプレイヤーがあせる
	void TitleBlockPlay::Topple(const Vector3& eye)
	{
		if (isToppling_)
		{
			return;
		}
		isToppling_ = true;
		toppleTimer_ = 0.0f;
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		for (Brick& brick : bricks_)
		{
			//上の段から先に、カメラの方へ少し浮きながら転がってくる
			Vector3 toEye = eye - brick.position;
			const float length = std::sqrt(toEye.x * toEye.x + toEye.y * toEye.y + toEye.z * toEye.z);
			toEye = toEye * (1.0f / std::max(length, 0.001f));
			const float speed = ShowcaseConfig::kToppleSpeed * (0.6f + unit(random_) * 0.6f);
			brick.velocity = toEye * speed + Vector3{ (unit(random_) - 0.5f) * 3.0f, 2.5f + unit(random_) * 2.0f, (unit(random_) - 0.5f) * 3.0f };
			brick.spin = { (unit(random_) - 0.5f) * 900.0f, (unit(random_) - 0.5f) * 400.0f, (unit(random_) - 0.5f) * 900.0f };
			const float height = brick.position.y / ShowcaseConfig::kBrickSize.y;
			brick.delay = (static_cast<float>(ShowcaseConfig::kWallRows) - height) * ShowcaseConfig::kToppleStagger + unit(random_) * 0.08f;
		}
	}

	//倒れてくる積み木とあせるプレイヤー
	void TitleBlockPlay::UpdateTopple(float deltaTime)
	{
		toppleTimer_ += deltaTime;
		for (Brick& brick : bricks_)
		{
			if (toppleTimer_ < brick.delay)
			{
				continue;
			}
			brick.velocity.y -= ShowcaseConfig::kToppleGravity * deltaTime;
			brick.position = brick.position + brick.velocity * deltaTime;
			brick.rotate = brick.rotate + brick.spin * deltaTime;
		}

		//カメラの方を向いてばたばたあせる
		const float turn = Segment(toppleTimer_, 0.0f, 0.2f);
		const float yaw = ShowcaseConfig::kTitlePlayerYaw + ShowcaseConfig::kTitleCheerTurn * turn;
		rig_.Apply(LerpPose(TitlePoses::IdlePose(toppleTimer_), TitlePoses::PanicPose(toppleTimer_), turn), ShowcaseConfig::kTitlePlayer, yaw, ShowcaseConfig::kPlayerScale);
	}

	//行列を確定する
	void TitleBlockPlay::PlaceBrick(Brick& brick)
	{
		//置いたときに縦にぷるんとつぶれて戻る
		const Vector3& size = ShowcaseConfig::kBrickSize;
		const float modelScale = 1.0f / GameConfig::kCubeModelSize;
		const float pop = std::sin(brick.pop * std::numbers::pi_v<float> * 2.0f) * brick.pop * 0.25f;
		brick.model->SetScale({ size.z * modelScale * (1.0f + pop), size.y * modelScale * (1.0f - pop), size.x * modelScale * (1.0f + pop) });
		brick.model->SetRotate(brick.rotate);
		brick.model->SetTranslate(brick.position);
		brick.model->TransferMatrix();
	}

	//積み木とプレイヤーとハートを描く
	void TitleBlockPlay::Draw()
	{
		for (Brick& brick : bricks_)
		{
			brick.model->Draw();
		}
		rig_.Draw();
		hearts_.Draw();
	}
}
