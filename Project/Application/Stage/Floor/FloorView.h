#pragma once
#include <RyoEngine.h>
#include <memory>
#include <vector>
#include "../Grid/Cell.h"
#include "../../Direction/Floor/FloorCrumble.h"
#include "../../Direction/Floor/FloorJiggle.h"
#include "../../Direction/Floor/FloorWear.h"

namespace App
{
	class Floor;

	//タイルの状態から位置と色と光り方を決めて描く床の見た目
	class FloorView
	{
	public:
		FloorView() = default;
		~FloorView() = default;

		FloorView(const FloorView&) = delete;
		FloorView& operator=(const FloorView&) = delete;

		//床があるマスにモデルを作る
		void CreateModels(const Floor& floor);

		//毎フレームタイルの位置と大きさと色と光り方を確定する
		void Update(const Floor& floor);
		//着地の強さで着地したマスからぷるるんと揺らす
		void Jiggle(const Floor& floor, const Cell& center, float power);
		//地面パンチで、歩数だけ遅らせてマスを跳ねさせる
		void Bounce(const std::vector<int32_t>& steps, float height, float stepDelay, float speed, float damping);

		//見えているタイルを描く
		void Draw(const Floor& floor);

	private:
		RyoEngine::Vector4 GetBaseColor(const Floor& floor, const Cell& cell) const; //いつもの色
		RyoEngine::Vector4 GetTileColor(const Floor& floor, const Cell& cell) const; //下の層と予告も入れた今の色
		bool IsPreviewGlowing(const Floor& floor, const Cell& cell) const; //予告で光らせるマスか

		std::vector<std::unique_ptr<RyoEngine::Model>> models_; //タイルと同じ並びで穴のマスはnullptrのモデル
		FloorCrumble crumble_; //ハートと砕けると欠片の崩れる演出
		FloorJiggle jiggle_; //着地のぷるるん
		FloorWear wear_; //穴のとなりの細いヒビ
		float previewGlowTime_ = 0.0f; //予告の明滅の時間
	};
}
