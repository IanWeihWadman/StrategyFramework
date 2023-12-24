#include "pch.h"
#include "MoveUnitActionMode.h"

#include "ActionModeRegister.h"

#include "ExampleRuleState.h"

namespace Example
{
	void MoveUnitActionMode::GetAllowedInputs( const ExampleRuleState& state, std::function<void( Input )> destination ) const
	{
		if ( RemainingMoves < 0 )
			return;

		if ( !SelectedUnitId )
		{
			for ( const auto& unit : state.Units )
			{
				if ( unit.Owner == OwningPlayer )
				{
					destination( Input{ SelectObjectInput{ unit.ObjectId } } );
				}
			}
		}
		else
		{
			for ( int32_t x = -2; x <= 2; ++x )
			{
				for ( int32_t y = -2; y <= 2; ++y )
				{
					auto result = state.GetUnit( *SelectedUnitId )->Position;
					result.X += x;
					result.Y += y;

					destination( Input{ SelectPositionInput{ result } } );
				}
			}
		}
	}

	void MoveUnitActionMode::GetNextMode( std::function<void( const ModeWrapper& )> handleNextFn ) {}

	std::optional<Action> MoveUnitActionMode::HandleInput( const SelectObjectInput& input )
	{
		SelectedUnitId = input.ObjectId;
		return {};
	}

	std::optional<Action> MoveUnitActionMode::HandleInput( const SelectPositionInput& input )
	{
		int32_t unitId = *SelectedUnitId;
		SelectedUnitId.reset();
		RemainingMoves--;

		return MoveUnitAction{ unitId, input.Position };
	}

}