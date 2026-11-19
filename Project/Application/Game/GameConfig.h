#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>

namespace App::GameConfig
{
	// ゲーム全体の数値

	//床
	inline constexpr float kFloorSize = 20.0f;          // 床全体の1辺の長さ (マスの数が減っても同じ。1層目は 2.0 x 10 マス)
	inline constexpr float kTileThickness = 0.45f;      // タイルの厚さ
	inline constexpr float kTileGap = 0.055f;           // タイルのすき間 (1マスに対する割合)
	inline constexpr float kFloorSpacing = 12.0f;       // 層と層の間の高さ
	inline constexpr float kLowerFloorBrightness = 0.25f; // 下の層の明るさ
	inline constexpr int32_t kLayerFileCount = 5;       // 層の CSV の数 (layer1.csv 〜 layer5.csv。6層目からは1層目に戻る)
	inline constexpr int32_t kFirstLayerWidth = 10;     // 1層目のマスの数 (出てくる位置を、層が変わっても同じ割合の所にするため)
	inline constexpr int32_t kFirstLayerDepth = 10;

	//出てくるマス (1層目のマスで指定。下の層では同じ割合の所になる)
	inline constexpr int32_t kPlayerSpawnX = 1;
	inline constexpr int32_t kPlayerSpawnZ = 1;
	inline constexpr int32_t kBossSpawnX = 6;
	inline constexpr int32_t kBossSpawnZ = 6;

	//床を崩す (テトリスの形)
	inline constexpr float kCollapseDelay = 2.6f;           // 崩してから穴になるまで(秒)
	inline constexpr float kCollapseWarningDuration = 0.95f; // 穴になる前、この時間から点滅する(秒)
	inline constexpr float kCollapseRedDuration = 0.26f;     // 穴になる前、この時間から赤くなる(秒)
	inline constexpr float kCollapseBlinkFrequency = 4.0f;   // 点滅の速さ (1秒に何回)
	inline constexpr float kShapePreviewBlend = 0.6f;        // 床に出す形の予告の色の濃さ (0 〜 1)
	inline constexpr float kPreviewGlowMin = 0.8f;           // 予告のマスの光の強さ (いちばん弱いとき)
	inline constexpr float kPreviewGlowMax = 1.8f;           // 予告のマスの光の強さ (いちばん強いとき。1 を超えるとブルームでにじむ)
	inline constexpr float kPreviewGlowSpeed = 1.5f;         // 予告のマスの明滅の速さ (1秒に何回)
	inline constexpr float kShapeCooldown = 1.0f;            // 形を使ってから次を使えるまで(秒。待っている間はパネルの NOW が暗くなる)
	inline constexpr bool kShuffleShapes = true;             // 形の順番を混ぜるか

	//切り離し
	inline constexpr float kEscapeDelay = 2.0f;         // 切り離されてから落ちるまで(秒)
	inline constexpr float kShakeDistance = 0.12f;      // 切り離された床の揺れ幅
	inline constexpr float kShakeFrequency = 13.0f;     // 切り離された床の揺れの速さ (1秒に何回)

	//下の層へ
	inline constexpr float kBossDropDuration = 0.8f;    // ボスが下の層まで落ちる時間(秒)
	inline constexpr float kDescentDuration = 0.8f;     // プレイヤーが下の層へ降りる時間(秒)
	inline constexpr float kDescentHopHeight = 1.6f;    // 降りるときに跳ねる高さ
	inline constexpr float kLandingImpactDuration = 0.5f; // ボスが落ちてから、足場が飛んでいくまでの待ち(秒)
	inline constexpr float kTileFlightDuration = 0.8f;  // 足場が1枚ボスまで飛ぶ時間(秒)
	inline constexpr float kTileLaunchWindow = 1.0f;    // 全部の足場が飛び立ち終わるまでの時間(秒)

	//ボスの体力・ダメージ
	inline constexpr int32_t kBossMaxHealth = 2000;
	inline constexpr int32_t kBossFallDamage = 100;      // ボスが下の層へ落ちたときのダメージ
	inline constexpr int32_t kDamagePerRemainingTile = 1; // 残った足場1枚のダメージ (穴のとなりの足場は 4 倍)
	inline constexpr int32_t kMaxTileDamage = 400;       // 足場のダメージの上限 (1回の落下につき)
	inline constexpr float kMaxBossSpeedBonus = 0.3f;    // 床が全部消えたときに、ボスが何割速くなるか

	//プレイヤー (1マスずつ跳んで動く)
	inline constexpr float kJumpHeight = 1.5f;           // 1マス跳ぶときの高さ
	inline constexpr float kJumpDuration = 0.48f;        // 1マス跳ぶ時間(秒)
	inline constexpr float kGravity = 20.0f;             // 下向きの強さ
	inline constexpr float kFallThreshold = 1.1f;        // 床の高さよりこれだけ下まで落ちたら「落ちた」
	inline constexpr float kPlayerSize = 0.78f;          // 箱の1辺 (仮のモデル)
	inline constexpr float kPlayerContactRadius = 0.39f; // 体の半径 (床のふち・ボスとの当たり)
	inline constexpr float kPlayerContactHeight = 1.3f;  // 体の高さ (ボスとの当たり)

	//ハート
	inline constexpr int32_t kMaxHearts = 5;
	inline constexpr int32_t kFallDamage = 1;            // 穴から落ちたときのダメージ
	inline constexpr int32_t kDescentHeal = 1;           // 下の層へ降りたときに回復する数
	inline constexpr float kInvincibilityTime = 1.0f;    // 当たったあと無敵の時間(秒)
	inline constexpr float kHitBlinkFrequency = 9.0f;    // 無敵の間の点滅 (1秒に何回)

	//ボスに触れたとき
	inline constexpr float kBossContactSizeX = 1.55f;    // ボスの体の大きさ (当たり)
	inline constexpr float kBossContactSizeY = 1.5f;
	inline constexpr float kBossContactSizeZ = 1.45f;
	inline constexpr float kContactKnockbackCells = 2.0f; // はじかれる距離 (マスの数)
	inline constexpr float kContactLiftSpeed = 3.0f;     // 上へ浮く速さ
	inline constexpr float kContactPushDuration = 0.32f; // はじかれている時間(秒)

	// 色 (Unity のプロトのマテリアルの色と同じにしてます！！仮のモデル cube にかける)
	inline constexpr RyoEngine::Vector4 kFloorColor = { 0.329f, 0.435f, 0.557f, 1.0f };   // ふつうの床
	inline constexpr RyoEngine::Vector4 kWarningColor = { 1.0f, 0.675f, 0.329f, 1.0f };   // パンチの範囲
	inline constexpr RyoEngine::Vector4 kSteppedTileColor = { 1.0f, 0.39f, 0.05f, 1.0f };   // 崩す予定の床
	inline constexpr RyoEngine::Vector4 kCollapseBlinkColor = { 1.0f, 0.82f, 0.04f, 1.0f }; // 崩れる前の点滅
	inline constexpr RyoEngine::Vector4 kCollapseRedColor = { 1.0f, 0.08f, 0.06f, 1.0f };   // 崩れる直前
	inline constexpr RyoEngine::Vector4 kIsolatedColor = { 0.925f, 0.349f, 0.459f, 1.0f }; // 切り離された床
	inline constexpr RyoEngine::Vector4 kBonusColor = { 0.05f, 0.74f, 0.91f, 1.0f };      // 穴のとなりの床 (飛ばすと 4 倍)
	inline constexpr RyoEngine::Vector4 kPlayerColor = { 1.0f, 1.0f, 1.0f, 1.0f };        // 白 = 木箱の色そのまま
	inline constexpr RyoEngine::Vector4 kPlayerFrontColor = { 0.188f, 0.82f, 0.725f, 1.0f }; // プレイヤーの正面の板 (向きがわかるように)
	inline constexpr RyoEngine::Vector4 kBossColor = { 0.941f, 0.392f, 0.349f, 1.0f };    // ボスの体 (赤)
	inline constexpr RyoEngine::Vector4 kBossArmorColor = { 0.525f, 0.173f, 0.286f, 1.0f }; // パンチの拳
	inline constexpr RyoEngine::Vector4 kAttackColor = { 1.0f, 0.753f, 0.294f, 1.0f };    // 近接攻撃の予告

	//次の形のパネル (画面の右はし。パネル : ステージ = 1 : 9)
	//画面の位置は「画面の真ん中が 0、右はし・上はしが 1、左はし・下はしが -1」で決める

	inline constexpr float kPanelWidthRate = 0.1f;          // 画面の横幅に対するパネルの幅 (1 : 9 なので 0.1)
	inline constexpr float kPanelDepth = 8.0f;              // カメラからブロックまでの距離 (ステージより手前に置く)
	inline constexpr std::array<float, 3> kPanelSlotY = { 0.55f, 0.02f, -0.45f };    // 今・次・その次 の縦の位置
	inline constexpr std::array<float, 3> kPanelCellSize = { 0.9f, 0.7f, 0.7f };     // 今・次・その次 の1マスの大きさ
	inline constexpr float kPanelLabelOffsetY = 0.27f;      // 形の上に出す文字 (NOW / NEXT) の高さ
	inline constexpr float kPanelBlockFill = 0.9f;          // ブロックの大きさ (1マスに対する割合。残りがすき間)
	inline constexpr float kPanelBlockHeight = 0.5f;        // ブロックの厚さ (1マスに対する割合)
	inline constexpr float kPanelWaitBrightness = 0.3f;     // 形を使えない間の NOW の明るさ (1 でいつもと同じ)

	//アイテム (HP が少ないとき、1層に1個だけ上から降ってくる)
	inline constexpr int32_t kItemSpawnHearts = 3;          // HP がこれ以下になったら降ってくる
	inline constexpr float kItemSize = 0.5f;                // 箱の1辺
	inline constexpr float kItemHover = 0.35f;              // 床から浮かせる高さ (箱の下の面まで)
	inline constexpr float kItemDropHeight = 10.0f;         // 降ってくる高さ (床から)
	inline constexpr float kItemDropDuration = 0.7f;        // 降ってくる時間(秒)
	inline constexpr float kItemSpinSpeed = 120.0f;         // 回る速さ (度 / 秒)
	inline constexpr float kItemTilt = 30.0f;               // 傾き (度)
	inline constexpr float kItemFallTime = 0.5f;            // 足場と一緒に落ちて消えるまで(秒)
	inline constexpr RyoEngine::Vector4 kCrossBombColor = { 1.0f, 0.25f, 0.75f, 1.0f }; // 十字爆弾 (ピンク)
	inline constexpr RyoEngine::Vector4 kJumpBoostColor = { 0.45f, 1.0f, 0.35f, 1.0f }; // 2マス乗り越え (黄緑)

	//十字爆弾
	inline constexpr float kBombStepDelay = 0.1f;           // 真ん中から1マス外へ行くごとに、崩れるのが遅れる時間(秒)

	//2マス乗り越え
	inline constexpr int32_t kNormalJumpGap = 1;            // いつも飛び越えられる穴の数
	inline constexpr int32_t kBoostedJumpGap = 2;           // アイテムを取ったあと飛び越えられる穴の数
	// true: 取ったあとの「次のジャンプ」で使い切る (ふつうの1マス移動でも消える)
	// false: 「穴2つを実際に飛び越えたとき」だけ使い切る
	inline constexpr bool kJumpBoostUseOnNextJump = false;

	//仮のモデル
	inline constexpr const char* kCubeModel = "cube";    // Kari/Model/cube.obj (1辺が 2)
	inline constexpr float kCubeModelSize = 2.0f;        // cube.obj の1辺の長さ (大きさを合わせるのに使う)
}
