#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace App {
	//形態で使える攻撃の種類
	enum class BossAttackType : int32_t {
		Rush,        //突進
		GroundPunch, //地面パンチ
		Melee,       //近接攻撃

		Count,
	};

	//攻撃の種類の数
	inline constexpr int32_t kBossAttackTypeCount = static_cast<int32_t>(BossAttackType::Count);

	//エディタに出す攻撃の名前
	inline constexpr const char* kBossAttackNames[kBossAttackTypeCount] = { "突進", "地面パンチ", "近接攻撃" };
	//JSONに書く攻撃の名前
	inline constexpr const char* kBossAttackIds[kBossAttackTypeCount] = { "Rush", "GroundPunch", "Melee" };

	//形態の中で、HPが減ったときに1回だけ発動するもの
	struct BossHpTrigger {
		float hpRate = 0.5f; //この形態のHPがこの割合以下になったら
		BossAttackType action = BossAttackType::GroundPunch; //発動する攻撃
	};

	//形態1つ分の設定
	struct BossPhaseData {
		std::string name = "形態";
		int32_t maxHp = 1000; //この形態のHP。尽きたら次の形態へ

		//移動・判断
		float moveSpeed = 1.5f; //移動の速さ。床の大きさの倍率をかける前の値
		bool canJump = true; //1マス飛び越える
		bool evacuateSmallIsland = false; //今いる島が小さいと大きい島へ先回りして移る
		float thinkInterval = 0.0f; //考える間隔の秒数
		float randomness = 0.0f; //迷い。大きいほど点数の低い行動も選ぶ

		//使える攻撃と選ばれやすさ
		std::array<bool, kBossAttackTypeCount> enabledAttacks{ true, true, true };
		std::array<float, kBossAttackTypeCount> attackWeights{ 1.0f, 1.0f, 1.0f };

		//賢さ。精度は0で使わない、1から10で使う確率
		int32_t avoidIslandSize = 0; //切り離し認知: この数以下の島には入らない
		int32_t isolationAccuracy = 0;
		int32_t historyAccuracy = 0; //プレイヤーの行動履歴を使う
		int32_t detourAccuracy = 0; //あえて遠回りする

		std::vector<BossHpTrigger> triggers; //HPトリガー
	};
}
