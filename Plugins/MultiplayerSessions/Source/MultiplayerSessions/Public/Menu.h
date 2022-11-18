// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 NumOfPublicConnections = 4, FString TypeOfMatch = FString("FreeForAll"), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby?listen")) );

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)override;//gets called when we switch to another level 
	
	//
	//Callbacks for custom delegates on the Multiplayer Sessions Subsystem
	//
	UFUNCTION()
	void OnCreateSessionComplete(bool bWasSuccessfull);

	//thise are not UFUNCTIONS because delgeates that will be binded are not dynamic
	void OnFindSessionsComplete(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccsessfull);
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySessionComplete(bool bWasSuccessfull);
	UFUNCTION()
	void OnStartSessionComplete(bool bWasSuccessfull);
	
private:
	//this macro binds cpp button to blueprint button 
	//IMPORTANT: name of variable in cpp MUST be exact same as it is in blueprint widget
	UPROPERTY(meta=(BindWidget))
	class UButton* Host_BTN;

	UPROPERTY(meta = (BindWidget))
	class UButton* Join_BTN;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	void MenuReset();

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{4};

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{FString("DeathMatch")};

	FString PathToLobbyMap{ TEXT("") };

	
	
};
