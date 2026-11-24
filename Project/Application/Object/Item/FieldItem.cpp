//[アイテム] チームで相談中なので一旦コメントアウト
//使うときは行頭のコメントを外す
// #include "FieldItem.h"
// #include "../../Config/GameConfig.h"
// #include "../../Stage/Floor/Floor.h"
// #include <algorithm>
// #include <cmath>

// using namespace RyoEngine;

// namespace App
// {
	// namespace
	// {
		// constexpr float kItemFallSpeed = 12.0f; //タイルと同じ足場と一緒に落ちる速さ
	// }

	// //モデルを作る
	// void FieldItem::Initialize()
	// {
		// model_ = Model::Create(GameConfig::kCubeModel);
		// model_->SetCastShadow(false);
		// state_ = State::None;
	// }

	// //floorのcellに上から降らせる
	// void FieldItem::Spawn(ItemType type, const Floor& floor, const Cell& cell)
	// {
		// type_ = type;
		// cell_ = cell;
		// timer_ = 0.0f;
		// yawDeg_ = 0.0f;

		// //マスの真上の高い所から降らせる
		// position_ = floor.GetCellCenter(cell);
		// startY_ = GetRestY(floor) + GameConfig::kItemDropHeight;
		// position_.y = startY_;
		// state_ = State::Dropping;

		// //種類で色を変える
		// if (type_ == ItemType::CrossBomb)
		// {
			// model_->SetColor(GameConfig::kCrossBombColor);
		// }
		// else
		// {
			// model_->SetColor(GameConfig::kJumpBoostColor);
		// }
	// }

	// //消す
	// void FieldItem::Remove()
	// {
		// state_ = State::None;
	// }

	// //取れる状態か
	// bool FieldItem::CanPickUp() const
	// {
		// if (state_ == State::Dropping || state_ == State::Resting)
		// {
			// return true;
		// }
		// return false;
	// }

	// //床の上に浮かんでいるときの箱の真ん中の高さ
	// float FieldItem::GetRestY(const Floor& floor) const
	// {
		// return floor.GetHeight() + GameConfig::kItemHover + GameConfig::kItemSize * 0.5f;
	// }

	// //降らせて回して足場と一緒に落とす
	// void FieldItem::Update(const Floor& floor, float deltaTime)
	// {
		// if (state_ == State::None)
		// {
			// return;
		// }

		// //いつも回る
		// yawDeg_ += GameConfig::kItemSpinSpeed * deltaTime;
		// if (yawDeg_ >= 360.0f)
		// {
			// yawDeg_ -= 360.0f;
		// }

		// //置いてある足場が崩れたら一緒に落ちる
		// if (CanPickUp() && floor.IsHole(cell_))
		// {
			// state_ = State::Falling;
			// timer_ = 0.0f;
			// startY_ = position_.y;
		// }

		// //だんだん速く上から降ってくる
		// if (state_ == State::Dropping)
		// {
			// timer_ += deltaTime;
			// const float t = std::clamp(timer_ / GameConfig::kItemDropDuration, 0.0f, 1.0f);
			// position_.y = startY_ + (GetRestY(floor) - startY_) * t * t;
			// if (t >= 1.0f)
			// {
				// state_ = State::Resting;
			// }
			// return;
		// }

		// //足場と一緒に落ちて少したったら消える
		// if (state_ == State::Falling)
		// {
			// timer_ += deltaTime;
			// position_.y = startY_ - kItemFallSpeed * timer_ * timer_;
			// if (timer_ >= GameConfig::kItemFallTime)
			// {
				// state_ = State::None;
			// }
		// }
	// }

	// //プレイヤーの体の箱と重なっているか
	// bool FieldItem::IsTouching(const Vector3& playerFeet, float playerSize) const
	// {
		// //真ん中から半分ずつのアイテムの箱
		// const float itemHalf = GameConfig::kItemSize * 0.5f;
		// const Vector3 itemMin = { position_.x - itemHalf, position_.y - itemHalf, position_.z - itemHalf };
		// const Vector3 itemMax = { position_.x + itemHalf, position_.y + itemHalf, position_.z + itemHalf };

		// //足元から上へplayerSizeのプレイヤーの箱
		// const float playerHalf = playerSize * 0.5f;
		// const Vector3 playerMin = { playerFeet.x - playerHalf, playerFeet.y, playerFeet.z - playerHalf };
		// const Vector3 playerMax = { playerFeet.x + playerHalf, playerFeet.y + playerSize, playerFeet.z + playerHalf };

		// //どれか1つの軸でも離れていたら重なっていない
		// if (itemMax.x < playerMin.x || itemMin.x > playerMax.x)
		// {
			// return false;
		// }
		// if (itemMax.y < playerMin.y || itemMin.y > playerMax.y)
		// {
			// return false;
		// }
		// if (itemMax.z < playerMin.z || itemMin.z > playerMax.z)
		// {
			// return false;
		// }
		// return true;
	// }

	// //小さい箱を傾けて回す行列を確定する
	// void FieldItem::UpdateVisuals()
	// {
		// const float scale = GameConfig::kItemSize / GameConfig::kCubeModelSize;
		// model_->SetScale({ scale, scale, scale });
		// model_->SetRotate({ GameConfig::kItemTilt, yawDeg_, GameConfig::kItemTilt });
		// model_->SetTranslate(position_);
		// model_->TransferMatrix();
	// }

	// //出ているときだけ描く
	// void FieldItem::Draw()
	// {
		// if (state_ == State::None)
		// {
			// return;
		// }
		// model_->Draw();
	// }
// }
