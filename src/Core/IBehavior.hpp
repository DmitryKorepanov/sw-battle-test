#pragma once

namespace sw::core
{
	class Unit;
	class IGameWorld;
	class IGameEvents;

	class IBehavior
	{
	public:
		virtual ~IBehavior() = default;

		// Returns true if the behavior can be executed in the current world state
		virtual bool canExecute(const Unit& unit, const IGameWorld& world) const = 0;

		// Executes the behavior
		virtual void execute(Unit& unit, IGameWorld& world, IGameEvents& events) = 0;
	};
}

