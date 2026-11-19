#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "../Stage/Floor.h"

namespace App
{
	// 今いる層・下の層・降りてきた上の層 の3つの床をまとめたもの (Game から分けたもの)
	// layer1.csv 〜 layer5.csv を順に使う (6層目からは1層目に戻る)
	class LayerFloors
	{
	public:
		LayerFloors() = default;
		~LayerFloors() = default;

		LayerFloors(const LayerFloors&) = delete;
		LayerFloors& operator=(const LayerFloors&) = delete;

		// layer 層目から始める (今いる層だけ作る)
		void Start(int32_t layer);
		// 下の層がまだなければ作る
		void EnsureBelow();
		// 下の層へ進む (今の層 → 上の層、下の層 → 今の層、その下を新しく作る)
		void AdvanceLayer();
		// 上の層を消す (足場を飛ばし終わったら)
		void ReleaseUpper();

		Floor& GetCurrent() { return *current_; }
		const Floor& GetCurrent() const { return *current_; }
		Floor* GetBelow() { return below_.get(); }  // まだないときは nullptr
		Floor* GetUpper() { return upper_.get(); }  // ないときは nullptr
		int32_t GetLayer() const { return layer_; } // 今いる層 (0 から)

		void UpdateVisuals(); // 3つの床の見た目を確定する
		void Draw();          // 下の層 → 今の層 → 上の層 の順に描く

		// 1層目のマスで決めた位置 (x, z) を、floor のマスの数に合わせた位置に直す
		static RyoEngine::Vector3 GetSpawnPosition(const Floor& floor, int32_t firstLayerX, int32_t firstLayerZ);

	private:
		// index 層目の床を作る (CSV を読む)。isForeground: 今いる層か
		std::unique_ptr<Floor> CreateFloor(int32_t index, bool isForeground) const;

		int32_t layer_ = 0;
		std::unique_ptr<Floor> current_{}; // 今いる層
		std::unique_ptr<Floor> below_{};   // 下の層 (暗く見えている)
		std::unique_ptr<Floor> upper_{};   // 降りてきた上の層 (足場がボスへ飛んでいく間だけ)
	};
}
