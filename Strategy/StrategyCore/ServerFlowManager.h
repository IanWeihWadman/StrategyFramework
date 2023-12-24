#pragma once

#include "RuleState.h"
#include "TurnPhase.h"
#include "MessageTypes.h"
#include "IOHelpers.h"

#include <vector>

namespace Strategy
{
	enum class ClientMessageResult
	{
		Send,
		NoSend,
		Error,
		FatalError,
		GameOver
	};

	template<class Types>
	class ServerFlowManager
	{
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		class ActionEntry
		{
			ActionEntry( Action action, int32_t player, int32_t totalPlayers )
			{
				Action = action;
				
				for ( int32_t p = 0; p < totalPlayers; ++p )
					ViewedPlayers.push_back( player == p );
			}

			bool Seen()
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
		}

	public:
		ServerFlowManager( TurnPhase<Types> initialPhase, RuleState<Types> state, int32_t players, GameContext context )
		{
			State = state;
			//WinCondition = condition;

			Context = context;

			PlayerCount = players;

			for ( int i = 0; i < players; ++i )
			{
				PlayerStates.push_back( PlayerState<Types>{ ActionModeManager( context ) } );//, TurnTimer = new TurnTimerManager( context.GetData<TimerConfig>() )
			}

			InitialPhase.SetContext( Context );
			TurnPhase = initialPhase;
			TurnPhase.OnBegin( PlayerStates, State );

			AdvanceTurnPhase();
		}

		ClientMessageResult RecieveClientMessage( SerializeBuffer s )
		{
			auto message = Deserialize<ClientMessage<Types>>( s );
			return RecieveClientMessage( message );
		}

		SerializeBuffer GetMessageForPlayer( int32_t player )
		{
			ServerMessage message;

			message.Recipient = player;
			message.State = State;
			message.Mode = PlayerStates[player].ModeManager.GetMode();
			//PlayerStates[player].TurnTimer.Poll();
			//message.Timer = PlayerStates[player].TurnTimer.Data;
			GetActionsForPlayer( player, [&message]( const auto& a ) { message.RemoteActions.push_back( a ); } );
			ClearSeenActions();

			return Serialize<ServerMessage>( message );
		}

		template<class Out>
		void GetActionsForPlayer( int player, Out output )
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
			Entries.erase_if( []( const auto& e ) { return e.Seen(); } );
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

			TurnPhaseResult phaseResult;
			PlayerStates[player].ModeManager.ResetMementos();

			for ( const auto& input : message.LocalInputs )
			{
				auto result = PlayerStates[player].ModeManager.ReceiveInput( State, input );

				if ( result.Action )
				{
					PendingActions.Add( *result.Action );
					success &= State.ApplyAction( *result.Action );
				}

				success &= result.InputValid;

				if ( !success )
					break;
			}

			if ( success )
			{
				phaseResult = TurnPhase.HandleInput( inputs, PlayerStates, State, player );
				success &= phaseResult.Success;
			}

			// Failed, unwind the action stack to return to original state
			if ( !success )
			{
				//Logger.Log( "Failed to apply inputs, reverting state", Severity.Warning );

				for ( int32_t i = PendingActions.size() - 1; i >= 0; --i )
				{
					if ( !State.UnapplyAction( PendingActions[i] ) )
					{
						//Logger.Log( "Failed to revert " + _pendingActions[i].ToString() + ", game cannot recover", Severity.Error );
						return ClientMessageResult::FatalError;
					}
				}

				// Revert the ModeManager manager to the original state for this mode
				while ( PlayerStates[player].ModeManager.RevertToMemento() > 0 ) {}

				PendingActions.Clear();
				return ClientMessageResult::Error;
			}

			for ( const auto& a : PendingActions )
			{
				Entries.push_back( ActionEntry{ a, player, PlayerCount } );
			}

			foreach( var a in phaseResult.Actions )
			{
				Entries.push_back( ActionEntry( a, -1, PlayerCount ) );

				if ( !State.Apply( a ) )
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

			auto update = phaseResult.UpdatePlayers ? ClientMessageResult::Send : ClientMessageResult::NoSend;
			return update;
		}

		void AdvanceTurnPhase()
		{
			while ( true )
			{
				std::unique_ptr<TurnPhase<Types>> nextPhase = TurnPhase->GetNext( State );

				if ( nextPhase )
				{
					nextPhase.Context = Context;
					TurnPhase.OnEnd( PlayerStates, State );
					nextPhase.OnBegin( PlayerStates, State );
					TurnPhase = std::move( nextPhase );
				}
				else
				{
					break;
				}
			}
		}

		int32_t PlayerCount;

		RuleState<Types> State;
		std::unique_ptr<TurnPhase<Types>> TurnPhase;
		//IWinCondition WinCondition { get; set; }

		std::vector<PlayerState> PlayerStates;
		std::vector<ActionEntry> Entries;

		std::vector<Action> PendingActions;

		GameContext Context;
	};
}