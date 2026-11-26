#pragma once
#include <RyoEngine.h>
#include <cstdint>

//ボスの体力とダメージと当たりとモデル
namespace App::BossConfig
{
	//体力とダメージ
	inline constexpr int32_t kBossMaxHealth = 20;
	inline constexpr int32_t kBossFallDamage = 100; //ボスが下の層へ落ちたときのダメージ
	inline constexpr int32_t kOrdinaryTileDamage = 4; //ふつうの足場1枚のダメージ
	inline constexpr int32_t kAdjacentTileDamage = 1; //穴のとなりのヒビの入った足場1枚のダメージ
	inline constexpr int32_t kMaxTileDamage = 400; //1回の落下で足場が与えるダメージの上限
	inline constexpr float kMaxBossSpeedBonus = 0.3f; //床が全部消えたときにボスが何割速くなるか

	//触れたとき
	inline constexpr float kBossContactSizeX = 1.55f; //当たりに使うボスの体の大きさ
	inline constexpr float kBossContactSizeY = 1.5f;
	inline constexpr float kBossContactSizeZ = 1.45f;
	inline constexpr int32_t kContactDamage = 1; //ボスに触れたときに減るハート

	//移動
	inline constexpr int32_t kBossJumpGap = 1; //ボスが飛び越えられる穴の数で、たどり着けるかの判定に使う

	//チュートリアルの下の層で寝ているボス
	inline constexpr float kBossSleepLift = 0.65f; //仰向けのときに背中が床に埋まらないように上げる高さ
	inline constexpr float kBossSleepCenter = 3.0f; //仰向けのときに体のまん中をマスに合わせる足から頭の半分の長さ
	inline constexpr float kBossSleepBreath = 0.04f; //息で体がふくらむ割合
	inline constexpr float kBossSleepBreathSpeed = 2.5f; //息の速さ
	inline constexpr float kBossFlinchSquash = 0.35f; //足場が当たってつぶれる量
	inline constexpr float kBossFlinchRecover = 4.0f; //つぶれたのが1秒で戻る量

	//攻撃の色
	inline constexpr RyoEngine::Vector4 kWarningColor = { 1.0f, 0.675f, 0.329f, 1.0f }; //パンチの範囲
	inline constexpr RyoEngine::Vector4 kBossFistColor = { 0.194f, 0.401f, 0.162f, 1.0f }; //ボスのモデルの手と同じ緑のパンチの拳
	inline constexpr RyoEngine::Vector4 kAttackColor = { 1.0f, 0.753f, 0.294f, 1.0f }; //近接攻撃の予告

	//モデルで色はmtlのまま使う
	inline constexpr const char* kBossModel = "Model/boss/boss.obj"; //ボスのモデル
	inline constexpr RyoEngine::Vector3 kBossModelScale = { 1.0f, 1.0f, 1.0f }; //モデルのままのボスの大きさの倍率
	inline constexpr float kBossModelYaw = 90.0f; //横を向いているボスのモデルを正面に向ける度数
}
