// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:

	ATeamsGameMode();
	//assigns Teams when player LoggedIn (Handles when player joined during already playing match)
	/** Called after new player successfully logged in.  This is the first place it is safe to call replicated functions on the PlayerController. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	//removes left player form team 
	//Logout - called when any player left the match 
	virtual void Logout(AController* Exiting) override;

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)override;
	void PlayerEleminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)override;

protected:
	//assigns Teams whecn match has Started
	//called when MathcState changed to MatchHasStarted , used to handle staff when Matach State changed to MatchHasStarted
	virtual void HandleMatchHasStarted() override;
	
};
