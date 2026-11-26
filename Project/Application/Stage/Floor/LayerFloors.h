#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <memory>
#include "Floor.h"

namespace App
{
	//今いる層と下の層と上の層の3つの床をまとめて、チュートリアルのあとlayer1から5のcsvを順に使う
	class LayerFloors
	{
	public:
		LayerFloors() = default;
		~LayerFloors() = default;

		LayerFloors(const LayerFloors&) = delete;
		LayerFloors& operator=(const LayerFloors&) = delete;

		//layer層目の今いる層だけ作って始める
		void Start(int32_t layer);
		//1層目の上のチュートリアルの床から始める
		void StartTutorial() { Start(-1); }
		//チュートリアルの床にいるか
		bool IsTutorial() const { return layer_ < 0; }
		//下の層がまだなければ作る
		void EnsureBelow();
		//下の層へ1つ進めて、その下を新しく作る
		void AdvanceLayer();
		//足場を飛ばし終わったら上の層を消す
		void ReleaseUpper();

		Floor& GetCurrent() { return *current_; }
		const Floor& GetCurrent() const { return *current_; }
		Floor* GetBelow() { return below_.get(); } //まだないときはnullptr
		Floor* GetUpper() { return upper_.get(); } //ないときはnullptr
		int32_t GetLayer() const { return layer_; } //0から数えた今いる層で、チュートリアルは-1

		void UpdateVisuals(); //3つの床の見た目を確定する
		void Draw(); //下の層、今の層、上の層の順に描く

		//1層目のマスで決めた位置をfloorのマスの数に合わせる
		static RyoEngine::Vector3 GetSpawnPosition(const Floor& floor, int32_t firstLayerX, int32_t firstLayerZ);

	private:
		//CSVを読んでindex層目の床を作る
		std::unique_ptr<Floor> CreateFloor(int32_t index, bool isForeground) const;

		int32_t layer_ = 0;
		std::unique_ptr<Floor> current_{}; //今いる層
		std::unique_ptr<Floor> below_{}; //暗く見えている下の層
		std::unique_ptr<Floor> upper_{}; //足場がボスへ飛んでいく間だけある上の層
	};
}
