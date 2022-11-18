// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include"OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UMenu::MenuSetup(int32 NumOfPublicConnections, FString TypeOfMatch,FString PathToLobby)
{
	PathToLobbyMap = FString::Printf(TEXT("%s?listen"),*PathToLobby);
	NumPublicConnections = NumOfPublicConnections;
	MatchType = TypeOfMatch;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World) 
	{
		APlayerController* PC = World->GetFirstPlayerController();

		if (PC) 
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputModeData);
			PC->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance;
	GameInstance = GetGameInstance();
	if (GameInstance) 
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	//Binding deldegates from Multiplayer Session Subsystem
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnCreateSessionComplete);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionCompleteDelegate.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsCompleteDelegate.AddUObject(this, &ThisClass::OnFindSessionsComplete);
		MultiplayerSessionsSubsystem->MultiplayerOnDestoySessionCompleteDelegate.AddDynamic(this, &ThisClass::OnDestroySessionComplete);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnStartSessionComplete);

	}

}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
		return false;

	if (Host_BTN) 
	{
		Host_BTN->OnClicked.AddDynamic(this,&ThisClass::HostButtonClicked);
	}

	if (Join_BTN)
	{
		Join_BTN->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	//has default values
	MenuSetup();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSessionComplete(bool bWasSuccessfull)
{
	if (bWasSuccessfull) 
	{

		
		UWorld* World = GetWorld();
		if (World)
		{
			if (!World->ServerTravel(PathToLobbyMap))
			{
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Couldnt travel to map ");
			}

			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, "Session Created");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Session NOT Created");
		Host_BTN->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessionsComplete(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccsessfull)
{
	if (MultiplayerSessionsSubsystem)
	{
		return;
	}


	for (const auto& Result : SearchResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);//gives value of "MatchType" variable in SessionSettings and sets value to SettingsValue variable
		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSessions(Result);
			return;
		}
	}

	if (!bWasSuccsessfull || SearchResults.Num() <= 0)//if  session not found or Search Result array for some reasons is empty
	{
		Join_BTN->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface) 
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);//traverls player to map 
			}
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success) // if we for some resons couldn't join session(ex:someone exited a session without destroying it)
	{
		Join_BTN->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySessionComplete(bool bWasSuccessfull)
{
}

void UMenu::OnStartSessionComplete(bool bWasSuccessfull)
{
}

void UMenu::HostButtonClicked()
{
	Host_BTN->SetIsEnabled(false);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Host Clicked");
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}


}

void UMenu::JoinButtonClicked()
{
	Join_BTN->SetIsEnabled(false);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Join Clicked");

	if (MultiplayerSessionsSubsystem) 
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}


}

void UMenu::MenuReset()
{
	RemoveFromParent();
	GetWorld()->GetFirstPlayerController()->SetInputMode(FInputModeGameOnly());
	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(false);

}


