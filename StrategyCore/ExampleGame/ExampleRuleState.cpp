#include "pch.h"
#include "ExampleRuleState.h"

#include <ranges>

namespace Example
{
	const Unit* ExampleRuleState::GetUnit( int32_t ObjectId ) const
	{
		auto found = std::ranges::find_if( Units, [ObjectId]( const auto& u ) { return u.ObjectId == ObjectId; } );
		return found != Units.cend() ? &*found : nullptr;
	}

	Unit* ExampleRuleState::GetUnit( int32_t ObjectId )
	{
		auto found = std::ranges::find_if( Units, [ObjectId]( const auto& u ) { return u.ObjectId == ObjectId; } );
		return found != Units.cend() ? &*found : nullptr;
	}

	bool ExampleRuleState::ApplyAction( MoveUnitAction& action )
	{
		auto* unit = GetUnit( action.ObjectId );

		if ( !unit || action.Origin )
			return false;

		action.Origin = unit->Position;
		unit->Position = action.Destination;
		
		return true;
	}

	bool ExampleRuleState::UnapplyAction( MoveUnitAction& action )
	{
		auto* unit = GetUnit( action.ObjectId );

		if ( !unit || !action.Origin )
			return false;

		unit->Position = *action.Origin;
		action.Origin = {};

		return true;
	}
}