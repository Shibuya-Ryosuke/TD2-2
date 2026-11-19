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

	// 床を崩す形 (テトリスのような形) をまとめたもの (Game から分けたもの)
	// ・5つの形 (X L Totu Line Block) を LB / RB (Q / E) で切り替えて、選んでいる形 (NOW) を置く
	//   X → L → Totu → Line → Block → X … とぐるっと回る。置いても形はそのまま (同じ形を続けて置ける)
	// ・1回置いたら kShapeCooldown 秒は置けない (その間はパネルの NOW が暗くなる。切り替えはできる)
	// ・床に NOW の形の予告を出す
	// ・画面の右はしに 上: LB で来る形 / 真ん中: NOW / 下: RB で来る形 を出す (NextShapePanel)
	class ShapeController
	{
	public:
		ShapeController() = default;
		~ShapeController() = default;

		ShapeController(const ShapeController&) = delete;
		ShapeController& operator=(const ShapeController&) = delete;

		void Initialize(); // 形を作る
		void Reset();      // 最初の形 (X) を選んで、待ち時間を消す

		// 待ち時間を進める (毎フレーム)
		void Update(float deltaTime);

		// 形を切り替える (isPrevious: LB / Q で前へ、isNext: RB / E で次へ)
		void Select(bool isPrevious, bool isNext);

		// NOW の形で、プレイヤーの前の床を崩す。崩せたら true
		// 待ち時間中・崩せる床が1つもないときは何もしない
		bool TryBreak(Floor& floor, const Cell& playerCell, const Cell& facing);

		// 床に NOW の形の予告を出す (isVisible が false なら消す)
		void UpdatePreview(Floor& floor, const Cell& playerCell, const Cell& facing, bool isVisible);

		// 画面の右はしのパネルを、カメラとプレイヤーの向きに合わせる
		void UpdatePanel(const StageCamera& camera, const Cell& facing);
		void DrawPanel();
		void DrawPanelLabels() const;

		// NOW の形を置けるか (待ち時間が終わっているか)
		bool IsReady() const { return cooldownTimer_ <= 0.0f; }

	private:
		// 選んでいる形から offset だけずらした形 (ぐるっと回る。-1: LB で来る形 / 0: NOW / 1: RB で来る形)
		const BreakShape* GetShape(int32_t offset) const;

		std::vector<BreakShape> shapes_; // 形の種類 (X L Totu Line Block。BreakShapeType と同じ順)
		int32_t selectedIndex_ = 0;      // 選んでいる形 (NOW) の番号
		std::vector<Cell> cells_;        // NOW の形で崩すマス (使い回し)
		float cooldownTimer_ = 0.0f;     // 次の形を置けるまで
		NextShapePanel panel_;           // 右はしの形の表示
	};
}
