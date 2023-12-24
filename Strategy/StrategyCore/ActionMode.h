#pragma once

#include "RuleState.h"

#include <functional>
#include <tuple>
#include <memory>
#include <optional>

namespace Strategy
{
	template<class Action>
	struct InputResult
	{
		bool InputValid = false;
		std::optional<Action> Action = {};
	};

	template<class Types>
	class ActionMode
	{
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		virtual void GetAllowedInputs( const RuleState<Types>& state, std::function<void(const Input&)> destination ) = 0;
		virtual InputResult<Action> ReceiveInput( const RuleState<Types>& state, const Input& input ) = 0;
		virtual std::unique_ptr<ActionMode<Types>> GetNextMode() = 0;
	};
}