// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Health_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_RedTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_ScoreSpacer;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount_Text;

	UPROPERTY(meta = (BindWidget))
	 UTextBlock* CarriedAmmoAmount_Text;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountDown_Text;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Nade_Text;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	 UTextBlock* Shield_Text;


	UPROPERTY(meta = (BindWidget))
	class UImage* HighPing_IMG;

	UPROPERTY(meta = (BindWidgetAnim),Transient)
	UWidgetAnimation* HighPingAnimation;


};
