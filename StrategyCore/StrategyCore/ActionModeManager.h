#pragma once

#include "BinaryReflectionIO.h"
#include "VariantActionMode.h"

#include <vector>
#include <memory>

namespace Strategy
{
	template<class Types, class ActionMode, class RuleState>
	class ActionModeManager
	{
	public:
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		bool IsDirty()
		{
			return InputsSinceSave > 0;
		}

		InputResult<Action> ReceiveInput( const RuleState& state, const Input& input )
		{
			auto result = Mode.ReceiveInput( state, input );

			if ( result.InputValid )
			{
				Mode.TryGoToNextMode();
				InputsSinceSave++;
			}

			if ( result.Action )
				SaveMemento();

			return result;
		}

		void GetAllowedInputs( const RuleState& state, std::function<void( const Input& )> destination )
		{
			Mode.GetAllowedInputs( state, destination );
		}

		void SaveMemento()
		{
			if ( NextMementoIndex >= Mementos.size() )
			{
				Mementos.emplace_back();
				ActionInputCounts.push_back( 0 );
			}

			ActionInputCounts[NextMementoIndex] = InputsSinceSave;
			Mementos[NextMementoIndex].PrepareWrite();

			BinaryWrite( Mode, Mementos[NextMementoIndex] );
			NextMementoIndex++;
			InputsSinceSave = 0;
		}

		void SetMode( ActionMode mode )
		{
			Mode = std::move( mode );

			Mode.TryGoToNextMode();
			ResetMementos();
		}

		ActionMode GetMode() const
		{
			return Mode;
		}

		int32_t RevertToMemento()
		{
			int32_t inputsUndone;

			if ( InputsSinceSave > 0 )
			{
				inputsUndone = InputsSinceSave;
			}
			else if ( NextMementoIndex > 1 )
			{
				NextMementoIndex--;
				inputsUndone = ActionInputCounts[NextMementoIndex];
			}
			else
			{
				return 0;
			}

			Mementos[NextMementoIndex - 1].PrepareRead();
			BinaryRead( Mode, Mementos[NextMementoIndex - 1] );

			InputsSinceSave = 0;
			return inputsUndone;
		}

		void ResetMementos()
		{
			InputsSinceSave = 0;
			NextMementoIndex = 0;
			SaveMemento();
		}

	private:
		std::vector<int32_t> ActionInputCounts;
		std::vector<SerializeBuffer> Mementos;
		ActionMode Mode;

		int32_t InputsSinceSave = 0;
		int32_t NextMementoIndex = 0;
	};
}