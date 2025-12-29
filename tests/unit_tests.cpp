#include "Core/GameWorld.hpp"
#include "Core/IGameEvents.hpp"
#include "Features/Behaviors/Utils.hpp"
#include "Features/Components.hpp"
#include "Features/Swordsman.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace
{
	class NullEvents final : public sw::core::IGameEvents
	{
	private:
		uint32_t _attacks{0};
		uint32_t _moves{0};
		uint32_t _deaths{0};
		uint32_t _marchEnded{0};

	public:
		void onMapCreated(uint32_t, uint32_t) override {}

		void onUnitSpawned(sw::core::UnitId, std::string_view, sw::core::Position) override {}

		void onMarchStarted(sw::core::UnitId, sw::core::Position, sw::core::Position) override {}

		void onUnitAttacked(sw::core::UnitId, sw::core::UnitId, uint32_t, uint32_t) override
		{
			++_attacks;
		}

		void onUnitMoved(sw::core::UnitId, sw::core::Position, sw::core::Position) override
		{
			++_moves;
		}

		void onUnitDied(sw::core::UnitId) override
		{
			++_deaths;
		}

		void onMarchEnded(sw::core::UnitId, sw::core::Position) override
		{
			++_marchEnded;
		}

		[[nodiscard]]
		uint32_t attacks() const noexcept
		{
			return _attacks;
		}

		[[nodiscard]]
		uint32_t moves() const noexcept
		{
			return _moves;
		}

		[[nodiscard]]
		uint32_t deaths() const noexcept
		{
			return _deaths;
		}

		[[nodiscard]]
		uint32_t marchEnded() const noexcept
		{
			return _marchEnded;
		}
	};

	void testImplicitTargetingHealthComponent()
	{
		using namespace sw;
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(3, 1);

		auto attacker = std::make_unique<Swordsman>(1, Position{0, 0}, /*hp*/ 10, /*strength*/ 1);
		auto target = std::make_unique<Swordsman>(2, Position{1, 0}, /*hp*/ 10, /*strength*/ 1);

		// Remove health to simulate "not targetable by current implicit rule".
		target->removeComponent<HealthComponent>();

		world.addUnit(std::move(attacker));
		world.addUnit(std::move(target));

		auto* attackerPtr = world.getUnitById(1);
		assert(attackerPtr != nullptr);

		auto targets = utils::getTargetsInRange(*attackerPtr, world, 1, 1);
		assert(targets.empty());
	}

	void testMoveCanGetStuckWhenNextCellOccupied()
	{
		using namespace sw;
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(2, 1);
		NullEvents events;
		const Position origin{0, 0};

		auto mover = std::make_unique<Swordsman>(1, origin, /*hp*/ 10, /*strength*/ 1);
		auto blocker = std::make_unique<Swordsman>(2, Position{1, 0}, /*hp*/ 10, /*strength*/ 1);

		// Remove attack capability so the only possible action would be MoveBehavior via MarchComponent.
		mover->removeComponent<StrengthComponent>();
		blocker->removeComponent<StrengthComponent>();

		// Set march target into an occupied cell.
		mover->addComponent<MarchComponent>(Position{1, 0});

		world.addUnit(std::move(mover));
		world.addUnit(std::move(blocker));

		auto* moverPtr = world.getUnitById(1);
		assert(moverPtr != nullptr);

		// Should not be able to move (blocked), so no action this turn.
		const bool acted = moverPtr->playTurn(world, events);
		assert(!acted);
		assert(moverPtr->getPosition() == origin);
		assert(events.moves() == 0);
	}

	void testAddBehaviorRejectsNull()
	{
		using namespace sw;
		using namespace sw::core;
		using namespace sw::features;

		Swordsman unit(1, Position{0, 0}, /*hp*/ 10, /*strength*/ 1);

		bool threw = false;
		try
		{
			unit.addBehavior(nullptr);
		}
		catch (const std::invalid_argument&)
		{
			threw = true;
		}
		assert(threw);
	}
}

int main()
{
	testImplicitTargetingHealthComponent();
	testMoveCanGetStuckWhenNextCellOccupied();
	testAddBehaviorRejectsNull();
	return 0;
}
