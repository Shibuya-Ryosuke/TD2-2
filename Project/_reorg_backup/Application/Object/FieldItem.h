// [アイテム] アイテムは一旦使わない (チームで相談中)。コメントアウトしている
// (使うときは、このファイルの行頭の "// " を外す)
// #pragma once
// #include <RyoEngine.h>
// #include <memory>
// #include "../Game/ItemType.h"
// #include "../Stage/Cell.h"

// namespace App
// {
	// class Floor;

	// // ステージに置いてあるアイテム (1個)
	// // ・上から降ってきて、床の少し上に浮かんで回る
	// // ・置いてある足場が崩れたら、一緒に落ちて消える
	// // ・見た目は cube を小さくして色を変えたもの (種類で色が変わる)
	// class FieldItem
	// {
	// public:
		// FieldItem() = default;
		// ~FieldItem() = default;

		// FieldItem(const FieldItem&) = delete;
		// FieldItem& operator=(const FieldItem&) = delete;

		// void Initialize(); // モデルを作る

		// // floor の cell に、上から降らせる
		// void Spawn(ItemType type, const Floor& floor, const Cell& cell);
		// // 消す (取った・やり直し・次の層)
		// void Remove();

		// // 降ってくる・回る・足場と一緒に落ちる (毎フレーム)
		// void Update(const Floor& floor, float deltaTime);
		// // 行列を確定する
		// void UpdateVisuals();
		// void Draw();

		// // ステージに出ているか (落ちて消えている途中も true)
		// bool IsActive() const { return state_ != State::None; }
		// // 取れる状態か (足場と一緒に落ちている途中は取れない)
		// bool CanPickUp() const;
		// ItemType GetType() const { return type_; }

		// // プレイヤーの体 (箱) と重なっているか (AABB)
		// // playerFeet: プレイヤーの足元の位置 / playerSize: プレイヤーの箱の1辺
		// bool IsTouching(const RyoEngine::Vector3& playerFeet, float playerSize) const;

	// private:
		// enum class State
		// {
			// None,     // 出ていない
			// Dropping, // 上から降ってきている
			// Resting,  // 床の上に浮かんでいる
			// Falling,  // 足場が崩れて、一緒に落ちている
		// };

		// float GetRestY(const Floor& floor) const; // 床の上に浮かんでいるときの、箱の真ん中の高さ

		// State state_ = State::None;
		// ItemType type_ = ItemType::CrossBomb;
		// Cell cell_{};                    // 置いてあるマス
		// RyoEngine::Vector3 position_{};  // 箱の真ん中の位置
		// float startY_ = 0.0f;            // 降ってき始めた高さ
		// float timer_ = 0.0f;             // 降ってくる・落ちる時間
		// float yawDeg_ = 0.0f;            // 回る角度 (度)

		// std::unique_ptr<RyoEngine::Model> model_{}; // 見た目 (仮: cube)
	// };
// }
