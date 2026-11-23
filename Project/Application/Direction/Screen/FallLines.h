#pragma once
#include <cstdint>

namespace App
{
	//層を降りている間パステルの線が下から上へ流れるポストエフェクト
	class FallLines
	{
	public:
		FallLines() = default;
		~FallLines() = default;

		FallLines(const FallLines&) = delete;
		FallLines& operator=(const FallLines&) = delete;

		void Initialize(); //止めた状態でポストエフェクトを用意する
		//降りているかで線を強くしたり弱くしたりする
		void Update(bool isFalling);

	private:
		uint32_t effect_ = 0; //ポストエフェクトの番号
		float strength_ = 0.0f; //今の強さ
	};
}
