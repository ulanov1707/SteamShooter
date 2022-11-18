
// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGameMode.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "SteamShooter/Public/PlayerStates/BlasterPlayerState.h"
#include "SteamShooter/Public/GameStates/BlasterGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;//GM will set match state to  "WaitingToStart" untill we manually start match with StartMatch()
}



void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	LevelStartingTime = GetWorld()->GetTimeSeconds();

}

void ABlasterGameMode::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress) 
	{
		CountTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountTime <= 0.f) 
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown) 
	{
		CountTime =CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountTime <= 0.f) 
		{
			RestartGame();// will work ony on packaged game 
		}
	}
}


void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	

	//loop through all PlayerControllers
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator() ; it;++it) 
	{
		ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(*it);
		if(PC)
		{
			PC->OnMatchStateSet(MatchState,bTeamMatch);
		}
	}

}


float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}


void ABlasterGameMode::PlayerEleminated(ABlasterCharacter* ElimedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackedPlayerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackedPlayerController ? Cast<ABlasterPlayerState>(AttackedPlayerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	//if player killed somebody not himself
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState) 
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyLeading; // Leading players lift before update
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers) 
		{
			PlayersCurrentlyLeading.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.f);//updates HUD value of Killer player
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState)) 
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->Multicast_GainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyLeading.Num(); i++) 
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyLeading[i])) 
			{
				ABlasterCharacter* Looser = Cast<ABlasterCharacter>(PlayersCurrentlyLeading[i]->GetPawn());
				if (Looser) 
				{
					Looser->Multicast_LostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState) 
	{
		VictimPlayerState->AddToDefeats(1);//updates HUD value of Killed player
	}
	if (ElimedCharacter)
	{
		//killed by somebody 
		ElimedCharacter->Eliminate(false);
	}

	//Each player updates his Elimination Widget
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			BlasterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState); //tells to each player who killed who  , keep in mind that it takes all PC and makes them to run it , and PC on the other hand will Run it on owning Client only(Autonomous proxies)

		}
	}
}
	


void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter) 
	{
		ElimmedCharacter->Reset();//Detached chracter from Contoller (Kind of Unposseses)
		ElimmedCharacter->Destroy();
	}

	//i think We need AController because contoller can  posses a Pawn,which pawn to spawn will be decided based on this GM's "default pawn" setting 
	if (ElimmedController) 
	{
		TArray<AActor*>PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this,APlayerStart::StaticClass(),PlayerStarts);
		int32 RandStart = FMath::RandRange(0, PlayerStarts.Num() - 1);

		//Spawns a Player  at player start location(can be any actor , but better to use playerstart )
		RestartPlayerAtPlayerStart(ElimmedController,PlayerStarts[RandStart]);
	}

}

//handles player Levaing the game (On Quit/ECS pressed)
//removes leaving player from topsocrers and play elimination effects ,sound , anim ets
void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{

	if (PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving)) 
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	ABlasterCharacter * LeavingCharacter = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());

	//player leaving the game 
	LeavingCharacter->Eliminate(true);

}
