#pragma once
#include <cstdint>

namespace App
{

	// マスの番号、またはマスの向き (となりのマスへのずれ)。
	// x: 左から何マス目 (向きなら +1 が右) / z: 手前から何マス目 (向きなら +1 が奥)
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
