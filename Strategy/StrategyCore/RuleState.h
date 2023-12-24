#pragma once

#include "StrategyTypes.h"
#include "IOHelpers.h"

#include <memory>

namespace Strategy
{
	template<class Types>
	class RuleState
	{
		using Action = typename Types::Action;
		using Input = typename Types::Input;

	public:
		SerializeBuffer Serialize() const
		{
			return Self->Serialize();
		}

		void Deserialize( const SerializeBuffer& buffer )
		{
			Self->Deserialize( buffer );
		}

		void ApplyAction( const Action& action )
		{
			Self->ApplyAction( action );
		}

		void UnapplyAction( const Action& action )
		{
			Self->UnapplyAction( action );
		}

	private:

		class Concept
		{
		public:
			virtual SerializeBuffer Serialize() const = 0;
			virtual void Deserialize( const SerializeBuffer& buffer ) = 0;
			virtual void ApplyAction( const Action& action ) = 0;
			virtual void UnapplyAction( const Action& action ) = 0;
		};

		template<class T>
		class Model : public Concept
		{
		public:
			SerializeBuffer Serialize() const override
			{
				return Serialize<T>( value );
			}

			void Deserialize( const SerializeBuffer& buffer ) override
			{
				value = Deserialize<T>( buffer );
			}

			void ApplyAction( const Action& action )
			{
				value.ApplyAction( action );
			}

			void UnapplyAction( const Action& action )
			{
				value.UnapplyAction( action );
			}

			T value;
		};

		std::unique_ptr<Concept> Self;
	};
}