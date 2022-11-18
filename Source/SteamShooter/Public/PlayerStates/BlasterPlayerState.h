// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SteamShooter/Public/Types/Teams.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	void AddToScore(float ScoreAmount);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void OnRep_Score()override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToDefeats(int32 DefeatsAmount);

private:
	class ABlasterCharacter* Character = nullptr;
	class ABlasterPlayerController* PC = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam; // which team this player belongs

	UFUNCTION()
	void OnRep_Team();

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
	
};
