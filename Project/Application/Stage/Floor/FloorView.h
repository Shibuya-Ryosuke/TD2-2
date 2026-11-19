#pragma once
#include <RyoEngine.h>
#include <memory>
#include <vector>
#include "../Grid/Cell.h"

namespace App
{
	class Floor;

	// 床の見た目 (Floor から分けたもの)
	// タイルごとのモデルを持って、Floor のタイルの状態から 位置・色・光り方 を決めて描く
	class FloorView
	{
	public:
		FloorView() = default;
		~FloorView() = default;

		FloorView(const FloorView&) = delete;
		FloorView& operator=(const FloorView&) = delete;

		// 床があるマスにモデルを作る (CSV で穴のマスは作らない)
		void CreateModels(const Floor& floor);

		// タイルの 位置・大きさ・色・光り方 を確定する (毎フレーム)
		void Update(const Floor& floor);

		// 見えているタイルを描く
		void Draw(const Floor& floor);

	private:
		RyoEngine::Vector4 GetBaseColor(const Floor& floor, const Cell& cell) const; // いつもの色 (切り離し・穴のとなり・ふつう)
		RyoEngine::Vector4 GetTileColor(const Floor& floor, const Cell& cell) const; // 今の色 (下の層・崩れる前・予告 も入れる)
		bool IsPreviewGlowing(const Floor& floor, const Cell& cell) const;           // 予告で光らせるマスか

		std::vector<std::unique_ptr<RyoEngine::Model>> models_; // タイルと同じ並び (CSV で穴のマスは nullptr)
		float previewGlowTime_ = 0.0f;                          // 予告の明滅の時間
	};
}
