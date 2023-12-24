#pragma once

#include "ActionModeManager.h"
#include "ClientFlowManagerBase.h"
#include "MessageTypes.h"

namespace Strategy
{
	template<class Types, class ActionMode, class RuleState>
	class ClientFlowManager : public ClientFlowManagerBase<Types>
	{
	public:
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		virtual void RecieveServerMessage( SerializeBuffer& buf ) override
		{
			Message = ServerMessage<Types, ActionMode, RuleState>{};
			BinaryRead( *Message, buf );

			ModeManager.SetMode( std::move( Message->Mode ) );
			//Timer.Data = Message.Timer;

			if ( !LocalPlayer )
			{
				LocalPlayer = Message->Recipient;
				//Logger.Log( "Local player set to " + msg.Recipient );
			}
			else if ( LocalPlayer != Message->Recipient )
			{
				//Logger.Log( "RecieveServerMessage incorrect recipient, message was for " + msg.Recipient );
			}

			CurrentAction = 0;
		}

		virtual void GetClientMessage( SerializeBuffer& buffer ) override
		{
			ClientMessage<Types> message;
			message.LocalInputs = std::move( InputsToSend );
			message.Player = *LocalPlayer;

			InputsToSend.clear();
			LocallyAppliedActions.clear();

			BinaryWrite( message, buffer );
		}

		virtual void GetAllowedInputs( std::vector<Input> allowedInputs ) override
		{
			allowedInputs.clear();
			ModeManager.GetAllowedInputs( State, [&allowedInputs]( const auto& i ) { allowedInputs.push_back( i ); } );
		}

		virtual std::optional<Action> ApplyAndReturnNextAction() override
		{
			//if ( !Message )
				//throw new Exception( "GetNextAction called but no server message was waiting" );

			if ( CurrentAction < Message->RemoteActions.size() )
			{
				std::visit( [this]( auto& a ) { State.ApplyAction( a ); }, Message->RemoteActions[CurrentAction] );
				return Message->RemoteActions[CurrentAction++];
			}

			SwapStates();
			return {};
		}

		virtual bool HasRemoteAction() override
		{
			return static_cast<bool>( Message ) && Message->RemoteActions.size() > 0;
		}

		virtual InputResult<Action> ReceiveInput( Input input ) override
		{
			if ( HasRemoteAction() )//|| GetTimerResult() != TurnTimerResult.Ongoing )
				return {};

			auto inputResult = ModeManager.ReceiveInput( State, input );

			if ( !inputResult.InputValid )
				return inputResult;

			if ( inputResult.Action )
			{
				std::visit( [this]( auto& a ) { State.ApplyAction( a ); }, *inputResult.Action );
				//Logger.Log( "Action of type " + inputResult.Item2.ToString() + " applied" );
				LocallyAppliedActions.push_back( *inputResult.Action );
			}

			InputsToSend.push_back( input );
			return inputResult;
		}

		virtual int32_t Undo() override
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
				std::visit( [this]( auto& a ) { State.UnapplyAction( a ); }, undone );
				//Logger.Log( "Action of type " + undone.ToString() + " reverted" );
				LocallyAppliedActions.pop_back();
			}

			return inputsUndone;
		}

		/*public TurnTimerResult GetTimerResult()
		{
			return Timer.GetResult();
		}*/

	private:
		void SwapStates()
		{
			if ( !Message )
				return;

			// TODO: Return expected/unexpected to handle exception cases
			// throw new Exception( "SwapStates called but no server message was waiting" );

			State = std::move( Message->State );
			Message = {};
		}

		std::optional<ServerMessage<Types, ActionMode, RuleState>> Message;
		int32_t CurrentAction = 0;
		std::vector<Input> InputsToSend;
		std::vector<Action> LocallyAppliedActions;

		RuleState State;
		ActionModeManager<Types, ActionMode, RuleState> ModeManager;
		
		// TODO: Bring back turn timer implementation
		// ITurnTimer Timer{ get; private set; }

		std::optional<int32_t> LocalPlayer;
	};
}