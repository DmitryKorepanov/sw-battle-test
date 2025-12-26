#pragma once

#include <vector>
#include <random>
#include <stdexcept>

namespace sw::core
{
	class Random
	{
	public:
		template<typename T>
		static T getItem(const std::vector<T>& items)
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
	};
}

