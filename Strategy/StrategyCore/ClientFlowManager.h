#pragma once

#include "ActionModeManager.h"
#include "RuleState.h"
#include "MessageTypes.h"

namespace Strategy
{
	template<class Types>
	class ClientFlowManager
	{
	public:
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		void RecieveServerMessage( SerializeBuffer s )
		{
			Message = Deserialize<ServerMessage>( s );

			ModeManager.SetMode( std::move( Message.Mode ) );
			//Timer.Data = Message.Timer;

			if ( !LocalPlayer )
			{
				LocalPlayer = Message.Recipient;
				//Logger.Log( "Local player set to " + msg.Recipient );
			}
			else if ( LocalPlayer != msg.Recipient )
			{
				//Logger.Log( "RecieveServerMessage incorrect recipient, message was for " + msg.Recipient );
			}

			CurrentAction = 0;
		}

		SerializeBuffer GetClientMessage()
		{
			ClientMessage<Types> message = new ClientMessage();
			message.LocalInputs = InputsToSend;
			message.Player = *LocalPlayer;

			InputsToSend.clear();
			LocallyAppliedActions.clear();

			return Serialize( message );
		}

		void GetAllowedInputs( std::vector<Input> allowedInputs )
		{
			allowedInputs.clear();
			ModeManager.GetAllowedInputs( State, [&allowedInputs]( const auto& i ) { allowedInputs.push_back( i ); } );
		}

		std::optional<Action> ApplyAndReturnNextAction()
		{
			//if ( !Message )
				//throw new Exception( "GetNextAction called but no server message was waiting" );

			if ( CurrentAction < Message.RemoteActions.size() )
			{
				State.ApplyAction( Message.RemoteActions[CurrentAction] );
				return Message.RemoteActions[CurrentAction++];
			}

			SwapStates();
			return {};
		}

		bool HasRemoteAction()
		{
			return static_cast<bool>( Message ) && Message->RemoteActions.size() > 0;
		}

		InputResult<Action> ReceiveInput( Input input )
		{
			if ( HasRemoteAction() )//|| GetTimerResult() != TurnTimerResult.Ongoing )
				return {};

			auto inputResult = ModeManager.ReceiveInput( State, input );

			if ( !inputResult.InputValid )
				return inputResult;

			if ( inputResult.Action )
			{
				State.ApplyAction( *inputResult.Action );
				//Logger.Log( "Action of type " + inputResult.Item2.ToString() + " applied" );
				LocallyAppliedActions.push_back( *inputResult.Action );
			}

			InputsToSend.push_back( input );
			return inputResult;
		}

		int32_t Undo()
		{
			bool undoingLocalAction = !ModeManager.IsDirty();
			int32_t inputsUndone = ModeManager.RevertToMemento();

			if ( inputsUndone == 0 )
				return 0;

			InputsToSend.resize( InputsToSend.size() - static_cast<size_t>( inputsUndone ) );

			if ( undoingLocalAction )
			{
				//if ( inputsUndone == 0 )
					//throw new Exception( "Action undone without any inputs being undone" );

				auto undone = LocallyAppliedActions.back();
				State.UnapplyAction( undone );
				//Logger.Log( "Action of type " + undone.ToString() + " reverted" );
				LocallyAppliedActions.pop_back();
			}

			return inputsUndone;
		}

		/*public TurnTimerResult GetTimerResult()
		{
			return Timer.GetResult();
		}*/

		private void SwapStates()
		{
			if ( !Message )
				return;

			// TODO: Return expected/unexpected to handle exception cases
			// throw new Exception( "SwapStates called but no server message was waiting" );

			State = std::move( Message.State );
			Message = {};
		}

		std::optional<ServerMessage> Message;
		int32_t CurrentAction = 0;
		std::vector<Input> InputsToSend;
		std::vector<Action> LocallyAppliedActions;

		RuleState<Types> State;
		ActionModeManager<Types> ModeManager;
		
		// TODO: Bring back turn timer implementation
		// ITurnTimer Timer{ get; private set; }

		std::optional<int32_t> LocalPlayer;
	};

}