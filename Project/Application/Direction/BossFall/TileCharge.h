#pragma once
#include <RyoEngine.h>
#include <vector>
#include "../../Stage/Grid/Cell.h"

namespace App
{
	class Floor;

	//ボスが落ちたあと上の層に残った足場が、ドクンドクンと光りながら浮いて力をためる
	class TileCharge
	{
	public:
		//upperの残った足場をためる準備をして、heldのマスはプレイヤーが乗っているので浮かせない
		void Begin(Floor& upper, const Cell& held);
		//プレイヤーが乗っている足場をsinkだけ沈める
		void SetHeldSink(float sink) { heldSink_ = sink; }
		//プレイヤーが飛び立ったので、乗っていた足場もほかの足場と一緒に浮かせる
		void Release();
		//ドクンと光と浮きを進めて足場の位置と光を入れる
		void Update(float deltaTime, Floor& upper);

	private:
		std::vector<Cell> cells_; //ためている足場のマス
		Cell held_{}; //プレイヤーが乗っている足場
		bool isHolding_ = false; //プレイヤーがまだ乗っているか
		float heldSink_ = 0.0f; //プレイヤーが乗っている足場を沈める深さ
		float releaseTimer_ = 0.0f; //プレイヤーが飛び立ってからの秒数
		float timer_ = 0.0f; //ため始めてからの秒数
	};
}
