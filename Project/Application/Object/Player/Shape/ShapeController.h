#pragma once
#include <cstdint>
#include <vector>
#include "BreakShape.h"
#include "../../../Stage/Grid/Cell.h"
#include "../../../UI/NextShapePanel.h"

namespace App
{
	class Floor;
	class StageCamera;

	//4つの形を切り替えて置き、予告と右上の輪を出す
	class ShapeController
	{
	public:
		ShapeController() = default;
		~ShapeController() = default;

		ShapeController(const ShapeController&) = delete;
		ShapeController& operator=(const ShapeController&) = delete;

		void Initialize(); //形を作る
		void Reset(); //最初の形を選ぶ

		//Gameが毎フレーム決める今形を置けるか
		void SetReady(bool isReady) { isReady_ = isReady; }

		//LBで前へ、RBで次へ形を切り替える
		void Select(bool isPrevious, bool isNext);

		//NOWの形でプレイヤーの前の床を崩せたらtrue
		bool TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing);

		//床にNOWの形の予告を出す
		void UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible);

		//右上の輪をカメラとプレイヤーの向きに合わせる
		void UpdatePanel(const StageCamera& camera, const Cell& facing);
		void DrawPanel();

		//NOWの形を置けるか
		bool IsReady() const { return isReady_; }

	private:
		//選んでいる形からoffsetだけずらした形
		const BreakShape* GetShape(int32_t offset) const;

		std::vector<BreakShape> shapes_; //BreakShapeTypeと同じ順の形
		int32_t selectedIndex_ = 0; //選んでいるNOWの番号
		std::vector<Cell> cells_; //NOWの形で崩すマス
		bool isReady_ = true; //今形を置けるか
		NextShapePanel panel_; //右上の形を選ぶ輪
	};
}
