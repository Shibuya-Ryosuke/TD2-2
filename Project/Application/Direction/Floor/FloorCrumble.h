#pragma once
#include <RyoEngine.h>
#include <cstdint>
#include <vector>
#include "FloorShards.h"
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//床がハートからガラスのように砕ける演出
	class FloorCrumble
	{
	public:
		//1マス分の崩れる演出の値
		struct State
		{
			float heartScale = 0.0f; //0で出ていないハートの大きさ
			float heartCrack = 0.0f; //1回目のガン！で入るハートのヒビ
			float heartBreak = 0.0f; //2回目のガン！でハートが割れて消えた割合
			float tileCrack = 0.0f; //タイルのひびの広がり
			float flash = 0.0f; //ガン！の光
			float shatter = 0.0f; //砕けた割合
			float scale = 1.0f; //ぷくっとぽんっの大きさの倍率
			RyoEngine::Vector3 shake{}; //ガタガタとビクッの位置のずれ
		};

		FloorCrumble() = default;
		~FloorCrumble() = default;

		FloorCrumble(const FloorCrumble&) = delete;
		FloorCrumble& operator=(const FloorCrumble&) = delete;

		//タイルの数に合わせて用意する
		void Initialize(size_t tileCount);

		//今の崩れる演出の値
		State Compute(const Floor& floor, const Cell& cell) const;
		//シェーダーに値を渡して砕け始めなら欠片を飛ばす
		void Apply(size_t index, RyoEngine::Model& model, const State& state, const Floor& floor, const Cell& cell);

		void Update(); //飛び散る欠片を動かす
		void Draw(); //飛び散る欠片を描く

	private:
		//崩れるまでの進み具合からハンマーの演出の値を作る
		static void ApplyHammer(State& effect, float progress, float duration);

		std::vector<uint8_t> wasShattering_; //前のフレームで砕けていたか
		FloorShards shards_; //砕けたときに飛び散る欠片
	};
}
