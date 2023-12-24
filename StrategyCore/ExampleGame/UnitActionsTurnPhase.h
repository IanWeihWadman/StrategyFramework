#pragma once

#include "ExampleTurnPhase.h"

namespace Example
{
	class UnitActionsTurnPhase : public ExampleTurnPhase
	{
	public:
		UnitActionsTurnPhase( int32_t player )
			: Player{ player }
		{}

		ExampleTurnPhaseResult HandleInput( std::vector<Input> inputs, std::vector<PlayerState>& playerContexts, ExampleRuleState& state, int32_t player ) override;

		std::unique_ptr<ExampleTurnPhase> GetNext( const ExampleRuleState& state ) const override;

		void OnBegin( std::vector<PlayerState>& playerContexts, ExampleRuleState& state ) override;
		void OnEnd( std::vector<PlayerState>& playerContexts, ExampleRuleState& state ) override;

		int32_t Player = 0;
		bool RecievedInputs = false;
	};
}