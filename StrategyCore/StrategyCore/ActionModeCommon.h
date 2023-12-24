#pragma once

namespace Strategy
{
	template<class D, class I>
	concept HandlesInputType = requires(D & d, const I & i)
	{
		{ d.HandleInput( i ) };
	};

	template<class A>
	struct InputResult
	{
		bool InputValid = false;
		std::optional<A> Action = {};

		InputResult() = default;
		
		InputResult( std::optional<A> action )
			: InputValid{ true }
			, Action{ action }
		{}
	};

	template<class Types, class RuleState, class Derived>
	class ActionModeCommon
	{
	public:
		using Input = typename Types::Input;
		using Action = typename Types::Action;

		InputResult<Action> ReceiveInput( const RuleState& state, const Input& input )
		{
			bool found = false;
			auto search = [&input, &found]( const auto& i )
			{
				if ( input == i )
					found = true;
			};

			static_cast<Derived*>( this )->GetAllowedInputs( state, search );

			if ( !found )
				return {};
			else
				return { HandleInputBase( input ) };
		}

		std::optional<Action> HandleInputBase( const Input& input )
		{
			return std::visit( [this]( const auto& i ) { return TryCallHandleInput( i ); }, input );
		}

	private:		

		template<class I> 
			requires ( HandlesInputType<Derived, I> )
		std::optional<Action> TryCallHandleInputImpl( const I& i, int64_t )
		{
			return static_cast<Derived*>( this )->HandleInput( i );
		}

		template<class I>
		std::optional<Action> TryCallHandleInputImpl( const I& i, int32_t )
		{
			return {};
		}

		template<class I>
		std::optional<Action> TryCallHandleInput( const I& i )
		{
			return TryCallHandleInputImpl( i, static_cast<int64_t>(0) );
		}
	};
}