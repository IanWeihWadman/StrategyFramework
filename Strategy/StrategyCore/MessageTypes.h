#pragma once

#include "ActionMode.h"

namespace Strategy
{
	template<class Types>
	class ClientMessage
	{
		int32_t Player = 0;
		std::vector<typename Types::Input> LocalInputs;
	};

	template<class Types>
	class ServerMessage
	{
		int32_t Recipient = 0;
		RuleState State;
		ActionMode<Types> Mode;
		std::vector<typename Types::Action> RemoteActions;

		// TODO: Bring back turn timer implementation
		//TurnTimerData Timer{ get; set; }
	};
}