#pragma once

#include "ActionMode.h"
#include "IOHelpers.h"

#include <vector>
#include <memory>

namespace Strategy
{
	template<class Types>
	class ActionModeManager
	{
		using Action = typename Types::Action;
		using Input = typename Types::Input;

		bool IsDirty()
		{
			return InputsSinceSave > 0;
		}

		InputResult RecieveInput( const RuleState<Types>& state, const Input& input )
		{
			auto result = Mode->ReceiveInput( state, input );

			if ( result.InputValid )
			{
				TryGotoNextMode();
				InputsSinceSave++;
			}

			if ( result.Action )
				SaveMemento();

			return result;
		}

		void GetAllowedInputs( const RuleState<Types>& state, std::function<void( const Input& )> destination )
		{
			Mode->GetAllowedInputs( state, destination );
		}

		private void SaveMemento()
		{
			if ( NextMementoIndex >= Mementos.Count )
			{
				Mementos.emplace_back();
				ActionInputCounts.push_back( 0 );
			}

			ActionInputCounts[NextMementoIndex] = InputsSinceSave;
			Mementos[NextMementoIndex].GetStream().Position = 0;

			Mementos[NextMementoIndex] = Serialize( Mode );
			NextMementoIndex++;
			InputsSinceSave = 0;
		}

		public void SetMode( std::unique_ptr<ActionMode<Types>> mode )
		{
			Mode = std::move( mode );

			TryGotoNextMode();
			ResetMementos();
		}

		public int RevertToMemento()
		{
			int inputsUndone;

			if ( InputsSinceSave > 0 )
			{
				inputsUndone = InputsSinceSave;
			}
			else if ( NextMementoIndex > 1 )
			{
				NextMementoIndex--;
				inputsUndone = InputCounts[NextMementoIndex];
			}
			else
			{
				return 0;
			}

			Mementos[NextMementoIndex - 1].GetStream().Position = 0;
			Mode = Deserialize<ActionMode<Types>>( Mementos[NextMementoIndex - 1] );

			InputsSinceSave = 0;
			return inputsUndone;
		}

		public void ResetMementos()
		{
			InputsSinceSave = 0;
			NextMementoIndex = 0;
			SaveMemento();
		}

		void TryGotoNextMode()
		{
			while ( auto next = Mode.GetNextMode(); next )
			{
				Mode = std::move( next );
			}
		}

	private:
		std::vector<int32_t> ActionInputCounts;
		std::vector<std::vector<uint8_t>> Mementos;
		std::unique_ptr<ActionMode<Types>> Mode;

		int32_t InputsSinceSave = 0;
		int32_t NextMementoIndex = 0;
	};
}