#pragma once
#include <cstddef>
#include <vector>

namespace App
{
	//穴のとなりになった床に細いヒビをじわっと広げる濃さを数える
	class FloorWear
	{
	public:
		//タイルの数に合わせて用意する
		void Initialize(size_t tileCount);

		//穴のとなりなら濃くして、そうでなければすぐ消した0から1のヒビの濃さ
		float Advance(size_t index, bool isWorn, float deltaTime);

	private:
		std::vector<float> rates_; //タイルごとのヒビの濃さ
	};
}
