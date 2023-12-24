#pragma once

#include "Reflection.h"
#include "ActionModeCommon.h"

#include <functional>
#include <tuple>
#include <memory>
#include <optional>

namespace Strategy
{
	template<class Types, class RuleState, class Variant>
	class VariantActionMode
	{
	public:
		using Input = typename Types::Input;
		using Action = typename Types::Action;

		VariantActionMode() = default;

		template<class T>
		VariantActionMode( T mode )
			: Self{ mode }
		{}

		template<class T>
		void SetMode( T mode )
		{
			Self = mode;
		}

		void GetAllowedInputs( const RuleState& state, std::function<void( Input )> destination ) const
		{
			std::visit( [&state, &destination]( const auto& m ) { m.GetAllowedInputs( state, destination ); }, Self );
		}

		InputResult<Action> ReceiveInput( const RuleState& state, const Input& input )
		{
			return std::visit( [&state, &input]( auto& m ) { return m.ReceiveInput( state, input ); }, Self );
		}

		template<class T>
		void GoToNextMode( T t )
		{
			Self = t.Value;
		}

		void TryGoToNextMode()
		{
			size_t count = 0;
			while ( count++ < 128 )
			{
				bool called = false;

				auto visitor = [this, &called]( auto& mode )
				{ 
					mode.GetNextMode
					(
						[this, &called]( const auto& wrapper )
						{
							called = true;
							GoToNextMode( wrapper );
						}
					);
				};
				
				std::visit( visitor, Self );

				if ( !called )
					return;
			}

			// TODO: Figure out assertions better here
		}

		BEGIN_ACCESSORS
			ACCESSOR( Self )
		END_ACCESSORS

		Variant Self;
	};
}