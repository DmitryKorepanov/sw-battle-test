#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <utility>
#include <stdexcept>
#include <type_traits>

namespace sw::core
{
	/// @brief Type-indexed registry with shared ownership and interface registration
	class TypeRegistry
	{
	private:
		std::unordered_map<std::type_index, std::shared_ptr<void>> _instances;
		std::unordered_map<std::type_index, std::vector<std::type_index>> _aliases;

	public:
		TypeRegistry() = default;

		TypeRegistry(const TypeRegistry&) = delete;
		TypeRegistry& operator=(const TypeRegistry&) = delete;

		TypeRegistry(TypeRegistry&&) = default;
		TypeRegistry& operator=(TypeRegistry&&) = default;

		template <typename T, typename... Interfaces, typename... Args>
		std::shared_ptr<T> emplace(Args&&... args)
		{
			static_assert((std::is_base_of_v<Interfaces, T> && ...), "T must inherit from all specified interfaces");

			auto instance = std::make_shared<T>(std::forward<Args>(args)...);
			auto typeT = std::type_index(typeid(T));
			
			_instances[typeT] = instance;
			
			if constexpr (sizeof...(Interfaces) > 0)
			{
				((_instances[std::type_index(typeid(Interfaces))] = instance), ...);
				_aliases[typeT] = { std::type_index(typeid(Interfaces))... };
			}
			
			return instance;
		}

		template <typename T, typename... Interfaces>
		void add(std::shared_ptr<T> instance)
		{
			static_assert((std::is_base_of_v<Interfaces, T> && ...), "T must inherit from all specified interfaces");

			auto typeT = std::type_index(typeid(T));
			_instances[typeT] = instance;
			
			if constexpr (sizeof...(Interfaces) > 0)
			{
				((_instances[std::type_index(typeid(Interfaces))] = instance), ...);
				_aliases[typeT] = { std::type_index(typeid(Interfaces))... };
			}
		}

		template <typename T>
		std::shared_ptr<T> get() const
		{
			auto it = _instances.find(std::type_index(typeid(T)));
			if (it != _instances.end())
			{
				return std::static_pointer_cast<T>(it->second);
			}
			return nullptr;
		}

		template <typename T>
		void remove()
		{
			auto typeT = std::type_index(typeid(T));
			_instances.erase(typeT);
			
			// Remove aliases
			auto it = _aliases.find(typeT);
			if (it != _aliases.end())
			{
				for (const auto& alias : it->second)
				{
					_instances.erase(alias);
				}
				_aliases.erase(it);
			}
		}
	};
}
