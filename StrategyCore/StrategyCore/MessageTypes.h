#pragma once

#include "VariantActionMode.h"

#include "Reflection.h"

namespace Strategy
{
	template<class Types>
	struct ClientMessage
	{
		int32_t Player = 0;
		std::vector<typename Types::Input> LocalInputs;

		BEGIN_ACCESSORS
			ACCESSOR( Player ),
			ACCESSOR( LocalInputs )
		END_ACCESSORS
	};

	template<class Types, class ActionMode, class RuleState>
	struct ServerMessage
	{
		int32_t Recipient = 0;
		RuleState State;
		ActionMode Mode;
		std::vector<typename Types::Action> RemoteActions;

		BEGIN_ACCESSORS
			ACCESSOR( Recipient ),
			ACCESSOR( State ),
			ACCESSOR( Mode ),
			ACCESSOR( RemoteActions )
		END_ACCESSORS

		// TODO: Bring back turn timer implementation
		//TurnTimerData Timer{ get; set; }
	};
}