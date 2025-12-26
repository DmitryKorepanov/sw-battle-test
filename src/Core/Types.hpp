#pragma once

#include <cstdint>

namespace sw::core
{
	using UnitId = uint32_t;
	using Coordinate = uint32_t;

	struct Position
	{
		Coordinate x;
		Coordinate y;

		bool operator==(const Position& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=(const Position& other) const
		{
			return !(*this == other);
		}
	};
}
