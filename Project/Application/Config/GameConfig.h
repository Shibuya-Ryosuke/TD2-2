#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>

namespace App::GameConfig
{
	//ゲーム全体の数値

	//床と長さ
	inline constexpr float kWorldScale = 1.25f; //タイルを大きくするために床とボスの届く距離などの長さにかける倍率
	inline constexpr float kFloorSize = 20.0f * kWorldScale; //床全体の1辺の長さ
	inline constexpr float kTileThickness = 0.45f; //タイルの厚さ
	inline constexpr float kTileGap = 0.055f; //1マスに対するタイルのすき間の割合
	inline constexpr float kFloorSpacing = 12.0f; //層と層の間の高さ
	inline constexpr float kLowerFloorBrightness = 0.25f; //下の層の明るさ
	inline constexpr int32_t kLayerFileCount = 5; //層のCSVの数で、6層目からは1層目に戻る
	inline constexpr int32_t kFirstLayerWidth = 10; //1層目のマスの数
	inline constexpr int32_t kFirstLayerDepth = 10;

	//ゲームを始めるときのボスのマスを1層目のマスで指定する
	inline constexpr int32_t kBossSpawnX = 6;
	inline constexpr int32_t kBossSpawnZ = 6;
	inline constexpr float kMinSpawnDistanceTiles = 5.0f; //層が始まるときにプレイヤーをボスから離すマスの数

	//床を崩す
	inline constexpr float kCollapseDelay = 1.5f; //崩してから穴になるまでの秒数
	inline constexpr float kShapePreviewBlend = 0.6f; //床に出す正面1マスの予告の色の濃さ
	inline constexpr RyoEngine::Vector4 kBreakPreviewColor = { 0.2f, 0.9f, 0.95f, 1.0f }; //正面1マスの予告の水色
	inline constexpr float kPreviewGlowMin = 0.8f; //予告のマスの光のいちばん弱いとき
	inline constexpr float kPreviewGlowMax = 1.8f; //予告のマスの光のいちばん強いとき
	inline constexpr float kPreviewGlowSpeed = 1.5f; //予告のマスが1秒に明滅する回数

	//切り離し
	inline constexpr float kEscapeDelay = 2.0f; //切り離されてから落ちるまでの秒数
	inline constexpr float kShakeDistance = 0.12f; //切り離された床の揺れ幅
	inline constexpr float kShakeFrequency = 13.0f; //切り離された床が1秒に揺れる回数

	//下の層へ
	inline constexpr float kBossDropDuration = 0.8f; //ボスが下の層まで落ちる秒数
	inline constexpr float kDescentDuration = 0.8f; //プレイヤーが下の層へ降りる秒数
	inline constexpr float kDescentHopHeight = 1.6f * kWorldScale; //降りるときに跳ねる高さ
	inline constexpr float kLandingImpactDuration = 0.5f; //ボスが落ちてから足場が飛んでいくまでの秒数
	inline constexpr float kTileFlightDuration = 0.8f; //足場が1枚ボスまで飛ぶ秒数
	inline constexpr float kTileLaunchWindow = 1.0f; //1つのまとまりの足場が飛び立ち終わるまでの秒数
	inline constexpr float kTileBarrageGroupPause = 0.25f; //穴のとなりの床を飛ばし終わってからふつうの床を飛ばすまでの秒数

	//ボスの体力とダメージ
	inline constexpr int32_t kBossMaxHealth = 2000;
	inline constexpr int32_t kBossFallDamage = 100; //ボスが下の層へ落ちたときのダメージ
	inline constexpr int32_t kOrdinaryTileDamage = 4; //ふつうの足場1枚のダメージ
	inline constexpr int32_t kAdjacentTileDamage = 1; //穴のとなりのヒビの入った足場1枚のダメージ
	inline constexpr int32_t kMaxTileDamage = 400; //1回の落下で足場が与えるダメージの上限
	inline constexpr float kMaxBossSpeedBonus = 0.3f; //床が全部消えたときにボスが何割速くなるか

	//プレイヤー
	inline constexpr float kJumpHeight = 1.5f * kWorldScale; //となりのマスへ跳ぶときの高さ
	inline constexpr float kJumpDuration = 0.48f; //となりのマスへ跳ぶ秒数
	inline constexpr float kGravity = 20.0f; //重力の強さ
	inline constexpr float kFallThreshold = 1.1f; //床よりこれだけ下まで落ちたら落ちたことにする
	inline constexpr float kPlayerSize = 0.78f; //仮の箱だったときの1辺でアイテムの当たりに使う
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
	inline constexpr int32_t kContactDamage = 1; //ボスに触れたときに減るハート

	//仮のモデルにかける色
	inline constexpr RyoEngine::Vector4 kFloorColor = { 0.329f, 0.435f, 0.557f, 1.0f }; //ふつうの床
	inline constexpr RyoEngine::Vector4 kWarningColor = { 1.0f, 0.675f, 0.329f, 1.0f }; //パンチの範囲
	inline constexpr RyoEngine::Vector4 kIsolatedColor = { 0.925f, 0.349f, 0.459f, 1.0f }; //切り離された床
	inline constexpr RyoEngine::Vector4 kBonusColor = { 0.05f, 0.74f, 0.91f, 1.0f }; //ヒビが入って飛ばすと1ダメージの穴のとなりの床
	inline constexpr RyoEngine::Vector4 kBossFistColor = { 0.194f, 0.401f, 0.162f, 1.0f }; //ボスのモデルの手と同じ緑のパンチの拳
	inline constexpr RyoEngine::Vector4 kAttackColor = { 1.0f, 0.753f, 0.294f, 1.0f }; //近接攻撃の予告

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

	//ボスの移動
	inline constexpr int32_t kBossJumpGap = 1; //ボスが飛び越えられる穴の数で、たどり着けるかの判定に使う

	//2マス乗り越え
	//[アイテム] アイテムは一旦使わないのでコメントアウト
	// inline constexpr int32_t kBoostedJumpGap = 2;           //アイテムを取ったあと飛び越えられる穴の数
	// //trueなら次のジャンプで使い切り、falseなら穴2つを飛び越えたときだけ使い切る
	// inline constexpr bool kJumpBoostUseOnNextJump = false;

	//プレイヤーとボスのモデルで色はmtlのまま使う
	inline constexpr const char* kPlayerModel = "animation/Player/playerIdol.glb"; //待機のアニメーションが入ったボーンつきのプレイヤーのモデル
	inline constexpr const char* kBossModel = "Model/boss/boss.obj"; //ボスのモデル
	inline constexpr float kPlayerModelScale = 0.75f; //プレイヤーのモデルの大きさの倍率
	inline constexpr RyoEngine::Vector3 kBossModelScale = { 1.0f, 1.0f, 1.0f }; //モデルのままのボスの大きさの倍率
	inline constexpr float kPlayerModelYaw = 90.0f; //横を向いているプレイヤーのモデルを正面に向ける度数
	inline constexpr float kBossModelYaw = 90.0f; //横を向いているボスのモデルを正面に向ける度数
	inline constexpr float kPlayerModelLift = 0.04f; //足の裏が床に来るようにプレイヤーのモデルを上げる高さ

	//ゲームシーンの背景の天球
	inline constexpr const char* kSkyDomeModel = "Model/SkyDome/SkyDome.obj"; //内側から見る天球のモデル
	inline constexpr float kSkyDomeSpan = 1.45f; //画面の何倍の大きさの楕円にするかで、小さいほど縦にピンクから水色まで広く映る
	inline constexpr float kSkyDomeDepth = 60.0f; //カメラから天球の奥までの長さでカメラの遠い面より手前
	inline constexpr float kSkyScrollSpeed = 0.004f; //1秒に雲が横に流れる量で1で1周
	inline constexpr float kSkyTextureFill = 0.97f; //画像の上から下までのうち画面の上から下に入れる割合で1より大きいとはしが反対側に回り込む

	//仮のモデル
	inline constexpr const char* kCubeModel = "cube"; //Kari/Model/cube.obj
	inline constexpr float kCubeModelSize = 2.0f; //cube.objの1辺の長さ
}
