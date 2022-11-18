// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"


//Our own delegates , other classes can bind their own functions to it (Designed for other classes to be bound and have updates on their side)
//using delagates cuz each of action (Destruction, joining etc) takes time and we are using delegates to know when it happend 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessfull);//sincse its dynamic delgate,paran type must be UClass or UStruct
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete,const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccsessfull);//since FOnlineSesionSearchResults isn't UCalss delegate is not dynamic
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestoySessionComplete, bool,bWasSuccessfull);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessfull);

//
//we are uisng UGameInstance Subsystem because we don't want to override some Engine classes like UGameInstance and also UGameInstance is designed for 
//other type of operations
//

//
//This class will handle all operations related to sessions (Creating/Joining/Finding etc...)
//
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionsSubsystem();

	//
	//To handle session functionality .NOT callbacks for delegates
	//
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSessions(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();


	//
	//Our custom delegates so our Menu widget can bind to it 
	//
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionCompleteDelegate;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsCompleteDelegate;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionCompleteDelegate;
	FMultiplayerOnDestoySessionComplete MultiplayerOnDestoySessionCompleteDelegate;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionCompleteDelegate;


	//Stores values  of MatchType & NumPublicCOnnections of created session
	int32 DesiredNumPublicConnections{};
	FString DesiredMatchType{};

protected:
	//
	//Internal callbacks for the delegates we'll add to Online Session Interface delegate list
	// Thise dont need to be called outside of this class 
	//
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	//
	//To add to Online Sessions Interface delegate list
	//we will bind our  Internal callback to this delegates
	//With this delegates we will know from  Online Session Interface : did we created session?Did we find session? and so on...
	//
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	//Delegate Handles - to remove our delegates from Online Sessions Interface's delegate list , as soon as we don't need them 
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };//do we have to create another session after destroying this sessions?
	int32 LastNumPublicConnections;
	FString LastMatchType;
	
};
