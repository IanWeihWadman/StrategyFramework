#pragma once

#include "ServerFlowManagerBase.h"
#include "ActionModeManager.h"
#include "PlayerState.h"
#include "TurnPhase.h"
#include "GameContext.h"
#include "MessageTypes.h"

#include <vector>

namespace Strategy
{
	template<class Types, class ActionMode, class RuleState>
	class ServerFlowManager : public ServerFlowManagerBase<Types>
	{
		using Action = typename Types::Action;
		using Input = typename Types::Input;
		using PlayerState = PlayerState<Types, ActionMode, RuleState>;
		using TurnPhase = TurnPhase<Types, ActionMode, RuleState>;

		struct ActionEntry
		{
			ActionEntry( Action action, int32_t player, int32_t totalPlayers )
			{
				Action = action;

				for ( int32_t p = 0; p < totalPlayers; ++p )
					ViewedPlayers.push_back( player == p );
			}

			bool Seen() const
			{
				for ( bool b : ViewedPlayers )
				{
					if ( !b )
						return false;
				}

				return true;
			}

			Action Action;
			std::vector<bool> ViewedPlayers;
		};

	public:
		ServerFlowManager( std::unique_ptr<TurnPhase> initialPhase, RuleState state, int32_t players, std::shared_ptr<GameContext> context )
		{
			State = state;
			//WinCondition = condition;

			Context = context;

			PlayerCount = players;

			for ( int i = 0; i < players; ++i )
			{
				PlayerStates.push_back( PlayerState{} );//, TurnTimer = new TurnTimerManager( context.GetData<TimerConfig>() )
			}

			initialPhase->SetContext( Context );
			CurrentPhase = std::move( initialPhase );
			CurrentPhase->OnBegin( PlayerStates, State );

			AdvanceTurnPhase();
		}

		ClientMessageResult RecieveClientMessage( SerializeBuffer& buf ) override
		{
			ClientMessage<Types> message;
			BinaryRead( message, buf );
			return RecieveClientMessage( message );
		}

		void GetMessageForPlayer( SerializeBuffer& buf, int32_t player ) override
		{
			ServerMessage<Types, ActionMode, RuleState> message;

			message.Recipient = player;
			message.State = State;
			message.Mode = PlayerStates[player].GetMode();
			//PlayerStates[player].TurnTimer.Poll();
			//message.Timer = PlayerStates[player].TurnTimer.Data;
			GetActionsForPlayer( player, [&message]( const auto& a ) { message.RemoteActions.push_back( a ); } );
			ClearSeenActions();

			BinaryWrite( message, buf );
		}

		void GetActionsForPlayer( int player, std::function<void( Action )> output )
		{
			for ( auto& entry : Entries )
			{
				if ( !entry.ViewedPlayers[player] )
				{
					entry.ViewedPlayers[player] = true;
					output( entry.Action );
				}
			}
		}

		void ClearSeenActions()
		{
			std::erase_if( Entries, []( const auto& e ) { return e.Seen(); } );
		}

		/*ClientMessageResult Poll(object obj)
		{
			for ( int i = 0; i < _playerStates.Count; ++i )
			{
				var playerState = _playerStates[i];

				playerState.TurnTimer.Poll();
				var result = playerState.TurnTimer.GetResult();

				switch ( result )
				{
					case TurnTimerResult.EndGame:
					{
						return ClientMessageResult.GameOver;
					}
					case TurnTimerResult.NoSubmit:
					{
						return RecieveClientMessage( new ClientMessage { LocalInputs = new List<InputVariant>(), Player = i } );
					}
					default:
					{
						break;
					}
				}
			}

			return ClientMessageResult.NoSend;
		}*/

		ClientMessageResult RecieveClientMessage( ClientMessage<Types> message )
		{
			bool success = true;
			int32_t player = message.Player;
			auto inputs = std::move( message.LocalInputs );

			std::optional<TurnPhaseResult<Types>> phaseResult;
			PlayerStates[player].ResetMementos();

			for ( const auto& input : inputs )
			{
				auto result = PlayerStates[player].ReceiveInput( State, input );

				if ( result.Action )
				{
					success &= std::visit( [this]( auto& a ) { return State.ApplyAction( a ); }, *result.Action );
					PendingActions.push_back( *result.Action );
				}

				success &= result.InputValid;

				if ( !success )
					break;
			}

			if ( success )
			{
				phaseResult = CurrentPhase->HandleInput( inputs, PlayerStates, State, player );
				success &= phaseResult->IsSuccess();
			}

			// Failed, unwind the action stack to return to original state
			if ( !success )
			{
				//Logger.Log( "Failed to apply inputs, reverting state", Severity.Warning );

				for ( int32_t i = static_cast<int32_t>( PendingActions.size() - 1 ); i >= 0; --i )
				{
					if ( !std::visit( [this]( auto& a ) { return State.UnapplyAction( a ); }, PendingActions[i] ) )
					{
						//Logger.Log( "Failed to revert " + _pendingActions[i].ToString() + ", game cannot recover", Severity.Error );
						return ClientMessageResult::FatalError;
					}
				}

				// Revert the ModeManager manager to the original state for this mode
				while ( PlayerStates[player].RevertToMemento() > 0 ) {}

				PendingActions.clear();
				return ClientMessageResult::Error;
			}

			for ( const auto& pending : PendingActions )
			{
				Entries.push_back( ActionEntry{ pending, player, PlayerCount } );
			}

			for( auto& phaseAction : phaseResult->GetActions() )
			{
				Entries.push_back( ActionEntry( phaseAction, -1, PlayerCount ) );

				if ( !std::visit( [this]( auto& a ) { return State.ApplyAction( a ); }, phaseAction ) )
				{
					//Logger.Log( "Failed to apply server driven action " + a.ToString() + ", game cannot recover", Severity.Error );
					return ClientMessageResult::FatalError;
				}
			}

			PendingActions.clear();

			AdvanceTurnPhase();

			//int winner = WinCondition.GetWinner( State );

			//if ( winner != -1 )
			//	return ClientMessageResult.GameOver;

			auto update = phaseResult->ShouldUpdatePlayers() ? ClientMessageResult::Send : ClientMessageResult::NoSend;
			return update;
		}

		void AdvanceTurnPhase()
		{
			while ( true )
			{
				std::unique_ptr<TurnPhase> nextPhase = CurrentPhase->GetNext( State );

				if ( nextPhase )
				{
					nextPhase->SetContext( Context );
					CurrentPhase->OnEnd( PlayerStates, State );
					nextPhase->OnBegin( PlayerStates, State );
					CurrentPhase = std::move( nextPhase );
				}
				else
				{
					break;
				}
			}
		}

		int32_t PlayerCount;

		RuleState State;
		std::unique_ptr<TurnPhase> CurrentPhase;
		//IWinCondition WinCondition { get; set; }

		std::vector<PlayerState> PlayerStates;
		std::vector<ActionEntry> Entries;

		std::vector<Action> PendingActions;

		std::shared_ptr<GameContext> Context;
	};
}