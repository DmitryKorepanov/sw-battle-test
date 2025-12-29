#pragma once

#include "IBehavior.hpp"
#include "IComponent.hpp"
#include "TypeRegistry.hpp"
#include "Types.hpp"

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace sw::core
{
	class GameWorld;  // Forward decl

	class Unit
	{
	private:
		// Allow GameWorld to access setPosition to maintain grid consistency.
		// Behaviors should use world.moveUnit() instead of modifying unit directly.
		friend class GameWorld;

		UnitId _id;
		Position _position;
		bool _isDead{false};

		TypeRegistry _components;
		std::vector<std::unique_ptr<IBehavior>> _behaviors;

		void setPosition(Position pos) noexcept
		{
			_position = pos;
		}

	public:
		Unit(UnitId id, Position pos) :
				_id(id),
				_position(pos)
		{}

		virtual ~Unit() = default;

		[[nodiscard]]
		UnitId getId() const noexcept
		{
			return _id;
		}

		[[nodiscard]]
		Position getPosition() const noexcept
		{
			return _position;
		}

		// === Components ===

		template <typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<IComponent, T>, "Component must inherit from IComponent");
			auto ptr = _components.emplace<T>(std::forward<Args>(args)...);
			return *ptr;
		}

		template <typename T>
		T* getComponent()
		{
			return _components.getPtr<T>();
		}

		template <typename T>
		const T* getComponent() const
		{
			return _components.getPtr<T>();
		}

		template <typename T>
		void removeComponent()
		{
			_components.remove<T>();
		}

		// === Behaviors ===

		void addBehavior(std::unique_ptr<IBehavior> behavior)
		{
			if (!behavior)
			{
				throw std::invalid_argument("Unit::addBehavior: behavior must not be null");
			}
			_behaviors.push_back(std::move(behavior));
		}

		bool playTurn(IGameWorld& world, IGameEvents& events)
		{
			for (auto& behavior : _behaviors)
			{
				if (behavior->canExecute(*this, world))
				{
					behavior->execute(*this, world, events);
					return true;
				}
			}
			return false;
		}

		// === State ===

		[[nodiscard]]
		bool isDead() const noexcept
		{
			return _isDead;
		}

		void setDead(bool dead) noexcept
		{
			_isDead = dead;
		}
	};
}
