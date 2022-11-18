// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"
#include"GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"


//called when player joined game
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//number of all player of on the level with Lobby GM set
	int32  NumOfPlayers = GameState.Get()->PlayerArray.Num();
	

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);

		if (NumOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;


			
				FString MatchType = Subsystem->DesiredMatchType;
				if (MatchType == "DeathMatch")
				{
					World->ServerTravel(FString("/Game/Maps/DeathMatch?listen"));
				}
				else if (MatchType == "TeamDeathMatch")
				{
					World->ServerTravel(FString("/Game/Maps/TeamDeathMatch?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlag?listen"));
				}
			}
		}
	}
}
