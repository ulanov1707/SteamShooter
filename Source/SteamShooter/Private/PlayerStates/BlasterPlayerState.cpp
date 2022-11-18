// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStates/BlasterPlayerState.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"



void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);
}
//called on the server 
void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(Score + ScoreAmount);

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PC = PC == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PC;
		if (PC)
		{
			PC->SetHUDScore(Score);
		}
	}
	
}



void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ?Cast<ABlasterCharacter>(GetPawn()):Character;
	if (Character) 
	{
		PC = PC == nullptr ? Cast<ABlasterPlayerController>(Character->Controller): PC;
		if (PC) 
		{
			PC->SetHUDScore(Score);
		}
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PC = PC == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PC;
		if (PC)
		{
			PC->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PC = PC == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PC;
		if (PC)
		{
			PC->SetHUDDefeats(Defeats);
		}
	}

}

void ABlasterPlayerState::OnRep_Team()
{
	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());

	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());

	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}

}

