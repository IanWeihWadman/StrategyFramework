#pragma once

#include "ActionModeRegister.h"
#include <StrategyCore/TurnPhase.h>

namespace Example
{
	using ExampleTurnPhase = Strategy::TurnPhase<ExampleTypes, Strategy::VariantActionMode<ExampleTypes, ExampleRuleState, ExampleActionModes>, ExampleRuleState>;
	using ExampleTurnPhaseResult = Strategy::TurnPhaseResult<ExampleTypes>;
}