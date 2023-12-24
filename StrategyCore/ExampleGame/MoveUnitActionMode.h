#pragma once

#include "ExampleTypes.h"
#include "ExampleRuleState.h"

#include <StrategyCore/ActionModeCommon.h>

namespace Example
{
	class MoveUnitActionMode : public Strategy::ActionModeCommon<ExampleTypes, ExampleRuleState, MoveUnitActionMode>
	{
	public:
		MoveUnitActionMode() = default;

		MoveUnitActionMode( int32_t moves, int32_t player )
			: RemainingMoves{ moves }
			, OwningPlayer{ player }
		{}

		void GetAllowedInputs( const ExampleRuleState& state, std::function<void( Input )> destination ) const;
		void GetNextMode( std::function<void( const ModeWrapper& )> handleNextFn );

		std::optional<Action> HandleInput( const SelectObjectInput& input );
		std::optional<Action> HandleInput( const SelectPositionInput& input );

		std::optional<int32_t> SelectedUnitId;
		int32_t OwningPlayer = 0;
		int32_t RemainingMoves = 0;

		BEGIN_ACCESSORS
			ACCESSOR( SelectedUnitId ),
			ACCESSOR( OwningPlayer ),
			ACCESSOR( RemainingMoves )
		END_ACCESSORS
	};
}