#pragma once

#include "PlayerState.h"
#include "GameContext.h"

#include <vector>

namespace Strategy
{
	template<class Types>
	class TurnPhaseResult
	{
	public:
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		static TurnPhaseResult Invalid()
		{
			return TurnPhaseResult( false, false, {} );
		}

		static TurnPhaseResult DoUpdatePlayers( std::initializer_list<Action> actions )
		{
			return TurnPhaseResult( true, true, actions );
		}

		static TurnPhaseResult SkipUpdatePlayers( std::initializer_list<Action> actions )
		{
			return TurnPhaseResult( true, false, actions );
		}

		bool IsSuccess() const { return Success; }
		bool ShouldUpdatePlayers() const { return UpdatePlayers; }
		std::vector<Action> GetActions() const { return Actions; }

	private:
		TurnPhaseResult( bool success, bool updatePlayers, std::initializer_list<Action> actions )
		{
			Success = success;
			UpdatePlayers = updatePlayers;
			Actions = { actions };
		}

		bool Success;
		bool UpdatePlayers;
		std::vector<Action> Actions;
	};

	template<class Types, class ActionMode, class RuleState>
	class TurnPhase
	{
	public:
		using Action = typename Types::Action;
		using Input = typename Types::Input;
		using PlayerState = PlayerState<Types, ActionMode, RuleState>;

		virtual TurnPhaseResult<Types> HandleInput( std::vector<Input> inputs, std::vector<PlayerState>& playerContexts, RuleState& state, int32_t player ) = 0;

		virtual std::unique_ptr<TurnPhase> GetNext( const RuleState& state ) const = 0;

		virtual void OnBegin( std::vector<PlayerState>& playerContexts, RuleState& state ) = 0;
		virtual void OnEnd( std::vector<PlayerState>& playerContexts, RuleState& state ) = 0;

		void SetContext( std::shared_ptr<GameContext> context )
		{
			Context = context;
		}

	protected:

		std::shared_ptr<GameContext> Context;
	};
}