// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState 
{
	extern STEAMSHOOTER_API const FName Cooldown;//match time has been reached .Display winner and begin cooldown timer
}

UCLASS()
class STEAMSHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABlasterGameMode();
	virtual void PlayerEleminated(class ABlasterCharacter* ElimedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackedPlayerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, class AController* ElimmedController);
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
	virtual void Tick(float DeltaTime)override;
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	//Displays time when BlasterGameMode is created
	float LevelStartingTime = 0.f;
	
	//
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	//used in Team Games which has overall team score, teammates can't shoot each other etc . Team oriented GMs sets it to true
	bool bTeamMatch = false;

protected:
	virtual void BeginPlay()override;
	virtual void OnMatchStateSet()override;
private:
	float CountTime = 0.f;

public:
	FORCEINLINE float GetCountTime()const { return CountTime; }
};
