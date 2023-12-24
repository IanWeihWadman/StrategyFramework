// StrategyConsole.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <ExampleGame/ExampleGame.h>
#include <StrategyCore/ClientFlowManagerBase.h>

int main()
{
	std::array<std::unique_ptr<Strategy::ClientFlowManagerBase<Example::ExampleTypes>>, 2> clientFlowManagers{ Example::CreateClientFlowManager(), Example::CreateClientFlowManager() };
	auto serverFlowManager = Example::CreateServerFlowManager( 0, 0, 2 );

	std::array<Strategy::SerializeBuffer, 2> ServerOutputBuffers;
	std::array<Strategy::SerializeBuffer, 2> ClientOutputBuffers;

	for ( int32_t i = 0; i < 2; ++i )
	{
		serverFlowManager->GetMessageForPlayer( ServerOutputBuffers[i], i );
		clientFlowManagers[i]->RecieveServerMessage( ServerOutputBuffers[i] );
	}


}