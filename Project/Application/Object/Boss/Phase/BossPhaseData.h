#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace App {
	//ボスの攻撃の種類
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

	//コンボの1段
	struct BossComboStep {
		BossAttackType attack = BossAttackType::Rush; //出す攻撃
		int32_t repeat = 1; //続けて何回出すか
		float interval = 0.0f; //1回終わってから次を始めるまでの秒数
	};

	//攻撃の組み合わせ。上の段から順に出す
	struct BossCombo {
		std::string name = "コンボ";
		std::vector<BossComboStep> steps;
	};

	//形態で使うコンボと選ばれやすさ
	struct BossPhaseComboEntry {
		std::string comboName; //コンボの名前で選ぶ
		float weight = 1.0f; //選ばれやすさ
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

		//使うコンボ
		std::vector<BossPhaseComboEntry> combos;
	};
}
