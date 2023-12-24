#pragma once

#include "ExampleTypes.h"

#include <StrategyCore/Reflection.h>

#include <vector>

namespace Example
{
	struct Unit
	{
		int32_t Owner;
		int32_t ObjectId;
		int32_t UnitType;
		Position Position;

		BEGIN_ACCESSORS
			ACCESSOR( Owner ),
			ACCESSOR( ObjectId ),
			ACCESSOR( UnitType ),
			ACCESSOR( Position )
		END_ACCESSORS
	};

	class ExampleRuleState
	{
	public:
		std::vector<Unit> Units;

		const Unit* GetUnit( int32_t ObjectId ) const;
		Unit* GetUnit( int32_t ObjectId );

		bool ApplyAction( MoveUnitAction& );
		bool UnapplyAction( MoveUnitAction& );

		BEGIN_ACCESSORS
			ACCESSOR( Units )
		END_ACCESSORS
	};
}