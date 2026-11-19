#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include "Cell.h"

namespace App
{
	// 床のマス目の計算だけをするクラス (Floor の親クラス)
	// ・マス ⇔ 位置 の変換、床の範囲、跳ぶときのずれ・道すじ
	// ・タイルの状態 (穴・崩れる予定など) は持たない。それは子の Floor が持つ
	// 床全体の真ん中が原点。マスの数が変わっても、床全体の大きさ (kFloorSize) は同じ
	class FloorGrid
	{
	public:
		int32_t GetWidth() const { return width_; }
		int32_t GetDepth() const { return depth_; }
		float GetHeight() const { return height_; }  // タイルの上の面の高さ
		float GetTileSizeX() const { return tileSizeX_; }
		float GetTileSizeZ() const { return tileSizeZ_; }

		// その位置が何マス目か (床の外なら、範囲外の番号が返る)
		Cell GetCellAt(const RyoEngine::Vector3& position) const;
		// マスの真ん中の位置 (高さはタイルの上の面)
		RyoEngine::Vector3 GetCellCenter(const Cell& cell) const;
		// タイルのいつもの位置 (タイルの厚さの真ん中)
		RyoEngine::Vector3 GetRestPosition(const Cell& cell) const;
		// 床の外に出ないように位置を収める (radius: 体の半径)
		RyoEngine::Vector3 ClampToBoundary(const RyoEngine::Vector3& position, float radius) const;
		// 床の範囲の中か
		bool Contains(const Cell& cell) const;

		size_t ToIndex(const Cell& cell) const; // マス → 配列の番号 (手前の行から順に、左から右へ)
		Cell ToCell(size_t index) const;        // 配列の番号 → マス

		// マスの向き → 1マス分のずれ (ワールドの長さ)
		RyoEngine::Vector3 StepOffset(const Cell& direction) const;
		// start から end へ、マスの線にそって進む (先に x、次に z。progress: 0 〜 1。高さは直線で変わる)
		static RyoEngine::Vector3 AlongGrid(const RyoEngine::Vector3& start, const RyoEngine::Vector3& end, float progress);

	protected:
		// マスの数と高さを決める (1マスの大きさは床全体の大きさから計算する)
		void SetupGrid(int32_t width, int32_t depth, float height);

		int32_t width_ = 0;
		int32_t depth_ = 0;
		float tileSizeX_ = 2.0f;
		float tileSizeZ_ = 2.0f;
		float height_ = 0.0f;
	};
}
