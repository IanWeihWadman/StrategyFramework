#pragma once

#include "BinaryReflectionIO.h"
#include "MessageTypes.h"

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
	class ServerFlowManagerBase
	{
	public:
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		virtual ClientMessageResult RecieveClientMessage( SerializeBuffer& buf ) = 0;
		virtual void GetMessageForPlayer( SerializeBuffer& buf, int32_t player ) = 0;
	};
}