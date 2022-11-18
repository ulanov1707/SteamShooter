// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ReturnMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"

void UReturnMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if (BTN_Return && !BTN_Return->OnClicked.IsBound())
	{
		BTN_Return->OnClicked.AddDynamic(this, &UReturnMainMenu::ReturnButtonClicked);
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		// we did our multiplayer subsystem child of UGameInstanceSubsystem
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerOnDestoySessionCompleteDelegate.IsBound())
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestoySessionCompleteDelegate.AddDynamic(this, &UReturnMainMenu::OnDestroySession);
		}
	}
}

bool UReturnMainMenu::Initialize()
{
	if (!Super::Initialize())// if couldn't initialise
	{
		return false;
	}

	return true;
}

//called after session destroyed 
// Travels you to main menu 
void UReturnMainMenu::OnDestroySession(bool bWasSuccessful)
{
	// if couldn't destroy Session enable button back 
	if (!bWasSuccessful)
	{
		BTN_Return->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode) // if server 
		{
			GameMode->ReturnToMainMenuHost(); // travels server and others to the Main Menu
		}
		else // if client 
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText()); // travels client to main menu 
			}
		}
	}
}

void UReturnMainMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	//unbidnig delegates
	if (BTN_Return && BTN_Return->OnClicked.IsBound())
	{
		BTN_Return->OnClicked.RemoveDynamic(this, &UReturnMainMenu::ReturnButtonClicked);
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestoySessionCompleteDelegate.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestoySessionCompleteDelegate.RemoveDynamic(this, &UReturnMainMenu::OnDestroySession);
	}
}

void UReturnMainMenu::ReturnButtonClicked()
{
	//Disable spamming
	BTN_Return->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
	
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if (BlasterCharacter)
			{
				// we can't leave the game immideatly , death animation/effects/sounds must be played and we got to be removed from TopScoring players list
				BlasterCharacter->Server_LeaveGame();
				BlasterCharacter->OnLeftGameDelegate.AddDynamic(this, &UReturnMainMenu::OnPlayerLeftGame);
			}
			else // waiting to respawn state
			{
				BTN_Return->SetIsEnabled(true);
			}
		}
	}
	
}
//after "death animation/effects/sounds must be played and we got to be removed from TopScoring players list" done can actually be removed from session
void UReturnMainMenu::OnPlayerLeftGame()
{
	//destroys session
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}