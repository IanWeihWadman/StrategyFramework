#pragma once
#include "ExampleTypes.h"
#include "ExampleRuleState.h"

#include <StrategyCore/ActionModeCommon.h>

namespace Example
{
	class EmptyActionMode : public Strategy::ActionModeCommon<ExampleTypes, ExampleRuleState, EmptyActionMode>
	{
	public:
		void GetAllowedInputs( const ExampleRuleState&, std::function<void( Input )> ) const {}
		void GetNextMode( std::function<void( const ModeWrapper& )> ) {}

		BEGIN_ACCESSORS
		END_ACCESSORS
	};
}