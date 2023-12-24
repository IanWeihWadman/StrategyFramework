#pragma once

#include "ExampleTypes.h"
#include "ExampleRuleState.h"
#include "EmptyActionMode.h"
#include "MoveUnitActionMode.h"
#include <StrategyCore/VariantActionMode.h>

namespace Example
{
	using ExampleActionModes = std::variant<EmptyActionMode, MoveUnitActionMode>;

	struct ModeWrapper
	{
		ExampleActionModes Value;
	};
}