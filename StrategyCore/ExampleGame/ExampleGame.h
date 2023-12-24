#pragma once

#include "ExampleTypes.h"

#include <StrategyCore/ClientFlowManagerBase.h>
#include <StrategyCore/ServerFlowManagerBase.h>

namespace Example
{
	std::unique_ptr<Strategy::ClientFlowManagerBase<ExampleTypes>> CreateClientFlowManager();
	std::unique_ptr<Strategy::ServerFlowManagerBase<ExampleTypes>> CreateServerFlowManager( uint32_t randomSeed, int32_t gameId, int32_t players );
}