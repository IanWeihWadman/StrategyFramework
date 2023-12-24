// ExampleGame.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "ExampleGame.h"
#include "ExampleRuleState.h"
#include "ActionModeRegister.h"
#include "UnitActionsTurnPhase.h"

#include <StrategyCore/ClientFlowManager.h>
#include <StrategyCore/ServerFlowManager.h>

namespace Example
{
	std::unique_ptr<Strategy::ClientFlowManagerBase<ExampleTypes>> CreateClientFlowManager()
	{
		return std::make_unique<Strategy::ClientFlowManager<ExampleTypes, Strategy::VariantActionMode<ExampleTypes, ExampleRuleState, ExampleActionModes>, ExampleRuleState>>();
	}

	std::unique_ptr<Strategy::ServerFlowManagerBase<ExampleTypes>> CreateServerFlowManager( uint32_t randomSeed, int32_t gameId, int32_t players )
	{
		return std::make_unique<Strategy::ServerFlowManager<ExampleTypes, Strategy::VariantActionMode<ExampleTypes, ExampleRuleState, ExampleActionModes>, ExampleRuleState>>
		(
			std::make_unique<UnitActionsTurnPhase>( 0 ),
			ExampleRuleState{},
			players,
			std::make_shared<Strategy::GameContext>( randomSeed, gameId )
		);
	}
}