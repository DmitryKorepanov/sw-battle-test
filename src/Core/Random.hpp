#pragma once

#include <random>
#include <stdexcept>
#include <vector>

namespace sw::core
{
	class Random
	{
	public:
		template <typename T>
		static const T& getItem(const std::vector<T>& items)
		{
			if (items.empty())
			{
				throw std::runtime_error("Cannot get random item from empty collection");
			}

			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<size_t> dis(0, items.size() - 1);

			return items[dis(gen)];
		}

		// Prevent returning a reference to an element of a temporary vector.
		template <typename T>
		static const T& getItem(std::vector<T>&&) = delete;
	};
}
