#pragma once
#include <cstdint>

namespace App
{

	//xが右へ、zが奥へ何マス目かを表すマスの番号か向き
	struct Cell
	{
		int32_t x = 0;
		int32_t z = 0;
	};

	inline bool operator==(const Cell& a, const Cell& b)
	{
		return a.x == b.x && a.z == b.z;
	}
	inline bool operator!=(const Cell& a, const Cell& b)
	{
		return !(a == b);
	}
	inline Cell operator+(const Cell& a, const Cell& b)
	{
		return Cell{ a.x + b.x, a.z + b.z };
	}
	inline Cell operator-(const Cell& a, const Cell& b)
	{
		return Cell{ a.x - b.x, a.z - b.z };
	}
	inline Cell operator-(const Cell& a)
	{
		return Cell{ -a.x, -a.z };
	}
	inline Cell operator*(const Cell& a, int32_t scale)
	{
		return Cell{ a.x * scale, a.z * scale };
	}
}
