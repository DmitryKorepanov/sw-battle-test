#pragma once

#include "../Core/IComponent.hpp"
#include "../Core/Types.hpp"

#include <cstdint>

namespace sw::features
{
	class HealthComponent : public core::IComponent
	{
	public:
		explicit HealthComponent(uint32_t hp) :
				_currentHp(static_cast<int32_t>(hp))
		{}

		bool takeDamage(uint32_t damage)
		{
			_currentHp -= static_cast<int32_t>(damage);
			return _currentHp <= 0;
		}

		[[nodiscard]] uint32_t getHp() const
		{
			return _currentHp < 0 ? 0 : static_cast<uint32_t>(_currentHp);
		}

		[[nodiscard]] bool isDead() const
		{
			return _currentHp <= 0;
		}

	private:
		int32_t _currentHp;
	};

	struct StrengthComponent : public core::IComponent
	{
		uint32_t value;

		explicit StrengthComponent(uint32_t v) :
				value(v)
		{}
	};

	struct AgilityComponent : public core::IComponent
	{
		uint32_t value;

		explicit AgilityComponent(uint32_t v) :
				value(v)
		{}
	};

	struct RangeComponent : public core::IComponent
	{
		uint32_t value;

		explicit RangeComponent(uint32_t v) :
				value(v)
		{}
	};

	struct MarchComponent : public core::IComponent
	{
		core::Position target;

		explicit MarchComponent(core::Position t) :
				target(t)
		{}
	};

	struct BlockerComponent : public core::IComponent
	{
		// Marker component: indicates this unit blocks movement.
		BlockerComponent() = default;
	};
}
