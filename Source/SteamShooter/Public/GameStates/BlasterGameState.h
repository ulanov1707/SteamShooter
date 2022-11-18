// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
		virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

		UPROPERTY(Replicated)
		TArray<class ABlasterPlayerState*>TopScoringPlayers;
	
		void UpdateTopScore( ABlasterPlayerState* ScoringPlayer);

		/**
		* Teams
		*/

		//called when each team scored 
		void RedTeamScored();
		void BlueTeamScored();

		//list of player on each team

		TArray<ABlasterPlayerState*> RedTeam;
		TArray<ABlasterPlayerState*> BlueTeam;

		UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
			float RedTeamScore = 0.f;

		//overall score of each team

		UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
			float BlueTeamScore = 0.f;

		UFUNCTION()
			void OnRep_RedTeamScore();

		UFUNCTION()
			void OnRep_BlueTeamScore();

private:
		float TopScore = 0.f;


		
};
