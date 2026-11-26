#include "GameOverBury.h"
#include "../../Object/Tsumiki/Tsumiki.h"
#include "../../Config/GameOverConfig.h"
#include "../../Config/ShowcaseConfig.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

using namespace RyoEngine;

namespace App
{
	namespace
	{
		//ゆっくり始まってゆっくり止まる進み具合
		float Smooth(float t)
		{
			const float c = std::clamp(t, 0.0f, 1.0f);
			return c * c * (3.0f - 2.0f * c);
		}
	}

	//プレイヤーと降らせる積み木を作る
	void GameOverBury::Initialize()
	{
		rig_.Initialize();
		timer_ = 0.0f;
		lookTime_ = GameOverConfig::kLookTime;
		landedCount_ = 0;
		isBuried_ = false;
		const size_t cells = static_cast<size_t>(GameOverConfig::kPileCells * GameOverConfig::kPileCells);
		pile_.assign(cells, 0.0f);

		//プレイヤーのまわりに集まるように、正規分布の位置から順に降らせる
		std::uniform_real_distribution<float> unit(0.0f, 1.0f);
		std::normal_distribution<float> spread(0.0f, GameOverConfig::kShowerSpread);
		blocks_.clear();
		for (int32_t i = 0; i < GameOverConfig::kShowerCount; ++i)
		{
			Block block;
			block.model = Tsumiki::Create();
			block.model->SetColor(ShowcaseConfig::kBlockColors[static_cast<size_t>(i) % ShowcaseConfig::kBlockColors.size()]);
			block.size = GameOverConfig::kShowerMinSize + (GameOverConfig::kShowerMaxSize - GameOverConfig::kShowerMinSize) * unit(random_);
			block.position = {
				GameOverConfig::kOverPlayer.x + spread(random_),
				GameOverConfig::kShowerHeight + unit(random_) * 3.0f,
				GameOverConfig::kOverPlayer.z + spread(random_) };
			block.rotate = { unit(random_) * 90.0f, unit(random_) * 90.0f, unit(random_) * 90.0f };
			block.spin = { (unit(random_) - 0.5f) * 600.0f, (unit(random_) - 0.5f) * 600.0f, (unit(random_) - 0.5f) * 600.0f };
			block.velocityY = -GameOverConfig::kShowerSpeed * (0.8f + unit(random_) * 0.4f);
			block.spawnTime = lookTime_ + GameOverConfig::kShowerTime * static_cast<float>(i) / static_cast<float>(GameOverConfig::kShowerCount);
			blocks_.push_back(std::move(block));
		}
	}

	//その位置の升目の積もった高さ
	float& GameOverBury::PileAt(const Vector3& position)
	{
		const float half = static_cast<float>(GameOverConfig::kPileCells) * GameOverConfig::kPileCell * 0.5f;
		const int32_t x = static_cast<int32_t>(std::floor((position.x - GameOverConfig::kOverPlayer.x + half) / GameOverConfig::kPileCell));
		const int32_t z = static_cast<int32_t>(std::floor((position.z - GameOverConfig::kOverPlayer.z + half) / GameOverConfig::kPileCell));
		if (x < 0 || z < 0 || x >= GameOverConfig::kPileCells || z >= GameOverConfig::kPileCells)
		{
			outside_ = 0.0f;
			return outside_;
		}
		return pile_[static_cast<size_t>(z * GameOverConfig::kPileCells + x)];
	}

	//きょろきょろと積み木のシャワーを1フレーム進める
	void GameOverBury::Update(float deltaTime)
	{
		timer_ += deltaTime;
		for (Block& block : blocks_)
		{
			UpdateBlock(block, deltaTime);
		}
		UpdatePlayer();
		if (!isBuried_ && landedCount_ >= static_cast<int32_t>(blocks_.size()))
		{
			isBuried_ = true;
		}
	}

	//落として、積もった高さに乗せる
	void GameOverBury::UpdateBlock(Block& block, float deltaTime)
	{
		if (timer_ < block.spawnTime || block.isLanded)
		{
			return;
		}

		block.velocityY -= GameOverConfig::kShowerGravity * deltaTime;
		block.position.y += block.velocityY * deltaTime;
		block.rotate = block.rotate + block.spin * deltaTime;

		//積もった所まで落ちたら、まわりより高すぎるときは低い方へ転がしてから止めて、山になるように積む
		if (block.position.y - block.size * 0.5f <= PileAt(block.position))
		{
			RollDown(block);
			float& height = PileAt(block.position);
			block.position.y = height + block.size * 0.5f;
			height += block.size * GameOverConfig::kPileStack;
			const float spill = block.size * GameOverConfig::kPileSpill;
			const float step = GameOverConfig::kPileCell;
			PileAt(block.position + Vector3{ step, 0.0f, 0.0f }) += spill;
			PileAt(block.position + Vector3{ -step, 0.0f, 0.0f }) += spill;
			PileAt(block.position + Vector3{ 0.0f, 0.0f, step }) += spill;
			PileAt(block.position + Vector3{ 0.0f, 0.0f, -step }) += spill;
			block.isLanded = true;
			++landedCount_;
		}
		Tsumiki::Place(*block.model, block.position, { block.size, block.size, block.size }, block.rotate);
		block.model->TransferMatrix();
	}

	//まわりの升目より積み木1個分以上高ければ、いちばん低いとなりへ転がす
	void GameOverBury::RollDown(Block& block)
	{
		const float step = GameOverConfig::kPileCell;
		const std::array<Vector3, 4> sides = { { { step, 0.0f, 0.0f }, { -step, 0.0f, 0.0f }, { 0.0f, 0.0f, step }, { 0.0f, 0.0f, -step } } };
		for (int32_t roll = 0; roll < GameOverConfig::kPileRollLimit; ++roll)
		{
			const float here = PileAt(block.position);
			Vector3 lowest = block.position;
			float lowestHeight = here;
			for (const Vector3& side : sides)
			{
				const float height = PileAt(block.position + side);
				if (height < lowestHeight)
				{
					lowestHeight = height;
					lowest = block.position + side;
				}
			}
			if (here - lowestHeight < block.size)
			{
				return;
			}
			block.position.x = lowest.x;
			block.position.z = lowest.z;
		}
	}

	//きょろきょろして、降ってきたら頭をかばってだんだん縮こまる
	void GameOverBury::UpdatePlayer()
	{
		const Vector3& feet = GameOverConfig::kOverPlayer;
		const float faceCamera = std::atan2(GameOverConfig::kOverEye.x - feet.x, GameOverConfig::kOverEye.z - feet.z) * 180.0f / std::numbers::pi_v<float>;
		const float speed = GameOverConfig::kLookSpeed;

		//首と体を左右にふって、足をもじもじさせる
		RigPose look;
		const float glance = std::sin(timer_ * speed);
		look.headYaw = 45.0f * glance * std::min(std::abs(glance) * 2.0f, 1.0f);
		look.waistTwist = 12.0f * std::sin(timer_ * speed - 0.4f);
		look.armOutL = 15.0f;
		look.armOutR = 15.0f;
		look.legForwardL = 8.0f * std::sin(timer_ * speed * 2.0f);
		look.legForwardR = -8.0f * std::sin(timer_ * speed * 2.0f);

		//最後に上を見上げて、はっとする
		const float lookUp = Smooth((timer_ - (lookTime_ - 0.4f)) / 0.3f);
		RigPose surprised;
		surprised.headNod = -35.0f;
		surprised.armOutL = 35.0f;
		surprised.armOutR = 35.0f;
		surprised.squash = 1.05f;
		RigPose pose = LerpPose(look, surprised, lookUp);

		//降ってきたら腕で頭をかばってばたばた、積もるほど縮こまる
		if (timer_ >= lookTime_)
		{
			const float buried = Smooth((timer_ - lookTime_) / GameOverConfig::kShowerTime);
			RigPose cover;
			cover.armForwardL = 150.0f + 25.0f * std::sin(timer_ * 24.0f);
			cover.armForwardR = 150.0f - 25.0f * std::sin(timer_ * 24.0f);
			cover.armOutL = -10.0f;
			cover.armOutR = -10.0f;
			cover.headNod = 20.0f;
			cover.waistLean = 25.0f * buried;
			cover.bob = -0.6f * buried;
			cover.squash = 1.0f - 0.2f * buried;
			pose = LerpPose(pose, cover, Smooth((timer_ - lookTime_) / 0.2f));
		}
		rig_.Apply(pose, feet, faceCamera, ShowcaseConfig::kPlayerScale);
	}

	//プレイヤーと降ってきた積み木を描く
	void GameOverBury::Draw()
	{
		rig_.Draw();
		for (Block& block : blocks_)
		{
			if (timer_ >= block.spawnTime)
			{
				block.model->Draw();
			}
		}
	}
}
