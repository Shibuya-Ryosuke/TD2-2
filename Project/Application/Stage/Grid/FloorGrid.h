#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "Cell.h"

namespace App
{
	//床の真ん中が原点のマス目の計算だけをするFloorの親クラス
	class FloorGrid
	{
	public:
		int32_t GetWidth() const { return width_; }
		int32_t GetDepth() const { return depth_; }
		float GetHeight() const { return height_; } //タイルの上の面の高さ
		float GetTileSizeX() const { return tileSizeX_; }
		float GetTileSizeZ() const { return tileSizeZ_; }

		//その位置が何マス目か
		Cell GetCellAt(const RyoEngine::Vector3& position) const;
		//タイルの上の面の高さのマスの真ん中の位置
		RyoEngine::Vector3 GetCellCenter(const Cell& cell) const;
		//タイルの厚さの真ん中のいつもの位置
		RyoEngine::Vector3 GetRestPosition(const Cell& cell) const;
		//体の半径を入れて床の外に出ないように位置を収める
		RyoEngine::Vector3 ClampToBoundary(const RyoEngine::Vector3& position, float radius) const;
		//床の範囲の中か
		bool Contains(const Cell& cell) const;

		size_t ToIndex(const Cell& cell) const; //マスから手前の行から順の配列の番号へ
		Cell ToCell(size_t index) const; //配列の番号からマスへ

		//マスの向きから1マス分のワールドのずれへ
		RyoEngine::Vector3 StepOffset(const Cell& direction) const;
		//startからendへprogressの分だけマスの線にそって進む
		static RyoEngine::Vector3 AlongGrid(const RyoEngine::Vector3& start, const RyoEngine::Vector3& end, float progress);

	protected:
		//床全体の大きさからマスの数と高さを決める
		void SetupGrid(int32_t width, int32_t depth, float height);

		int32_t width_ = 0;
		int32_t depth_ = 0;
		float tileSizeX_ = 2.0f;
		float tileSizeZ_ = 2.0f;
		float height_ = 0.0f;
	};
}
