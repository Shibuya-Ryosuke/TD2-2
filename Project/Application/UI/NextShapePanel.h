#pragma once
#include <RyoEngine.h>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include "../Stage/Cell.h"

namespace App {
	class BreakShape;
	class StageCamera;

	/// <summary>
	/// 画面の右はしに出す「今の形・次の形・その次の形」(テトリスの NEXT のような表示)。
	/// パネル : ステージ = 1 : 9。
	/// ブロックは cube.obj を小さくして並べる (新しい画像やモデルは使わない。背景もなし)。
	/// 形はステージの床と同じ向きに寝かせて、プレイヤーの向きに合わせて回す。
	/// カメラは1つしか映せないので、ステージのカメラの手前に置いて画面の右はしに見せている。
	/// </summary>
	class NextShapePanel {
	public:
		static constexpr int32_t kSlotCount = 3; // 今・次・その次

		NextShapePanel() = default;
		~NextShapePanel() = default;

		NextShapePanel(const NextShapePanel&) = delete;
		NextShapePanel& operator=(const NextShapePanel&) = delete;

		void Initialize(); // 最初の状態にする (ブロックは必要になったときに作る)

		/// <summary>カメラに合わせて、ブロックを並べ直す (毎フレーム)</summary>
		/// <param name="camera">ステージのカメラ</param>
		/// <param name="shapes">今・次・その次 の形 (nullptr なら出さない)</param>
		/// <param name="facing">プレイヤーの向き</param>
		void Update(const StageCamera& camera, const std::array<const BreakShape*, kSlotCount>& shapes, const Cell& facing);

		void Draw(); // ブロックを描く
		void DrawLabels() const; // NOW / NEXT の文字 (デバッグ表示。英数字だけ)

	private:
		/// <summary>画面の位置を3Dの位置に直すための値 (カメラから作る)</summary>
		struct ScreenBasis {
			RyoEngine::Vector3 origin{};  // カメラの位置
			RyoEngine::Vector3 right{};   // 画面の右
			RyoEngine::Vector3 up{};      // 画面の上
			RyoEngine::Vector3 forward{}; // 画面の奥
			float halfWidth = 0.0f;       // 画面の横の半分に映る長さ
			float halfHeight = 0.0f;      // 画面の縦の半分に映る長さ
		};

		/// <summary>画面の位置 (-1 〜 1) と奥行きから、3Dの位置を求める</summary>
		static RyoEngine::Vector3 ToWorld(const ScreenBasis& basis, float screenX, float screenY, float depth);

		float GetCenterX() const; // パネルの真ん中の横の位置 (-1 〜 1)
		void UpdateSlot(int32_t slot, const ScreenBasis& basis, const BreakShape* shape, const Cell& facing); // 1つの形のブロックを置く

		std::array<std::vector<std::unique_ptr<RyoEngine::Model>>, kSlotCount> blocks_{}; // 形ごとのブロック
		std::array<size_t, kSlotCount> activeCounts_{}; // 形ごとの、今使っているブロックの数
		std::vector<Cell> cells_; // 形のマス (使い回し)
	};
}
