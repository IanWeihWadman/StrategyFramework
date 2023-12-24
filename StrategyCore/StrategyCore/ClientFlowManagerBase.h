#pragma once

#include "ActionModeCommon.h"
#include "BinaryReflectionIO.h"

#include <cstdint>
#include <vector>
#include <optional>

namespace Strategy
{
	template<class Types>
	class ClientFlowManagerBase
	{
	public:
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		virtual void RecieveServerMessage( SerializeBuffer& buf ) = 0;
		virtual void GetClientMessage( SerializeBuffer& buf ) = 0;
		virtual void GetAllowedInputs( std::vector<Input> allowedInputs ) = 0;
		virtual std::optional<Action> ApplyAndReturnNextAction() = 0;
		virtual bool HasRemoteAction() = 0;
		virtual InputResult<Action> ReceiveInput( Input input ) = 0;
		virtual int32_t Undo() = 0;
	};
}