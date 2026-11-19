#pragma once
#include <vector>
#include "BreakShape.h"
#include "BreakShapeQueue.h"
#include "../Stage/Cell.h"
#include "../UI/NextShapePanel.h"

namespace App
{
	class Floor;
	class StageCamera;

	// 床を崩す形 (テトリスのような形) をまとめたもの (Game から分けたもの)
	// ・形の種類と順番 (BreakShapeQueue)
	// ・1回使ったら kShapeCooldown 秒は使えない (その間はパネルの NOW が暗くなる)
	// ・床に今の形の予告を出す
	// ・画面の右はしに 今・次・その次 の形を出す (NextShapePanel)
	class ShapeController
	{
	public:
		ShapeController() = default;
		~ShapeController() = default;

		ShapeController(const ShapeController&) = delete;
		ShapeController& operator=(const ShapeController&) = delete;

		void Initialize(); // 形を作る
		void Reset();      // 形の順番を最初からにする

		// 待ち時間を進める (毎フレーム)
		void Update(float deltaTime);

		// 今の形で、プレイヤーの前の床を崩す。崩せたら true (次の形へ進む)
		// 待ち時間中・崩せる床が1つもないときは何もしない
		bool TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing);

		// 床に今の形の予告を出す (isVisible が false なら消す)
		void UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible);

		// 画面の右はしのパネルを、カメラとプレイヤーの向きに合わせる
		void UpdatePanel(const StageCamera& camera, const Cell& facing);
		void DrawPanel();
		void DrawPanelLabels() const;

		// 今の形を使えるか (待ち時間が終わっているか)
		bool IsReady() const { return cooldownTimer_ <= 0.0f; }

	private:
		const BreakShape* FindShape(BreakShapeType type) const; // 種類から形を探す (なければ nullptr)
		const BreakShape* GetCurrentShape();                    // 今の形 (なければ nullptr)

		std::vector<BreakShape> shapes_; // 形の種類 (X L Totu Line Block)
		BreakShapeQueue queue_;          // 次に来る形の順番
		std::vector<Cell> cells_;        // 今の形で崩すマス (使い回し)
		float cooldownTimer_ = 0.0f;     // 次の形を使えるまで
		NextShapePanel panel_;           // 今・次・その次 の形の表示
	};
}
