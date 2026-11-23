#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>

namespace App::GameConfig
{
	//ゲーム全体の数値

	//床
	inline constexpr float kFloorSize = 20.0f; //床全体の1辺の長さ
	inline constexpr float kTileThickness = 0.45f; //タイルの厚さ
	inline constexpr float kTileGap = 0.055f; //1マスに対するタイルのすき間の割合
	inline constexpr float kFloorSpacing = 12.0f; //層と層の間の高さ
	inline constexpr float kLowerFloorBrightness = 0.25f; //下の層の明るさ
	inline constexpr int32_t kLayerFileCount = 5; //層のCSVの数で、6層目からは1層目に戻る
	inline constexpr int32_t kFirstLayerWidth = 10; //1層目のマスの数
	inline constexpr int32_t kFirstLayerDepth = 10;

	//出てくるマスを1層目のマスで指定する
	inline constexpr int32_t kPlayerSpawnX = 1;
	inline constexpr int32_t kPlayerSpawnZ = 1;
	inline constexpr int32_t kBossSpawnX = 6;
	inline constexpr int32_t kBossSpawnZ = 6;

	//床を崩す
	inline constexpr float kCollapseDelay = 2.6f; //崩してから穴になるまでの秒数
	inline constexpr float kShapePreviewBlend = 0.6f; //床に出す形の予告の色の濃さ
	inline constexpr float kPreviewGlowMin = 0.8f; //予告のマスの光のいちばん弱いとき
	inline constexpr float kPreviewGlowMax = 1.8f; //予告のマスの光のいちばん強いとき
	inline constexpr float kPreviewGlowSpeed = 1.5f; //予告のマスが1秒に明滅する回数
	inline constexpr int32_t kBossForecastTurns = 5; //ボスの行動を何回先まで残像で見せるか

	//切り離し
	inline constexpr float kEscapeDelay = 2.0f; //切り離されてから落ちるまでの秒数
	inline constexpr float kShakeDistance = 0.12f; //切り離された床の揺れ幅
	inline constexpr float kShakeFrequency = 13.0f; //切り離された床が1秒に揺れる回数

	//下の層へ
	inline constexpr float kBossDropDuration = 0.8f; //ボスが下の層まで落ちる秒数
	inline constexpr float kDescentDuration = 0.8f; //プレイヤーが下の層へ降りる秒数
	inline constexpr float kDescentHopHeight = 1.6f; //降りるときに跳ねる高さ
	inline constexpr float kLandingImpactDuration = 0.5f; //ボスが落ちてから足場が飛んでいくまでの秒数
	inline constexpr float kTileFlightDuration = 0.8f; //足場が1枚ボスまで飛ぶ秒数
	inline constexpr float kTileLaunchWindow = 1.0f; //全部の足場が飛び立ち終わるまでの秒数

	//ボスの体力とダメージ
	inline constexpr int32_t kBossMaxHealth = 2000;
	inline constexpr int32_t kBossFallDamage = 100; //ボスが下の層へ落ちたときのダメージ
	inline constexpr int32_t kDamagePerRemainingTile = 1; //残った足場1枚のダメージで、穴のとなりは4倍
	inline constexpr int32_t kMaxTileDamage = 400; //1回の落下で足場が与えるダメージの上限
	inline constexpr float kMaxBossSpeedBonus = 0.3f; //床が全部消えたときにボスが何割速くなるか

	//プレイヤー
	inline constexpr float kJumpHeight = 1.5f; //1マス跳ぶときの高さ
	inline constexpr float kJumpDuration = 0.48f; //1マス跳ぶ秒数
	inline constexpr float kGravity = 20.0f; //重力の強さ
	inline constexpr float kFallThreshold = 1.1f; //床よりこれだけ下まで落ちたら落ちたことにする
	inline constexpr float kPlayerSize = 0.78f; //仮のモデルの箱の1辺
	inline constexpr float kPlayerContactRadius = 0.39f; //床のふちとボスとの当たりに使う体の半径
	inline constexpr float kPlayerContactHeight = 1.3f; //ボスとの当たりに使う体の高さ

	//ハート
	inline constexpr int32_t kMaxHearts = 5;
	inline constexpr int32_t kFallDamage = 1; //穴から落ちたときのダメージ
	inline constexpr int32_t kDescentHeal = 1; //下の層へ降りたときに回復する数
	inline constexpr float kInvincibilityTime = 1.0f; //当たったあと無敵の秒数
	inline constexpr float kHitBlinkFrequency = 9.0f; //無敵の間に1秒に点滅する回数

	//ボスに触れたとき
	inline constexpr float kBossContactSizeX = 1.55f; //当たりに使うボスの体の大きさ
	inline constexpr float kBossContactSizeY = 1.5f;
	inline constexpr float kBossContactSizeZ = 1.45f;
	inline constexpr float kContactKnockbackCells = 2.0f; //はじかれるマスの数
	inline constexpr float kContactLiftSpeed = 3.0f; //上へ浮く速さ
	inline constexpr float kContactPushDuration = 0.32f; //はじかれている秒数

	//仮のモデルにかける色
	inline constexpr RyoEngine::Vector4 kFloorColor = { 0.329f, 0.435f, 0.557f, 1.0f }; //ふつうの床
	inline constexpr RyoEngine::Vector4 kWarningColor = { 1.0f, 0.675f, 0.329f, 1.0f }; //パンチの範囲
	inline constexpr RyoEngine::Vector4 kIsolatedColor = { 0.925f, 0.349f, 0.459f, 1.0f }; //切り離された床
	inline constexpr RyoEngine::Vector4 kBonusColor = { 0.05f, 0.74f, 0.91f, 1.0f }; //飛ばすと4倍になる穴のとなりの床
	inline constexpr RyoEngine::Vector4 kPlayerColor = { 1.0f, 1.0f, 1.0f, 1.0f }; //木箱の色そのまま
	inline constexpr RyoEngine::Vector4 kPlayerFrontColor = { 0.188f, 0.82f, 0.725f, 1.0f }; //向きがわかるプレイヤーの正面の板
	inline constexpr RyoEngine::Vector4 kBossColor = { 0.941f, 0.392f, 0.349f, 1.0f }; //ボスの体
	inline constexpr RyoEngine::Vector4 kBossArmorColor = { 0.525f, 0.173f, 0.286f, 1.0f }; //パンチの拳
	inline constexpr RyoEngine::Vector4 kAttackColor = { 1.0f, 0.753f, 0.294f, 1.0f }; //近接攻撃の予告

	//画面の右上の形を選ぶ輪
	//画面の位置は真ん中が0で右と上のはしが1
	inline constexpr float kRingCenterX = 0.78f; //輪の真ん中の横の位置
	inline constexpr float kRingCenterY = 0.7f; //輪の真ん中の縦の位置
	inline constexpr float kRingRadiusX = 0.14f; //輪の横の半径
	inline constexpr float kRingRadiusY = 0.1f; //輪の縦の半径で、小さいほどつぶれた楕円
	inline constexpr float kRingCellSize = 0.65f; //手前の形の1マスの大きさ
	inline constexpr float kRingBackScale = 0.55f; //手前に対する一番奥の形の大きさ
	inline constexpr float kRingBackBrightness = 0.45f; //手前に対する一番奥の形の明るさ
	inline constexpr float kRingTurnDuration = 0.15f; //1つ分回る秒数
	inline constexpr float kPanelDepth = 8.0f; //カメラから手前の形までの距離
	inline constexpr float kRingDepthSpread = 4.0f; //奥の形ほど遠くに置く距離
	inline constexpr float kPanelBlockFill = 0.9f; //1マスに対するブロックの大きさ
	inline constexpr float kPanelBlockHeight = 0.5f; //1マスに対するブロックの厚さ
	inline constexpr float kPanelWaitBrightness = 0.3f; //ボスの番の間のNOWの明るさ

	//[アイテム] アイテムは一旦使わないのでコメントアウト
	// //HPが少ないとき1層に1個だけ降ってくるアイテム
	// inline constexpr int32_t kItemSpawnHearts = 3;          //HPがこれ以下になったら降ってくる
	// inline constexpr float kItemSize = 0.5f;                //箱の1辺
	// inline constexpr float kItemHover = 0.35f;              //床から箱の下の面まで浮かせる高さ
	// inline constexpr float kItemDropHeight = 10.0f;         //床から降ってくる高さ
	// inline constexpr float kItemDropDuration = 0.7f;        //降ってくる秒数
	// inline constexpr float kItemSpinSpeed = 120.0f;         //1秒に回る度数
	// inline constexpr float kItemTilt = 30.0f;               //度の傾き
	// inline constexpr float kItemFallTime = 0.5f;            //足場と一緒に落ちて消えるまでの秒数
	// inline constexpr RyoEngine::Vector4 kCrossBombColor = { 1.0f, 0.25f, 0.75f, 1.0f }; //ピンクの十字爆弾
	// inline constexpr RyoEngine::Vector4 kJumpBoostColor = { 0.45f, 1.0f, 0.35f, 1.0f }; //黄緑の2マス乗り越え

	// //十字爆弾
	// inline constexpr float kBombStepDelay = 0.1f;           //1マス外へ行くごとに崩れるのが遅れる秒数

	//ジャンプ
	inline constexpr int32_t kNormalJumpGap = 1; //いつも飛び越えられる穴の数

	//2マス乗り越え
	//[アイテム] アイテムは一旦使わないのでコメントアウト
	// inline constexpr int32_t kBoostedJumpGap = 2;           //アイテムを取ったあと飛び越えられる穴の数
	// //trueなら次のジャンプで使い切り、falseなら穴2つを飛び越えたときだけ使い切る
	// inline constexpr bool kJumpBoostUseOnNextJump = false;

	//仮のモデル
	inline constexpr const char* kCubeModel = "cube"; //Kari/Model/cube.obj
	inline constexpr float kCubeModelSize = 2.0f; //cube.objの1辺の長さ
}
