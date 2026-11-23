#include "BossView.h"
#include "Boss.h"
#include "../../Config/GameConfig.h"
#include "BossDetail.h"

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
		//赤い箱の体
		body_ = Model::Create(GameConfig::kCubeModel);
		body_->SetColor(GameConfig::kBossColor);

		//拳
		fist_ = Model::Create(GameConfig::kCubeModel);
		fist_->SetColor(GameConfig::kBossArmorColor);

		//足元の板の近接攻撃の予告
		meleeTell_ = Model::Create(GameConfig::kCubeModel);
		meleeTell_->SetColor(GameConfig::kAttackColor);
		meleeTell_->SetCastShadow(false);

		//前にのびる板のパンチの予告
		punchTell_ = Model::Create(GameConfig::kCubeModel);
		punchTell_->SetColor(GameConfig::kWarningColor);
		punchTell_->SetCastShadow(false);
	}

	//ボスの位置と向きと攻撃に合わせて行列を確定する
	void BossView::Update(const Boss& boss)
	{
		//Y軸だけ回すボスの向き
		const Vector3 rotate = { 0.0f, boss.GetYawDeg(), 0.0f };

		//体
		body_->SetScale(CubeScale(kBodySize));
		body_->SetRotate(rotate);
		body_->SetTranslate(boss.ToWorld(kBodyCenter));
		body_->TransferMatrix();

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
	}

	//体と拳と攻撃の予告を描く
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
