//[アイテム] チームで相談中なので一旦コメントアウト
//使うときは行頭のコメントを外す
// #pragma once
// #include <RyoEngine.h>
// #include <memory>
// #include "ItemType.h"
// #include "../../Stage/Grid/Cell.h"

// namespace App
// {
	// class Floor;

	// //上から降ってきて浮かんで回り、足場が崩れたら一緒に落ちるアイテム
	// class FieldItem
	// {
	// public:
		// FieldItem() = default;
		// ~FieldItem() = default;

		// FieldItem(const FieldItem&) = delete;
		// FieldItem& operator=(const FieldItem&) = delete;

		// void Initialize(); //モデルを作る

		// //floorのcellに上から降らせる
		// void Spawn(ItemType type, const Floor& floor, const Cell& cell);
		// //取ったときややり直しで消す
		// void Remove();

		// //毎フレーム降らせて回して足場と一緒に落とす
		// void Update(const Floor& floor, float deltaTime);
		// //行列を確定する
		// void UpdateVisuals();
		// void Draw();

		// //落ちて消えている途中も入れてステージに出ているか
		// bool IsActive() const { return state_ != State::None; }
		// //足場と一緒に落ちている途中以外で取れる状態か
		// bool CanPickUp() const;
		// ItemType GetType() const { return type_; }

		// //プレイヤーの足元と大きさから体の箱と重なっているか
		// bool IsTouching(const RyoEngine::Vector3& playerFeet, float playerSize) const;

	// private:
		// enum class State
		// {
			// None,     //出ていない
			// Dropping, //上から降ってきている
			// Resting,  //床の上に浮かんでいる
			// Falling,  //足場が崩れて一緒に落ちている
		// };

		// float GetRestY(const Floor& floor) const; //床の上に浮かんでいるときの箱の真ん中の高さ

		// State state_ = State::None;
		// ItemType type_ = ItemType::CrossBomb;
		// Cell cell_{};                    //置いてあるマス
		// RyoEngine::Vector3 position_{};  //箱の真ん中の位置
		// float startY_ = 0.0f;            //降ってき始めた高さ
		// float timer_ = 0.0f;             //降ってくる時間か落ちる時間
		// float yawDeg_ = 0.0f;            //度の回る角度

		// std::unique_ptr<RyoEngine::Model> model_{}; //仮のcubeの見た目
	// };
// }
