#pragma once

#include <cstdint>
#include <random>

namespace Strategy
{
	class GameContext
	{
		GameContext( uint32_t randomSeed, int32_t gameId )
		{
			GameId = gameId;
			Random.seed( randomSeed );
		}

		int32_t GetGameId()
		{
			return GameId;
		}

		int32_t Next( int lower, int exclusiveUpper )
		{
			std::uniform_int_distribution<> dist( lower, exclusiveUpper - 1 );
			return dist( Random );
		}

	private:
		int32_t GameId;
		std::mt19937 Random;
	}
}