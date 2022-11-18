// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API UReturnMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void MenuSetup(); // initailises and adds to viewport
	void MenuTearDown(); // destroy widget

protected:
	virtual bool Initialize() override; // similar to BeginPlay()

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* BTN_Return;

	UFUNCTION()
		void ReturnButtonClicked();

	UPROPERTY()
		class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY()
		class APlayerController* PlayerController;
};
