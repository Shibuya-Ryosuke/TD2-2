#include "BossView.h"
#include "Boss.h"
#include "../../Config/GameConfig.h"

using namespace RyoEngine;

namespace App
{
	namespace
	{
		// --- 見た目の大きさ・位置 (Unity のプロトタイプのボスと同じ。ボスのローカル) ---
		constexpr Vector3 kBodySize = { 1.55f, 1.5f, 1.45f };
		constexpr Vector3 kBodyCenter = { 0.0f, 0.75f, 0.0f };
		constexpr Vector3 kFistSize = { 0.75f, 0.75f, 0.9f };
		constexpr float kMeleeTellHeight = 0.025f; // 近接攻撃の予告の厚さ
		constexpr float kMeleeTellY = 0.035f;
		constexpr float kPunchTellHeight = 0.035f; // パンチの予告の厚さ
		constexpr float kPunchTellY = 0.05f;

		// cube.obj (1辺 2) を、1辺が size の箱にする大きさ
		Vector3 CubeScale(const Vector3& size)
		{
			return size * (1.0f / GameConfig::kCubeModelSize);
		}
	}

	// モデルを作る
	void BossView::Initialize()
	{
		// 体 (赤い箱)
		body_ = Model::Create(GameConfig::kCubeModel);
		body_->SetColor(GameConfig::kBossColor);

		// 拳
		fist_ = Model::Create(GameConfig::kCubeModel);
		fist_->SetColor(GameConfig::kBossArmorColor);

		// 近接攻撃の予告 (足元の板)
		meleeTell_ = Model::Create(GameConfig::kCubeModel);
		meleeTell_->SetColor(GameConfig::kAttackColor);
		meleeTell_->SetCastShadow(false);

		// パンチの予告 (前にのびる板)
		punchTell_ = Model::Create(GameConfig::kCubeModel);
		punchTell_->SetColor(GameConfig::kWarningColor);
		punchTell_->SetCastShadow(false);
	}

	// ボスの位置・向き・攻撃に合わせて、行列を確定する
	void BossView::Update(const Boss& boss)
	{
		// ボスの向き (Y 軸だけ回す)
		const Vector3 rotate = { 0.0f, boss.GetYawDeg(), 0.0f };

		// 体
		body_->SetScale(CubeScale(kBodySize));
		body_->SetRotate(rotate);
		body_->SetTranslate(boss.ToWorld(kBodyCenter));
		body_->TransferMatrix();

		// 拳
		fist_->SetScale(CubeScale(kFistSize));
		fist_->SetRotate(rotate);
		fist_->SetTranslate(boss.ToWorld(boss.GetFistLocal()));
		fist_->TransferMatrix();

		// 近接攻撃の予告
		const float meleeSize = boss.GetMeleeTellSize();
		meleeTell_->SetScale(CubeScale({ meleeSize, kMeleeTellHeight, meleeSize }));
		meleeTell_->SetRotate(rotate);
		meleeTell_->SetTranslate(boss.ToWorld({ 0.0f, kMeleeTellY, 0.0f }));
		meleeTell_->TransferMatrix();

		// パンチの予告
		const float punchRange = boss.GetPunchTellRange();
		punchTell_->SetScale(CubeScale({ boss.GetPunchTellWidth(), kPunchTellHeight, punchRange }));
		punchTell_->SetRotate(rotate);
		punchTell_->SetTranslate(boss.ToWorld({ 0.0f, kPunchTellY, punchRange * 0.5f }));
		punchTell_->TransferMatrix();
	}

	// 体・拳・攻撃の予告を描く
	void BossView::Draw(const Boss& boss)
	{
		if (!boss.IsActive())
		{
			return;
		}
		body_->Draw();
		fist_->Draw();
		if (boss.IsMeleeTellVisible())
		{
			meleeTell_->Draw();
		}
		if (boss.IsPunchTellVisible())
		{
			punchTell_->Draw();
		}
	}
}
