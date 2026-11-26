#pragma once
#include <RyoEngine.h>
#include <array>
#include <memory>
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//チュートリアルで壊すマスを上からスポットライトで照らして、ふちをキラキラ光らせる
	class TutorialSpot
	{
	public:
		TutorialSpot() = default;
		~TutorialSpot() = default;

		TutorialSpot(const TutorialSpot&) = delete;
		TutorialSpot& operator=(const TutorialSpot&) = delete;

		void Initialize(); //光の柱と枠のモデルを作る
		//照らすマスを決めてふわっと光らせる
		void Show(const std::vector<Cell>& cells);
		//ふわっと消す
		void Hide();
		//まだ床があるマスだけ光をマスに合わせる
		void Update(const Floor& floor);
		void Draw();

	private:
		static constexpr size_t kMaxSpots = 4; //一度に照らせるマスの数

		//1マス分の光
		struct Spot
		{
			//unique_ptrを持つのでコピーを禁止する
			Spot() = default;
			~Spot() = default;
			Spot(const Spot&) = delete;
			Spot& operator=(const Spot&) = delete;

			std::unique_ptr<RyoEngine::Model> beam; //光の柱
			std::unique_ptr<RyoEngine::Model> frame; //マスの上の枠
			Cell cell{};
			bool isUsed = false; //照らすマスか
			bool isVisible = false; //このフレームで描くか
		};

		std::array<Spot, kMaxSpots> spots_;
		float strength_ = 0.0f; //今の光の強さ
		bool isShown_ = false; //光らせているか
	};
}
