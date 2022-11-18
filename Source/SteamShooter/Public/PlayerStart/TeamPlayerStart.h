// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "../Types/Teams.h"
#include "../Types/Teams.h"
#include "TeamPlayerStart.generated.h"

/**
 * Player start based on team , each player can spawned only on its own team player start
 */
UCLASS()
class STEAMSHOOTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
	
};
