#pragma once

#include <StrategyCore/Reflection.h>

namespace Example
{
	struct Position
	{
		int32_t X;
		int32_t Y;

		bool operator==( const Position& ) const = default;
		
		BEGIN_ACCESSORS
			ACCESSOR( X ),
			ACCESSOR( Y )
		END_ACCESSORS
	};

	struct SelectObjectInput
	{
		int32_t ObjectId;

		bool operator==( const SelectObjectInput& ) const = default;

		BEGIN_ACCESSORS
			ACCESSOR( ObjectId )
		END_ACCESSORS
	};

	struct SelectPositionInput
	{
		Position Position;

		bool operator==( const SelectPositionInput& ) const = default;

		BEGIN_ACCESSORS
			ACCESSOR( Position )
		END_ACCESSORS
	};

	struct MoveUnitAction
	{
		int32_t ObjectId;
		Position Destination;
		std::optional<Position> Origin;

		BEGIN_ACCESSORS
			ACCESSOR( ObjectId ),
			ACCESSOR( Destination ),
			ACCESSOR( Origin )
		END_ACCESSORS
	};

	using Input = std::variant<SelectObjectInput, SelectPositionInput>;
	using Action = std::variant<MoveUnitAction>;

	struct ExampleTypes
	{
		using Input = Input;
		using Action = Action;
	};

	struct ModeWrapper;
}