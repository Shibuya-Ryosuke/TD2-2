#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include <vector>
#include "../Stage/Grid/Cell.h"

namespace App
{
	class BreakShape;
	class StageCamera;

	//画面の右上で4つの形を輪に並べて手前がNOWになる形を選ぶ輪
	class NextShapePanel
	{
	public:
		NextShapePanel() = default;
		~NextShapePanel() = default;

		NextShapePanel(const NextShapePanel&) = delete;
		NextShapePanel& operator=(const NextShapePanel&) = delete;

		//selectedIndexの形が手前に来た最初の状態にする
		void Initialize(int32_t selectedIndex);

		//毎フレーム輪を回してブロックを並べ直す
		void Update(const StageCamera& camera, const std::vector<BreakShape>& shapes, int32_t selectedIndex, const Cell& facing, bool isNowReady);

		void Draw(); //ブロックを描く

	private:
		//カメラから作る画面の位置を3Dに直すための値
		struct ScreenBasis
		{
			RyoEngine::Vector3 origin{}; //カメラの位置
			RyoEngine::Vector3 right{}; //画面の右
			RyoEngine::Vector3 up{}; //画面の上
			RyoEngine::Vector3 forward{}; //画面の奥
			float halfWidth = 0.0f; //画面の横の半分に映る長さ
			float halfHeight = 0.0f; //画面の縦の半分に映る長さ
		};

		//画面の位置と奥行きから3Dの位置を求める
		static RyoEngine::Vector3 ToWorld(const ScreenBasis& basis, float screenX, float screenY, float depth);
		//番号の差を輪の近い向きの差に収める
		static float WrapOffset(float offset, float count);

		//輪を選んでいる形の方へ回す
		void UpdateRotation(int32_t selectedIndex, size_t count);
		//輪の上の角度と明るさで1つの形のブロックを置く
		void UpdateItem(size_t index, const ScreenBasis& basis, const BreakShape& shape, const Cell& facing, float angle, float brightness);

		std::vector<std::vector<std::unique_ptr<RyoEngine::Model>>> blocks_; //形ごとのブロック
		std::vector<size_t> activeCounts_; //形ごとの今使っているブロックの数
		std::vector<Cell> cells_; //使い回す形のマス
		float displayIndex_ = 0.0f; //回っている途中は小数の手前に見えている番号
	};
}
