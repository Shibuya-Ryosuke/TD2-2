#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace App::DirectionConfig
{
	//演出の時間と色と強さの数値

	//場面切り替えの積み木
	inline constexpr const char* kTransitionBlockShader = "TransitionBlock"; //積み木と紙吹雪のシェーダー
	inline constexpr const char* kGustLinesShader = "GustLines"; //吹き飛ぶときの横風のポストエフェクト
	inline constexpr float kTransitionDepth = 20.0f; //カメラから積み木の壁までの距離
	inline constexpr float kTransitionNearHide = 0.5f; //これよりカメラに近づいたブロックは消す
	inline constexpr float kTransitionMaxDeltaTime = 1.0f / 30.0f; //読み込み直後に一気に進まないよう1フレームで進める秒数の上限
	//がれき
	inline constexpr int32_t kTransitionDebrisCount = 20; //がれきの数
	inline constexpr float kTransitionDebrisTime = 0.35f; //がれきが落ちきるまでの秒数
	inline constexpr float kTransitionDebrisSpread = 0.12f; //落ち始めのばらつきの秒数
	inline constexpr float kTransitionDebrisMinSize = 0.2f; //積み木の高さに対する大きさの最小
	inline constexpr float kTransitionDebrisSizeRange = 0.2f; //大きさのばらつき
	inline constexpr float kTransitionDebrisSway = 0.6f; //紙吹雪が左右にひらひらする幅
	inline constexpr float kTransitionDebrisSwaySpeed = 30.0f; //ひらひらの速さ
	//紙吹雪とドン！の縁の光に使うパステルの色
	inline constexpr std::array<RyoEngine::Vector4, 6> kTransitionPopColors = { {
		{ 1.0f, 0.55f, 0.75f, 1.0f }, //ピンク
		{ 1.0f, 0.85f, 0.35f, 1.0f }, //きいろ
		{ 0.5f, 0.95f, 0.7f, 1.0f }, //ミント
		{ 0.5f, 0.8f, 1.0f, 1.0f }, //みずいろ
		{ 0.78f, 0.6f, 1.0f, 1.0f }, //むらさき
		{ 1.0f, 0.68f, 0.45f, 1.0f }, //オレンジ
	} };
	//積み上がる
	inline constexpr int32_t kTransitionColumns = 5; //横の数
	inline constexpr int32_t kTransitionRows = 5; //縦の数
	inline constexpr float kTransitionBlockGap = 0.015f; //マスに対する積み木どうしのすき間
	inline constexpr float kTransitionStackTime = 0.9f; //25個積み上がるまでの秒数
	inline constexpr float kTransitionBlockFallTime = 0.15f; //1個が落ちてくる秒数
	inline constexpr float kTransitionGlowTime = 0.2f; //ドン！の縁の光が消えるまでの秒数
	inline constexpr float kTransitionSquash = 0.55f; //ドン！で高さに対してつぶれる量
	inline constexpr float kTransitionSquashDamping = 8.0f; //ぽよよんが収まる速さ
	inline constexpr float kTransitionSquashSpeed = 36.0f; //ぽよよんの細かさ
	inline constexpr float kTransitionSquashWiden = 0.7f; //つぶれたとき横にふくらむ量
	inline constexpr float kTransitionFallStretch = 0.4f; //落ちている間に縦にのびる量
	inline constexpr float kTransitionHoldTime = 0.1f; //埋まってから次のシーンへ行くまでの秒数
	//次のシーンで吹き飛ぶ間は操作させない
	inline constexpr float kTransitionBlowTime = 0.9f; //全部吹き飛ぶまでの秒数
	inline constexpr float kTransitionBlowColumnDelay = 0.06f; //風は左からなので1列右ごとに飛び始めが遅れる秒数
	inline constexpr float kTransitionBlowRandomDelay = 0.08f; //飛び始めのばらつきの秒数
	inline constexpr float kTransitionBlowDistance = 1.2f; //画面の外へ出てからさらに飛ぶ長さ
	inline constexpr float kTransitionBlowLift = 0.35f; //ふわっと上下に流れる量
	inline constexpr float kGustLinesStrength = 1.0f; //横風の線の強さ

	//床が崩れる演出
	//ハートがポンっ、ガン！でヒビ、ガン！でぱっかーん、ガタガタ、ひびにそって砕ける
	inline constexpr const char* kFloorTileShader = "FloorTile";
	inline constexpr RyoEngine::Vector3 kHeartColor = { 1.0f, 0.3f, 0.62f }; //飛び散る欠片の光の色
	inline constexpr float kHeartPopTime = 0.3f; //ハートがポンっと出て落ち着くまでの秒数
	inline constexpr std::array<float, 2> kHammerHits = { 0.35f, 0.62f }; //崩れるまでの時間に対するガン！2回のタイミング
	inline constexpr float kHeartCrackTime = 0.15f; //1回目のガン！でヒビが走りきるまでの秒数
	inline constexpr float kHeartBreakTime = 0.3f; //2回目のガン！でハートが割れて消えるまでの秒数
	inline constexpr float kTileCrackTime = 0.12f; //2回目のガン！でひびがタイル全体へ走りきるまでの秒数
	inline constexpr float kHammerFlashDecay = 12.0f; //ガン！の光が消える速さ
	inline constexpr float kHammerJolt = 0.1f; //ガン！でタイルがビクッと沈む量
	inline constexpr float kCrumbleShakeTime = 0.45f; //穴になる何秒前からガタガタ揺れるか
	inline constexpr float kCrumbleShakeDistance = 0.08f; //ガタガタのいちばん大きい揺れ幅
	inline constexpr float kCrumbleShakeSpeed = 55.0f; //ガタガタの速さ
	inline constexpr float kCrumblePuff = 0.12f; //穴になる直前にぷくっとふくらむ量
	inline constexpr float kDropPop = 0.15f; //落ち始めにぽんっとふくらむ量
	inline constexpr float kDropPopTime = 0.25f; //落ちる時間に対するぽんっの長さ
	inline constexpr float kIsolatedDissolveStart = 0.3f; //切り離された床がどこまで落ちたら砕け始めるか
	//砕けたときに飛び散るピンクの欠片
	inline constexpr size_t kShardPoolSize = 96; //使い回す欠片の数
	inline constexpr int32_t kShardsPerTile = 6; //1マスで飛ぶ欠片の数
	inline constexpr float kShardSize = 0.2f; //タイルの幅に対する欠片の大きさ
	inline constexpr float kShardSpeed = 3.0f; //外へ飛ぶ速さ
	inline constexpr float kShardJump = 5.0f; //上へはじける速さ
	inline constexpr float kShardGravity = 20.0f; //落ちる強さ
	inline constexpr float kShardLife = 0.6f; //消えるまでの秒数
	inline constexpr float kShardGlow = 0.8f; //欠片の光の強さ
	inline constexpr RyoEngine::Vector4 kShardColor = { 1.0f, 0.62f, 0.8f, 1.0f }; //欠片の色

	//見た目だけのプレイヤーのぽよん
	inline constexpr const char* kPlayerPopShader = "PlayerPop"; //着地でふちがピカッと光るシェーダー
	inline constexpr float kPlayerStretchPerSpeed = 0.012f; //上下の速さ1あたりに縦にのびる量
	inline constexpr float kPlayerStretchMax = 0.3f; //縦にのびる量の上限
	inline constexpr float kPlayerStretchFollow = 18.0f; //のびが速さに追いつく速さ
	inline constexpr float kPlayerLandSpeedRef = 10.0f; //この速さの着地をふつうの着地にする
	inline constexpr float kPlayerLandMinSpeed = 3.0f; //これより遅い着地はぽよんしない
	inline constexpr float kPlayerStopSpeed = 2.0f; //着地したとみなす止まった速さ
	inline constexpr float kPlayerTeleportDistance = 2.0f; //1フレームでこれより動いたら置き直しとみなす
	inline constexpr float kPlayerLandSquash = 0.35f; //ふつうの着地でつぶれる量
	inline constexpr float kPlayerLandSquashMax = 0.5f; //層を降りたときの大きい着地でつぶれる量の上限
	inline constexpr float kPlayerSquashDamping = 9.0f; //ぽよよんが収まる速さ
	inline constexpr float kPlayerSquashSpeed = 34.0f; //ぽよよんの速さ
	inline constexpr float kPlayerSquashWiden = 0.6f; //つぶれたとき横にふくらむ量
	inline constexpr float kPlayerLandGlow = 1.0f; //着地のピカッの強さ
	inline constexpr float kPlayerLandGlowDecay = 7.0f; //ピカッが消える速さ
	//着地のほこり
	inline constexpr size_t kDustPoolSize = 40; //使い回すほこりの数
	inline constexpr int32_t kDustPerLanding = 8; //ふつうの着地で出る数
	inline constexpr float kDustSize = 0.14f; //大きさ
	inline constexpr float kDustSpeed = 2.6f; //外へ広がる速さ
	inline constexpr float kDustLift = 1.2f; //上へふわっと上がる速さ
	inline constexpr float kDustDrag = 5.0f; //だんだん遅くなる強さ
	inline constexpr float kDustLife = 0.45f; //消えるまでの秒数

	//ダメージを受けたときの画面のふち
	inline constexpr const char* kDamageVignetteShader = "DamageVignette";
	inline constexpr float kDamageFlashRise = 0.05f; //パッと光るまでの秒数
	inline constexpr float kDamageFlashTime = 0.4f; //光り始めてからすっと消えるまでの秒数
	inline constexpr float kDamageFlashStrength = 1.0f; //シェーダーの値に掛ける強さ

	//層を降りるときに下から上へ流れる線
	inline constexpr const char* kFallLinesShader = "FallLines";
	inline constexpr float kFallLinesRise = 0.15f; //線が出きるまでの秒数
	inline constexpr float kFallLinesFade = 0.3f; //降り終わってから消えるまでの秒数
	inline constexpr float kFallLinesStrength = 1.0f; //線の強さ

	//着地したときの足場のぷるるん
	inline constexpr float kJiggleRadius = 1.5f; //揺れが広がるマスの数
	inline constexpr float kJiggleRippleDelay = 0.05f; //1マス離れるごとに揺れ始めが遅れる秒数
	inline constexpr float kJiggleDepth = 0.12f; //沈む深さ
	inline constexpr float kJiggleSquash = 0.35f; //縦につぶれる量
	inline constexpr float kJiggleWiden = 0.08f; //横にふくらむ量
	inline constexpr float kJiggleDamping = 7.0f; //ぷるぷるが収まる速さ
	inline constexpr float kJiggleSpeed = 32.0f; //ぷるぷるの速さ
	inline constexpr float kJigglePlayerMax = 1.8f; //プレイヤーの着地の強さの上限
	inline constexpr float kJiggleBossPower = 1.6f; //重いボスの着地の強さ

	//ボスの次の行動を見せる半透明の残像
	inline constexpr RyoEngine::Vector4 kBossGhostColor = { 0.3f, 0.85f, 1.0f, 0.3f };
	inline constexpr float kBossGhostMinTime = 0.7f; //1つの行動を見せるいちばん短い秒数
	inline constexpr float kBossGhostMaxTime = 1.8f; //1つの行動を見せるいちばん長い秒数
	inline constexpr float kBossGhostTimeScale = 0.45f; //本当の時間に対する見せる時間の割合
	inline constexpr float kBossGhostRefresh = 0.1f; //先読みを作り直す間隔の秒数
}
