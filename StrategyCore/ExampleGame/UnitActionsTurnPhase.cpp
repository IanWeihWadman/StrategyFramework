#include "pch.h"
#include "UnitActionsTurnPhase.h"

#include "ActionModeRegister.h"

namespace Example
{
	ExampleTurnPhaseResult UnitActionsTurnPhase::HandleInput( std::vector<Input> inputs, std::vector<PlayerState>& playerContexts, ExampleRuleState& state, int32_t player )
	{
		if ( player != Player )
			return ExampleTurnPhaseResult::Invalid();

		return ExampleTurnPhaseResult::DoUpdatePlayers( {} );
	}

	std::unique_ptr<ExampleTurnPhase> UnitActionsTurnPhase::GetNext( const ExampleRuleState& ) const
	{
		if ( RecievedInputs )
			return std::make_unique<UnitActionsTurnPhase>( 1 - Player );

		return {};
	}

	void UnitActionsTurnPhase::OnBegin( std::vector<PlayerState>& playerContexts, ExampleRuleState& )
	{
		for ( int32_t i = 0; i < static_cast<int32_t>( playerContexts.size() ); ++i )
		{
			if ( i == Player )
				playerContexts[i].SetMode( MoveUnitActionMode( 3, Player ) );
			else 
				playerContexts[i].SetMode( EmptyActionMode() );
		}
	}

	void UnitActionsTurnPhase::OnEnd( std::vector<PlayerState>&, ExampleRuleState& )
	{

	}
}