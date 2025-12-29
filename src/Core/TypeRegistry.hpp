#pragma once

#include <concepts>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sw::core
{
	/// @brief Type-indexed registry with shared ownership and interface registration
	class TypeRegistry
	{
	private:
		struct Entry
		{
			std::shared_ptr<void> instance;
			std::type_index owner;
		};

		std::unordered_map<std::type_index, Entry> _instances;
		std::unordered_map<std::type_index, std::vector<std::type_index>> _aliases;

		template <typename T>
		[[nodiscard]]
		static std::type_index typeIndex() noexcept
		{
			// Normalize type to avoid accidental duplication for const/ref-qualified queries like get<const T>().
			using TNormalized = std::remove_cvref_t<T>;
			return std::type_index(typeid(TNormalized));
		}

		void removeAliases(const std::type_index& owner)
		{
			if (auto it = _aliases.find(owner); it != _aliases.end())
			{
				for (const auto& alias : it->second)
				{
					// Sanity check: ensure we are removing an alias that still belongs to this owner
					if (auto entry = _instances.find(alias); entry != _instances.end() && entry->second.owner == owner)
					{
						_instances.erase(entry);
					}
				}
				_aliases.erase(it);
			}
		}

		template <typename Interface>
		void checkAliasCollision(const std::type_index& owner) const
		{
			auto aliasType = typeIndex<Interface>();
			if (auto it = _instances.find(aliasType); it != _instances.end() && it->second.owner != owner)
			{
				throw std::runtime_error(
					std::string("TypeRegistry alias collision: interface '") + typeid(Interface).name()
					+ "' is already registered by '" + it->second.owner.name() + "'");
			}
		}

		template <typename T, typename Interface>
		void registerInterface(const std::shared_ptr<T>& instance, const std::type_index& owner)
		{
			auto aliasType = typeIndex<Interface>();
			// CRITICAL: Store shared_ptr<Interface> erased to void.
			// This ensures the void* points to the Interface subobject, not the Concrete object start.
			std::shared_ptr<Interface> interfacePtr = instance;
			_instances.insert_or_assign(aliasType, Entry{interfacePtr, owner});
		}

	public:
		TypeRegistry() = default;

		TypeRegistry(const TypeRegistry&) = delete;
		TypeRegistry& operator=(const TypeRegistry&) = delete;

		TypeRegistry(TypeRegistry&&) = default;
		TypeRegistry& operator=(TypeRegistry&&) = default;

		template <typename T, typename... Interfaces, typename... Args>
			requires(std::derived_from<T, Interfaces> && ...)
		[[nodiscard]]
		std::shared_ptr<T> emplace(Args&&... args)
		{
			auto instance = std::make_shared<T>(std::forward<Args>(args)...);
			add<T, Interfaces...>(instance);
			return instance;
		}

		template <typename T, typename... Interfaces>
			requires(std::derived_from<T, Interfaces> && ...)
		void add(std::shared_ptr<T> instance)
		{
			if (!instance)
			{
				throw std::invalid_argument("TypeRegistry::add: instance must not be null");
			}

			const auto ownerType = typeIndex<T>();

			// 1. Check for collisions early to fail fast on configuration errors.
			(checkAliasCollision<Interfaces>(ownerType), ...);

			// 2. Clear old aliases if we are updating an existing object
			removeAliases(ownerType);

			// 3. Register concrete type
			_instances.insert_or_assign(ownerType, Entry{instance, ownerType});

			// 4. Register interfaces
			if constexpr (sizeof...(Interfaces) > 0)
			{
				// Track aliases owned by this concrete type so remove() can clean them up.
				_aliases[ownerType] = {typeIndex<Interfaces>()...};

				(registerInterface<T, Interfaces>(instance, ownerType), ...);
			}
		}

		template <typename T>
		[[nodiscard]]
		std::shared_ptr<T> get()
		{
			if (auto it = _instances.find(typeIndex<T>()); it != _instances.end())
			{
				return std::static_pointer_cast<T>(it->second.instance);
			}
			return nullptr;
		}

		template <typename T>
		[[nodiscard]]
		std::shared_ptr<const T> get() const
		{
			if (auto it = _instances.find(typeIndex<T>()); it != _instances.end())
			{
				return std::static_pointer_cast<const T>(it->second.instance);
			}
			return nullptr;
		}

		template <typename T>
		[[nodiscard]]
		bool contains() const noexcept
		{
			return _instances.contains(typeIndex<T>());
		}

		template <typename T>
		void remove()
		{
			const auto type = typeIndex<T>();
			auto it = _instances.find(type);
			if (it == _instances.end())
			{
				return;
			}

			const auto owner = it->second.owner;

			// Optimization: if T is the concrete type, we can erase 'it' directly.
			if (type == owner)
			{
				_instances.erase(it);
			}
			else
			{
				_instances.erase(owner);
			}

			removeAliases(owner);
		}
	};
}
