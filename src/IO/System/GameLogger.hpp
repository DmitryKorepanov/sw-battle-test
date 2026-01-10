#pragma once

#include "../../Core/IGameEvents.hpp"
#include "EventLog.hpp"

#include <cstdint>
#include <string_view>

namespace sw::io
{
	// Adapter for core::IGameEvents -> EventLog
	class GameLogger : public sw::core::IGameEvents
	{
	private:
		sw::EventLog& _log;
		uint64_t& _tick;

	public:
		GameLogger(sw::EventLog& log, uint64_t& tickRef);

		void onMapCreated(uint32_t width, uint32_t height) override;
		void onUnitSpawned(sw::core::UnitId unit, std::string_view unitType, sw::core::Position pos) override;
		void onMarchStarted(sw::core::UnitId unit, sw::core::Position from, sw::core::Position target) override;
		void onUnitAttacked(sw::core::UnitId attacker, sw::core::UnitId target, uint32_t damage, uint32_t targetHp) override;
		void onUnitMoved(sw::core::UnitId unit, sw::core::Position from, sw::core::Position to) override;
		void onUnitDied(sw::core::UnitId unit) override;
		void onMarchEnded(sw::core::UnitId unit, sw::core::Position pos) override;
	};
}
