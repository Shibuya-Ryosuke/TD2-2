#pragma once
#include <cstdint>

namespace App
{
	//ダメージで画面のふちが赤くぎらっと光って脈打つポストエフェクト
	class DamageFlash
	{
	public:
		DamageFlash() = default;
		~DamageFlash() = default;

		DamageFlash(const DamageFlash&) = delete;
		DamageFlash& operator=(const DamageFlash&) = delete;

		void Initialize(); //止めた状態でポストエフェクトを用意する
		void Play(); //パッと光らせる
		void Update(); //すっと消していく

	private:
		uint32_t effect_ = 0; //ポストエフェクトの番号
		float timer_ = -1.0f; //光ってからの時間でマイナスは光っていない
	};
}
