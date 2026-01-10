#include "Core/GameWorld.hpp"
#include "Core/IGameEvents.hpp"
#include "Features/Behaviors/Utils.hpp"
#include "Features/Components.hpp"
#include "Features/Hunter.hpp"
#include "Features/Swordsman.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// --- Minimal Test Framework ---

#define TEST_ASSERT(cond) \
	do { \
		if (!(cond)) { \
			throw std::runtime_error("Assertion failed: " #cond " at " + std::string(__FILE__) + ":" + std::to_string(__LINE__)); \
		} \
	} while(0)

template <typename T>
void assertEqual(const T& actual, const T& expected, const char* file, int line)
{
	if (actual != expected)
	{
		throw std::runtime_error("Assertion failed at " + std::string(file) + ":" + std::to_string(line)
			+ "\n  Expected: " + std::to_string(expected)
			+ "\n  Actual:   " + std::to_string(actual));
	}
}

// Specialization for strings if needed, or generic to_string reliance
// For simple types in this project (int, size_t), std::to_string works.

#define TEST_ASSERT_EQ(actual, expected) \
	assertEqual(actual, expected, __FILE__, __LINE__)

namespace
{
	// --- Enhanced Mock ---
	
	struct AttackEventData
	{
		sw::core::UnitId attacker;
		sw::core::UnitId target;
		uint32_t damage;
		uint32_t targetHp;
	};

	struct MoveEventData
	{
		sw::core::UnitId unit;
		sw::core::Position from;
		sw::core::Position to;
	};

	class TestEvents final : public sw::core::IGameEvents
	{
	public:
		std::vector<AttackEventData> attacks;
		std::vector<MoveEventData> moves;
		std::vector<sw::core::UnitId> deaths;
		std::vector<std::string> sequence; // For verifying global order

		void onMapCreated(uint32_t, uint32_t) override {}
		void onUnitSpawned(sw::core::UnitId, std::string_view, sw::core::Position) override {}
		void onMarchStarted(sw::core::UnitId, sw::core::Position, sw::core::Position) override {}
		void onMarchEnded(sw::core::UnitId, sw::core::Position) override {}

		void onUnitAttacked(sw::core::UnitId attacker, sw::core::UnitId target, uint32_t damage, uint32_t targetHp) override
		{
			attacks.push_back({attacker, target, damage, targetHp});
			sequence.push_back("Attack " + std::to_string(attacker) + "->" + std::to_string(target));
		}

		void onUnitMoved(sw::core::UnitId unit, sw::core::Position from, sw::core::Position to) override
		{
			moves.push_back({unit, from, to});
			sequence.push_back("Move " + std::to_string(unit));
		}

		void onUnitDied(sw::core::UnitId unit) override
		{
			deaths.push_back(unit);
			sequence.push_back("Die " + std::to_string(unit));
		}

		void reset()
		{
			attacks.clear();
			moves.clear();
			deaths.clear();
			sequence.clear();
		}
	};

	// --- Tests ---

	// 1. Порядок ходов: Units must act in creation order
	void testTurnExecutionOrder()
	{
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(5, 5);
		TestEvents events;

		auto u1 = std::make_unique<Swordsman>(1, 10, 1);
		u1->addComponent<MarchComponent>(Position{0, 2});
		
		auto u2 = std::make_unique<Swordsman>(2, 10, 1);
		u2->addComponent<MarchComponent>(Position{2, 0});

		world.addUnit(std::move(u1), Position{0, 0});
		world.addUnit(std::move(u2), Position{2, 2});

		// Simulate the loop from main.cpp
		world.forEachUnit([&](Unit& unit) {
			unit.playTurn(world, events);
		});

		// Verify order in sequence log
		TEST_ASSERT_EQ(events.sequence.size(), (size_t)2);
		TEST_ASSERT(events.sequence[0] == "Move 1");
		TEST_ASSERT(events.sequence[1] == "Move 2");
	}

	// 2. Диагональный шаг: Check that unit approaches target, don't enforce specific implementation
	void testDiagonalMarchProgress()
	{
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(5, 5);
		TestEvents events;

		Position start{0, 0};
		Position target{2, 2};

		auto u1 = std::make_unique<Swordsman>(1, 10, 1);
		u1->addComponent<MarchComponent>(target);
		world.addUnit(std::move(u1), start);

		world.getUnitById(1)->playTurn(world, events);

		TEST_ASSERT_EQ(events.moves.size(), (size_t)1);
		
		auto newPosOpt = world.getUnitPosition(1);
		TEST_ASSERT(newPosOpt.has_value());
		Position newPos = *newPosOpt;
		
		// Metric: Chebyshev distance (max of dx, dy) should decrease
		int startDist = std::max(std::abs((int)target.x - (int)start.x), std::abs((int)target.y - (int)start.y));
		int newDist = std::max(std::abs((int)target.x - (int)newPos.x), std::abs((int)target.y - (int)newPos.y));
		
		TEST_ASSERT(newDist < startDist);
		TEST_ASSERT(newPos != start);
	}

	// 3. Блокировка пути: Ensure unit does NOT move into occupied cell
	void testMarchSafetyConstraint()
	{
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(5, 1);
		TestEvents events;

		// Mover at 0,0 wants to go to 2,0
		auto mover = std::make_unique<Swordsman>(1, 10, 1);
		mover->addComponent<MarchComponent>(Position{2, 0});
		mover->removeComponent<StrengthComponent>(); // Disable attack

		// Blocker at 1,0 (Direct path)
		auto blocker = std::make_unique<Swordsman>(2, 10, 1);

		world.addUnit(std::move(mover), Position{0, 0});
		world.addUnit(std::move(blocker), Position{1, 0});

		bool acted = world.getUnitById(1)->playTurn(world, events);

		// Assert: The unit might wait (acted=false) or flank (acted=true, moved to 0,1 or 1,1 if map 2d)
		// But CRITICAL: It must NOT be at 1,0
		auto currentPosOpt = world.getUnitPosition(1);
		TEST_ASSERT(currentPosOpt.has_value());
		Position currentPos = *currentPosOpt;
		TEST_ASSERT(currentPos != (Position{1, 0}));
		
		// In current implementation, it waits.
		if (!acted) {
			TEST_ASSERT(currentPos == (Position{0, 0}));
		}
	}

	// 4. Компоненты: Hunter without Agility cannot shoot
	void testComponentDependency()
	{
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(5, 1);
		TestEvents events;

		// Hunter at 0,0. Target at 2,0. Range 3.
		auto hunter = std::make_unique<Hunter>(1, 10, 5, 1, 3);
		hunter->removeComponent<AgilityComponent>(); // BREAK IT
		
		auto target = std::make_unique<Swordsman>(2, 10, 1);

		world.addUnit(std::move(hunter), Position{0, 0});
		world.addUnit(std::move(target), Position{2, 0});

		bool acted = world.getUnitById(1)->playTurn(world, events);

		TEST_ASSERT(!acted);
		TEST_ASSERT(events.attacks.empty());
	}

	void testSwordsmanAttack()
	{
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(3, 1);
		TestEvents events;

		auto s1 = std::make_unique<Swordsman>(1, 10, 5);
		auto s2 = std::make_unique<Swordsman>(2, 10, 1);

		world.addUnit(std::move(s1), Position{0, 0});
		world.addUnit(std::move(s2), Position{1, 0});

		world.getUnitById(1)->playTurn(world, events);

		TEST_ASSERT_EQ(events.attacks.size(), (size_t)1);
		TEST_ASSERT_EQ(events.attacks[0].attacker, (UnitId)1);
		TEST_ASSERT_EQ(events.attacks[0].target, (UnitId)2);
		TEST_ASSERT_EQ(events.attacks[0].damage, (uint32_t)5);
		TEST_ASSERT_EQ(events.attacks[0].targetHp, (uint32_t)5);
	}

	void testUnitDeathCycle()
	{
		using namespace sw::core;
		using namespace sw::features;

		GameWorld world(3, 1);
		TestEvents events;

		auto s1 = std::make_unique<Swordsman>(1, 10, 10);
		auto s2 = std::make_unique<Swordsman>(2, 10, 1); // 10 HP

		world.addUnit(std::move(s1), Position{0, 0});
		world.addUnit(std::move(s2), Position{1, 0});

		// Attack kills unit 2
		world.getUnitById(1)->playTurn(world, events);
		
		auto* u2 = world.getUnitById(2);
		TEST_ASSERT(u2->isDead());
		TEST_ASSERT(events.deaths.empty()); // Not removed from world yet

		// Cleanup phase
		auto deadIds = world.removeDeadUnits();
		for(auto id : deadIds) events.onUnitDied(id);

		TEST_ASSERT_EQ(events.deaths.size(), (size_t)1);
		TEST_ASSERT_EQ(events.deaths[0], (UnitId)2);
		TEST_ASSERT(world.getUnitById(2) == nullptr);
	}

	// --- Restored Tests ---

	void testImplicitTargetingHealthComponent()
	{
		using namespace sw::core;
		using namespace sw::features;
		GameWorld world(3, 1);
		auto attacker = std::make_unique<Swordsman>(1, 10, 1);
		auto target = std::make_unique<Swordsman>(2, 10, 1);
		
		target->removeComponent<HealthComponent>(); // Should become untargetable
		
		world.addUnit(std::move(attacker), Position{0, 0});
		world.addUnit(std::move(target), Position{1, 0});
		
		auto targets = utils::getTargetsInRange(*world.getUnitById(1), world, 1, 1);
		TEST_ASSERT(targets.empty());
	}

	void testAddBehaviorRejectsNull()
	{
		using namespace sw::features;
		using namespace sw::core;

		Swordsman unit(1, 10, 1);
		bool threw = false;
		try {
			unit.addBehavior(nullptr);
		} catch (const std::invalid_argument&) {
			threw = true;
		}
		TEST_ASSERT(threw);
	}
}

int main()
{
	try {
		testTurnExecutionOrder();
		testDiagonalMarchProgress();
		testMarchSafetyConstraint();
		testComponentDependency();
		testSwordsmanAttack();
		testUnitDeathCycle();
		testImplicitTargetingHealthComponent();
		testAddBehaviorRejectsNull();
		
		std::cout << "All extended unit tests passed!" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Test failed: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
