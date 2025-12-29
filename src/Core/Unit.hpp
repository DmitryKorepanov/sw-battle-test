#pragma once

#include "Types.hpp"
#include "IComponent.hpp"
#include "IBehavior.hpp"
#include "IGameWorld.hpp"
#include "IGameEvents.hpp"
#include "TypeRegistry.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <type_traits>

namespace sw::core
{
	class GameWorld; // Forward decl

	class Unit
	{
	public:
		Unit(UnitId id, Position pos)
			: _id(id)
			, _position(pos)
		{}

		virtual ~Unit() = default;

		UnitId getId() const { return _id; }
		Position getPosition() const { return _position; }
		
		// Allow GameWorld to access setPosition to maintain grid consistency.
		// Behaviors should use world.moveUnit() instead of modifying unit directly.
		friend class GameWorld;

	private:
		void setPosition(Position pos) { _position = pos; }

	public:
		// === Components ===

		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<IComponent, T>, "Component must inherit from IComponent");
			auto ptr = _components.emplace<T>(std::forward<Args>(args)...);
			return *ptr;
		}

		template<typename T>
		std::shared_ptr<T> getComponent()
		{
			return _components.get<T>();
		}

		template<typename T>
		std::shared_ptr<const T> getComponent() const
		{
			return _components.get<T>();
		}

		template<typename T>
		void removeComponent()
		{
			_components.remove<T>();
		}

		// === Behaviors ===

		void addBehavior(std::unique_ptr<IBehavior> behavior)
		{
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
		
		bool isDead() const { return _isDead; }
		void setDead(bool dead) { _isDead = dead; }

	private:
		UnitId _id;
		Position _position;
		bool _isDead{ false };

		TypeRegistry _components;
		std::vector<std::unique_ptr<IBehavior>> _behaviors;
	};
}
