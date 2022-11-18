// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/CaptureTheFlagGameMode.h"
#include "SteamShooter/Public/Weapon/Flag.h"
#include "SteamShooter/Public/CaptureTheFlag/FlagZone.h"
#include "SteamShooter/Public/GameStates/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEleminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEleminated(ElimmedCharacter, VictimController, AttackerController);
}

//caled when Flag Captured 
void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState);
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScored();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScored();
		}
	}
}
