#pragma once
#include <cstdint>
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class FloorGrid;

	//着地したマスが沈んでぷるぷる揺れ、まわりへ波のように広がる
	class FloorJiggle
	{
	public:
		//FloorViewが位置と大きさに掛ける1マス分の揺れ
		struct Offset
		{
			float offsetY = 0.0f; //上下のずれ
			float scaleY = 1.0f; //縦につぶれる倍率
			float scaleXZ = 1.0f; //横にふくらむ倍率
		};

		FloorJiggle() = default;
		~FloorJiggle() = default;

		FloorJiggle(const FloorJiggle&) = delete;
		FloorJiggle& operator=(const FloorJiggle&) = delete;

		void Initialize(size_t tileCount); //タイルの数に合わせて用意する

		//centerのマスに着地した強さで揺らし始める
		void Trigger(const FloorGrid& grid, const Cell& center, float power);

		void Update(); //揺れを進める
		Offset Get(size_t index) const; //そのタイルの今の揺れ

	private:
		//1マス分の揺れの状態
		struct Wobble
		{
			float time = 10.0f; //揺れ始めてからの時間でマイナスはまだ波が届いていない
			float amount = 0.0f; //揺れの大きさ
		};

		//0から沈んで戻ってだんだん小さくなる揺れの値
		static float GetWave(const Wobble& wobble);

		std::vector<Wobble> wobbles_;
	};
}
