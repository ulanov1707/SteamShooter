// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);


/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay()override;
	virtual void SetupInputComponent() override;

	void SetHUDTime();
	virtual void Tick(float DeltaTime)override;

	//
	//Syncing Server time with Client time
	//

	//Request the current server time , passing in client's time when request was sent
	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(float TimeOfClientRequest);

	//Reports current Server time to the client , with client's time when request was sent
	UFUNCTION(Client, Reliable)
	void Client_ReportServerTime(float TimeOfClientRequest,float TimeServerRecievedRequest);

	float ClientServerDelta = 0.f;//difference between Client and Server time(Delay)
	UPROPERTY(EditAnywhere,Category = Time)
	float TimeSyncFrequency = 5.f;//every 5 seconds we will correct our time with Server Time  
	float TimeSyncRunningTime = 0;//how much time passed since last  server time sync
	void CheckTimeSync(float DeltaTime);
	void PollInit();
	UFUNCTION(Server,Reliable)
	void Server_CheckMatchState();


	UFUNCTION(Client, Reliable)
	void Client_JoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingLevel, float CoolDown);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);
	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<class ABlasterPlayerState*>& Players);
	FString GetTeamsInfoText(class ABlasterGameState* BlasterGameState);
public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);

	class ABlasterHUD* BlasterHUD = nullptr;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);//updates hud for current ammo on the magazine
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDMatchCountdown(float Time);
	void SetHUDAccouncementCountdown(float Time); // Warmup timer
	void SetHUDNades(int32 Nades); // Warmup timer
	virtual void OnPossess(APawn* InPawn)override;
	virtual float GetServerTime();//Returns current time on the Server
	virtual void ReceivedPlayer()override;// Helps to sync with server clock as soon as possible
	void OnMatchStateSet(FName State,bool bTeamMatch = false);
	void HandleMatchHasStarted(bool bTeamMatch = false);
	void HandleCoolDownState();

	//Time taken info to reach from Server to Client (half of rount trip time)
	float SingleTripTime = 0;


	//This Delegate tells everyone bound does client has HIGH PING or NOT
	//when we have a high ping , our weapons should Disable Server Side Rewind
	//our weapons will bind to this delegate and whenever client will have high ping weapons will got alarmed and they stop using Server Side Rewind 
	FHighPingDelegate HighPingDelegate;

	
	void ShowReturnToMainMenu();

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
private:
	float MatchTime;//120 secs will 1 match take
	int32 CountDownInt = 0;//how many seconds left for game/round end
	float WarmupTime;
	float LevelStartingTime;
	float CoolDownTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	class UCharacterOverlay* CharacterOverlay = nullptr;
	

	//Character overlay can be sometimes not valid , and we are caching this value for Character Overlay so when it becomes valid we will set them
	float HUDHealth, HUDMaxHealth;
	bool bInitializeHealth = false;

	float HUDScore;
	bool bInitializeScore = false;

	float HUDShield, HUDMaxShield;
	bool bInitializeShield = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDNades;
	bool bInitializeNades = false;

	float HUDCarriedAmmo;
	bool bInitialiseCarriedAmmo = false;

	float HUDWeaponAmmo;
	bool bInitialiseWeaponAmmo = false;

	class ABlasterGameMode* GM;

	float HighPingRunningTime = 0.f; //Time Since laste check of high ping 
	float PingAnimationRunningTime = 0.f; // 

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;   // how many seconds to show highping animation ?

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f; // how often to check ping 

	UFUNCTION(Server, Reliable)
	void Server_ReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 150.f; 

	/**
	* Return to main menu
	*/

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidgetClass; // WBP Class to spawn 

	UPROPERTY()
	class UReturnMainMenu* ReturnToMainMenu; // Spawned widget

	//show or hide widget
	bool bReturnToMainMenuOpen = false;


};
