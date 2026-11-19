// [アイテム] アイテムは一旦使わない (チームで相談中)。コメントアウトしている
// (使うときは、このファイルの行頭の "// " を外す)
// #pragma once
// #include <RyoEngine.h>
// #include <cstdint>
// #include <random>
// #include "ItemType.h"
// #include "../Object/FieldItem.h"
// #include "../Stage/Cell.h"

// namespace App
// {
	// class Floor;
	// class Player;

	// // アイテムの出し方と、持っている十字爆弾をまとめたもの
	// // ・戦闘中に HP が kItemSpawnHearts 以下で、この層でまだ出ていなければ、1個降ってくる (1層に1個)
	// // ・十字爆弾 / 2マス乗り越え は半々でランダム。出る場所は、その時点で床があるマスからランダム
	// // ・やり直し (ResetForRetry): 置いてあるアイテムも持っている爆弾も消える。もう一度出るようになる
	// // ・下の層へ (ResetForNewLayer): 置いてあるアイテムは消える。持っている爆弾は持ち越す
	// // 2マス乗り越えはプレイヤーが持つ (Player::SetJumpBoost)
	// class ItemManager
	// {
	// public:
		// ItemManager(); // 乱数を毎回ちがう値で始める
		// ~ItemManager() = default;

		// ItemManager(const ItemManager&) = delete;
		// ItemManager& operator=(const ItemManager&) = delete;

		// void Initialize(); // モデルを作る

		// // やり直し: 置いてあるアイテム・持っている爆弾を消して、この層でもう一度出るようにする
		// void ResetForRetry();
		// // 下の層へ: 置いてあるアイテムを消して、新しい層で出るようにする (持っている爆弾はそのまま)
		// void ResetForNewLayer();
		// // 置いてあるアイテムだけ消す (ボスが落ち始めたときなど)
		// void ClearFieldItem();

		// // 出す・動かす・取る (毎フレーム)
		// // canSpawn: 戦闘中か / hearts: プレイヤーの HP / プレイヤーとボスのいるマスには出さない
		// void Update(const Floor& floor, bool canSpawn, int32_t hearts, Player& player, const RyoEngine::Vector3& bossPosition, float deltaTime);

		// // 十字爆弾を使う: center から十字に、ステージの端まで崩す (真ん中から外へ少しずつ遅れて)
		// // 爆弾を持っていなければ何もしないで false
		// bool TryUseBomb(Floor& floor, const Cell& center);

		// bool HasBomb() const { return hasBomb_; }

		// void UpdateVisuals();
		// void Draw();

	// private:
		// // プレイヤーが触れたら取る (十字爆弾はここで持つ。2マス乗り越えはプレイヤーにすぐ効果を付ける)
		// // 爆弾を持っているときは、爆弾は取れない (そのまま置いておく)
		// void PickUp(Player& player);

		// // 出せるマスからランダムに1つ選んで降らせる。出せたら true
		// bool TrySpawn(const Floor& floor, const Cell& playerCell, const Cell& bossCell);

		// FieldItem item_;                 // ステージに置いてあるアイテム (1個)
		// bool isSpawnedThisLayer_ = false; // この層でもう出したか
		// bool hasBomb_ = false;           // 十字爆弾を持っているか (1個まで)
		// std::mt19937 random_;
	// };
// }
