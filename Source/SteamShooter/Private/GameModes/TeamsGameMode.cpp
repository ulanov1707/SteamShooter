// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/TeamsGameMode.h"
#include "SteamShooter/Public/GameStates/BlasterGameState.h"
#include "SteamShooter/Public/PlayerStates/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"


ATeamsGameMode::ATeamsGameMode()
{
	bTeamMatch = true;
}

//assigns a team to newly logged in player 
//handles players who joined a during already playing match
void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		ABlasterPlayerState* BlasterPlayState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayState && BlasterPlayState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BlasterPlayState);
				BlasterPlayState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BlasterPlayState);
				BlasterPlayState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

//remove Exited player from team
void ATeamsGameMode::Logout(AController* Exiting)
{
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* BlasterPlayState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if (BGameState && BlasterPlayState)
	{
		if (BGameState->RedTeam.Contains(BlasterPlayState))
		{
			BGameState->RedTeam.Remove(BlasterPlayState);
		}
		if (BGameState->BlueTeam.Contains(BlasterPlayState))
		{
			BGameState->BlueTeam.Remove(BlasterPlayState);
		}
	}

}



// when match starts if there is player without a team assigna a team for him 
void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		/** GameState->PlayrArray  - Array of all PlayerStates, maintained on both server and clients (PlayerStates are always relevant) */
		for (auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;

	if (VictimPState == AttackerPState) // player self damaging 
	{
		return BaseDamage;
	}

	//Players CAN'T damage teammates
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}


void ATeamsGameMode::PlayerEleminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	Super::PlayerEleminated(ElimmedCharacter, VictimController, AttackerController);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScored();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScored();
		}
	}
}