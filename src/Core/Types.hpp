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

		constexpr bool operator==(const Position& other) const noexcept
		{
			return x == other.x && y == other.y;
		}

		constexpr bool operator!=(const Position& other) const noexcept
		{
			return !(*this == other);
		}
	};
}
