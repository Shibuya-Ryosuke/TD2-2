#include "BossView.h"
#include "Boss.h"
#include "../../Config/GameConfig.h"
#include "BossDetail.h"
#include "../ModelColor.h"
#include <cmath>
#include <numbers>

using namespace RyoEngine;
using namespace App::BossDetail;

namespace App
{
	namespace
	{
		//予告の大きさと位置
		constexpr float kMeleeTellHeight = 0.025f; //近接攻撃の予告の厚さ
		constexpr float kMeleeTellY = 0.035f;
		constexpr float kPunchTellHeight = 0.035f; //パンチの予告の厚さ
		constexpr float kPunchTellY = 0.05f;

		//1辺2のcube.objを1辺sizeの箱にする大きさ
		Vector3 CubeScale(const Vector3& size)
		{
			return size * (1.0f / GameConfig::kCubeModelSize);
		}
	}

	//モデルを作る
	void BossView::Initialize()
	{
		//mtlの色のままのボスのモデル
		body_ = Model::Create(GameConfig::kBossModel);
		ModelColor::Apply(*body_, GameConfig::kBossModel);

		//モデルの手と同じ緑の箱の拳
		fist_ = Model::Create(GameConfig::kCubeModel);
		fist_->SetColor(GameConfig::kBossFistColor);

		//足元の板の近接攻撃の予告
		meleeTell_ = Model::Create(GameConfig::kCubeModel);
		meleeTell_->SetColor(GameConfig::kAttackColor);
		meleeTell_->SetCastShadow(false);

		//前にのびる板のパンチの予告
		punchTell_ = Model::Create(GameConfig::kCubeModel);
		punchTell_->SetColor(GameConfig::kWarningColor);
		punchTell_->SetCastShadow(false);
	}

	//立っているか仰向けに寝ているかで体の行列を確定する
	void BossView::UpdateBody(const Boss& boss)
	{
		//寝ている間はすうすう息をして、足場が当たるとむぎゅっとつぶれる
		breath_ += GetDeltaTime();
		const float sleep = boss.GetSleep();
		const float breath = 1.0f + std::sin(breath_ * GameConfig::kBossSleepBreathSpeed) * GameConfig::kBossSleepBreath * sleep;
		const float squash = 1.0f - GameConfig::kBossFlinchSquash * boss.GetFlinch();
		const float widen = 1.0f + GameConfig::kBossFlinchSquash * 0.5f * boss.GetFlinch();
		const Vector3& modelScale = GameConfig::kBossModelScale;
		if (sleep < 0.5f)
		{
			body_->SetScale({ modelScale.x * widen, modelScale.y * squash, modelScale.z * widen });
			body_->SetRotate({ 0.0f, boss.GetYawDeg() + GameConfig::kBossModelYaw, 0.0f });
			body_->SetTranslate(boss.GetPosition());
			body_->TransferMatrix();
			return;
		}

		//仰向けは正面を上へ倒して、頭を向いていた方の反対へ寝かせ、体のまん中をマスに合わせる
		const Vector3 forward = boss.GetForward();
		const Vector3 head = { -forward.x, 0.0f, -forward.z };
		const float headDeg = std::atan2(head.z, head.x) * 180.0f / std::numbers::pi_v<float>;
		body_->SetScale({ modelScale.x * breath * squash, modelScale.y * widen, modelScale.z * breath * widen });
		body_->SetRotate({ headDeg, 0.0f, -90.0f });
		const float center = GameConfig::kBossSleepCenter * modelScale.y;
		body_->SetTranslate(boss.GetPosition() + Vector3{ -head.x * center, GameConfig::kBossSleepLift * modelScale.x, -head.z * center });
		body_->TransferMatrix();
	}

	//ボスの位置と向きと攻撃に合わせて行列を確定する
	void BossView::Update(const Boss& boss)
	{
		//Y軸だけ回すボスの向き
		const Vector3 rotate = { 0.0f, boss.GetYawDeg(), 0.0f };

		UpdateBody(boss);

		//拳
		fist_->SetScale(CubeScale(kFistSize));
		fist_->SetRotate(rotate);
		fist_->SetTranslate(boss.ToWorld(boss.GetFistLocal()));
		fist_->TransferMatrix();

		//近接攻撃の予告
		const float meleeSize = boss.GetMeleeTellSize();
		meleeTell_->SetScale(CubeScale({ meleeSize, kMeleeTellHeight, meleeSize }));
		meleeTell_->SetRotate(rotate);
		meleeTell_->SetTranslate(boss.ToWorld({ 0.0f, kMeleeTellY, 0.0f }));
		meleeTell_->TransferMatrix();

		//パンチの予告
		const float punchRange = boss.GetPunchTellRange();
		punchTell_->SetScale(CubeScale({ boss.GetPunchTellWidth(), kPunchTellHeight, punchRange }));
		punchTell_->SetRotate(rotate);
		punchTell_->SetTranslate(boss.ToWorld({ 0.0f, kPunchTellY, punchRange * 0.5f }));
		punchTell_->TransferMatrix();


		//突進のルートのマスに警告の板を置く
		const Rush& rush = boss.GetRush();
		const std::vector<Vector3>& centers = rush.GetRouteCenters();
		rushTellCount_ = static_cast<int32_t>(centers.size());

		//足りない分だけ板を作る
		while (rushTells_.size() < centers.size()) {
			std::unique_ptr<Model> tell = Model::Create(GameConfig::kCubeModel);
			tell->SetColor(GameConfig::kWarningColor);
			tell->SetCastShadow(false);
			rushTells_.push_back(std::move(tell));
		}

		const Vector2& tileSize = rush.GetTileSize();
		for (size_t i = 0; i < centers.size(); ++i) {
			Model& tell = *rushTells_[i];
			tell.SetScale(CubeScale({ tileSize.x * rushTellScale_, rushTellHeight_, tileSize.y * rushTellScale_ }));
			tell.SetRotate({ 0.0f, 0.0f, 0.0f });
			tell.SetTranslate(centers[i] + Vector3{ 0.0f, rushTellY_, 0.0f });
			tell.TransferMatrix();
		}
	}

	//体と拳と攻撃の予告を描く
	void BossView::Draw(const Boss& boss)
	{
		if (!boss.IsActive())
		{
			return;
		}
		body_->Draw();
		if (boss.IsPunching())
		{
			fist_->Draw();
		}
		if (boss.IsMeleeTellVisible())
		{
			meleeTell_->Draw();
		}
		if (boss.IsPunchTellVisible())
		{
			punchTell_->Draw();
		}
		//突進の警告
		for (int32_t i = 0; i < rushTellCount_; ++i) {
			rushTells_[static_cast<size_t>(i)]->Draw();
		}
	}
}
