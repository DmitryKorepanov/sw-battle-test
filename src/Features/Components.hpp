#pragma once

#include "../Core/IComponent.hpp"
#include "../Core/Types.hpp"

#include <cstdint>

namespace sw::features
{
	struct HealthComponent : public core::IComponent
	{
		int32_t currentHp;

		explicit HealthComponent(uint32_t hp) :
				currentHp(static_cast<int32_t>(hp))
		{}
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
