//[アイテム] チームで相談中なので一旦コメントアウト
//使うときは行頭のコメントを外す
// #pragma once
// #include <RyoEngine.h>
// #include <cstdint>
// #include <random>
// #include "ItemType.h"
// #include "FieldItem.h"
// #include "../../Stage/Grid/Cell.h"

// namespace App
// {
	// class Floor;
	// class Player;

	// //HPが少ないと1層に1個アイテムを降らせて、持っている十字爆弾をまとめる
	// class ItemManager
	// {
	// public:
		// ItemManager(); //乱数を毎回ちがう値で始める
		// ~ItemManager() = default;

		// ItemManager(const ItemManager&) = delete;
		// ItemManager& operator=(const ItemManager&) = delete;

		// void Initialize(); //モデルを作る

		// //やり直しで置いてあるアイテムと爆弾を消してもう一度出るようにする
		// void ResetForRetry();
		// //下の層で置いてあるアイテムを消して爆弾は持ち越す
		// void ResetForNewLayer();
		// //ボスが落ち始めたときなどに置いてあるアイテムだけ消す
		// void ClearFieldItem();

		// //毎フレーム出して動かして取る
		// void Update(const Floor& floor, bool canSpawn, int32_t hearts, Player& player, const RyoEngine::Vector3& bossPosition, float deltaTime);

		// //持っている十字爆弾でcenterから十字に端まで崩して、なければfalse
		// bool TryUseBomb(Floor& floor, const Cell& center);

		// bool HasBomb() const { return hasBomb_; }

		// void UpdateVisuals();
		// void Draw();

	// private:
		// //プレイヤーが触れたら取るが、爆弾を持っていたら爆弾は取れない
		// void PickUp(Player& player);

		// //出せるマスからランダムに1つ選んで降らせたらtrue
		// bool TrySpawn(const Floor& floor, const Cell& playerCell, const Cell& bossCell);

		// FieldItem item_;                 //ステージに置いてある1個のアイテム
		// bool isSpawnedThisLayer_ = false; //この層でもう出したか
		// bool hasBomb_ = false;           //1個まで持てる十字爆弾を持っているか
		// std::mt19937 random_;
	// };
// }
