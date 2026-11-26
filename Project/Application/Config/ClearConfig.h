#pragma once
#include <RyoEngine.h>
#include <cstdint>

//クリアでプレイヤーが倒れたボスをハンマーでたたいてバラバラにし、ギャルっぽいダンスをする演出の数値
namespace App::ClearConfig
{
	//流れの秒数
	inline constexpr float kWalkTime = 1.5f; //ボスへ歩いていく秒数
	inline constexpr float kWindupTime = 0.6f; //ハンマーを振り上げる秒数
	inline constexpr float kSwingTime = 0.12f; //ハンマーを振り下ろす秒数
	inline constexpr float kHitStopTime = 0.12f; //当たった瞬間に止まって見せる秒数
	inline constexpr float kScatterTime = 1.1f; //バラバラになったボスを見せる秒数
	inline constexpr float kTurnTime = 0.6f; //カメラの方を向いてくるっと回る秒数
	inline constexpr float kBeat = 0.48f; //ダンスでリズムに乗る1拍の秒数

	//置く所
	inline constexpr RyoEngine::Vector3 kBossCenter = { -1.0f, 0.0f, 1.0f }; //倒れているボスの体のまん中の床
	inline constexpr float kBossScale = 1.0f; //ボスのパーツの大きさの倍率
	inline constexpr float kBossLength = 4.0f; //ボスの足から頭までの長さ
	inline constexpr float kBossBackDepth = 0.42f; //仰向けのときに背中が床に埋まらないように上げる高さ
	inline constexpr RyoEngine::Vector3 kPlayerStart = { 5.5f, 0.0f, 0.6f }; //プレイヤーが歩き始める所
	inline constexpr RyoEngine::Vector3 kPlayerStop = { 2.0f, 0.0f, 0.75f }; //ハンマーを振る所
	inline constexpr float kPlayerFaceBossYaw = -90.0f; //ボスの方を向く度
	inline constexpr float kHammerScale = 0.55f; //プレイヤーに持たせるハンマーの倍率
	inline constexpr float kHammerAlongArm = 143.6f; //柄を腕の向きにそろえるハンマーのXの度
	inline constexpr RyoEngine::Vector3 kImpactPoint = { 0.6f, 0.6f, 0.9f }; //ハンマーがボスに当たる所

	//バラバラに飛ぶ
	inline constexpr float kScatterSpeed = 5.5f; //横へ飛ぶ速さ
	inline constexpr float kScatterLift = 8.0f; //上へ飛ぶ速さ
	inline constexpr float kScatterGravity = 15.0f; //パーツの重力
	inline constexpr float kScatterBounce = 0.35f; //床ではねる割合
	inline constexpr float kScatterSpin = 600.0f; //1秒に回るいちばん大きい度
	inline constexpr float kImpactShake = 0.35f; //当たったときにカメラが揺れる長さ

	//カメラ
	inline constexpr RyoEngine::Vector3 kWideEye = { 2.6f, 3.6f, -9.5f }; //歩いてくるのを引きで見るカメラの位置
	inline constexpr RyoEngine::Vector3 kWideTarget = { 0.6f, 1.0f, 0.8f };
	inline constexpr RyoEngine::Vector3 kPushEye = { 4.4f, 1.7f, -7.6f }; //振り上げるときに下から寄るカメラの位置
	inline constexpr RyoEngine::Vector3 kPushTarget = { 0.8f, 1.2f, 0.9f };
	inline constexpr RyoEngine::Vector3 kScatterEye = { 1.0f, 4.6f, -10.5f }; //バラバラに飛ぶのを引いて見るカメラの位置
	inline constexpr RyoEngine::Vector3 kScatterTarget = { 0.0f, 2.4f, 0.8f };
	inline constexpr RyoEngine::Vector3 kDanceEye = { 0.2f, 2.4f, -7.0f }; //ダンスを映すカメラの位置で、プレイヤーは画面のまん中寄りの右
	inline constexpr RyoEngine::Vector3 kDanceTarget = { 0.9f, 1.5f, 0.6f };
	inline constexpr float kShotBlendTime = 0.45f; //カメラが次の映し方へ移る秒数

	//ボスのパーツと、画像がないときに使う仮の色
	inline constexpr const char* kBossBody = "Model/boss/Body/bossBody.obj";
	inline constexpr const char* kBossHead = "Model/boss/Head/bossHead.obj";
	inline constexpr const char* kBossLeftArm = "Model/boss/LeftArm/bossLeftArm.obj";
	inline constexpr const char* kBossRightArm = "Model/boss/RightArm/bossRightArm.obj";
	inline constexpr const char* kBossLeftFeet = "Model/boss/LeftFeet/bossLeftFeet.obj";
	inline constexpr const char* kBossRightFeet = "Model/boss/RightFeet/bossRightFeet.obj";
	inline constexpr bool kUseBossTint = false; //trueならパーツに下の仮の色をつけて、falseならmtlの画像の色を使う
	inline constexpr RyoEngine::Vector4 kBossBodyColor = { 0.94f, 0.51f, 0.28f, 1.0f }; //今のボスの胴体のオレンジ
	inline constexpr RyoEngine::Vector4 kBossHeadColor = { 0.13f, 0.6f, 0.64f, 1.0f }; //今のボスの顔の青緑
	inline constexpr RyoEngine::Vector4 kBossArmColor = { 0.19f, 0.4f, 0.16f, 1.0f }; //今のボスの手の緑
	inline constexpr RyoEngine::Vector4 kBossFeetColor = { 0.19f, 0.1f, 0.05f, 1.0f }; //今のボスの木の茶色
	//パーツのまん中で、読みこむときにXが反転したあとの値
	inline constexpr RyoEngine::Vector3 kBodyCenter = { 0.0f, 1.8f, 0.0f };
	inline constexpr RyoEngine::Vector3 kHeadCenter = { 0.0f, 3.2f, 0.02f };
	inline constexpr RyoEngine::Vector3 kLeftArmCenter = { -1.24f, 1.73f, 0.0f };
	inline constexpr RyoEngine::Vector3 kRightArmCenter = { 1.24f, 1.73f, 0.0f };
	inline constexpr RyoEngine::Vector3 kLeftFeetCenter = { -0.39f, 0.62f, 0.14f };
	inline constexpr RyoEngine::Vector3 kRightFeetCenter = { 0.39f, 0.62f, 0.14f };
}
