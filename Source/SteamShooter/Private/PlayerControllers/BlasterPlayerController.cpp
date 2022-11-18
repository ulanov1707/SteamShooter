// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllers/BlasterPlayerController.h"
#include"SteamShooter/Public/HUD/BlasterHUD.h"
#include"SteamShooter/Public/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include"Components/TextBlock.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "SteamShooter/Public/GameModes/BlasterGameMode.h"
#include "SteamShooter/Public/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "SteamShooter/Public/Components/CombatComponent.h"
#include "SteamShooter/Public/GameStates/BlasterGameState.h"
#include "SteamShooter/Public/PlayerStates/BlasterPlayerState.h"
#include "Components/Image.h"
#include "SteamShooter/Public/HUD/ReturnMainMenu.h"
#include "SteamShooter/Public/Types/Announcement.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	Server_CheckMatchState();

	SetInputMode(FInputModeGameOnly());
}


void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;


	// Binding it here so it will work even if there is no character(ex: player died and waiting for respawn or Spectator Pawn)
	InputComponent->BindAction("Quit", IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);

}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidgetClass == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnMainMenu>(this, ReturnToMainMenuWidgetClass);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			//adds to viewport and does some setup
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			//Removes from viewport
			ReturnToMainMenu->MenuTearDown();
		}
	}

}



void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
}


void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
	
}

//Each 5 seconds will sync client time with server time
void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

//checks ping if its high Show high ping image and play animation
void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;

	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : PlayerState;

		if (PlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPing() * 4 : %d"), PlayerState->GetPing() * 4);
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				Server_ReportPingStatus(true); // Reports to the server that client has HIGH PING
			}
			else 
			{
				Server_ReportPingStatus(false); // Reports to the server that client NOT HIGH PING
			}
		}
		HighPingRunningTime = 0.f;
	}

	bool bHighPingAnimPlaying = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingAnimation
		&& BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimPlaying)
	{
		
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
			PingAnimationRunningTime = 0.f;
		}
	}
	
}

void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

//will be eun on owning player only "Autonomous proxy"
void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				BlasterHUD->AddElimAnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				BlasterHUD->AddElimAnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				BlasterHUD->AddElimAnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				BlasterHUD->AddElimAnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			BlasterHUD->AddElimAnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

//is the ping too high
void ABlasterPlayerController::Server_ReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}
void ABlasterPlayerController::PollInit()
{
		if (CharacterOverlay == nullptr)
		{
			if (BlasterHUD && BlasterHUD->CharacterOverlay)
			{
				CharacterOverlay = BlasterHUD->CharacterOverlay;
				if (CharacterOverlay)
				{
					if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
					if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
					
				
					if(bInitializeScore) SetHUDScore(HUDScore);
					if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);

					if (bInitialiseCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
					if (bInitialiseWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
					
					ABlasterCharacter * ContolledCharacter = Cast<ABlasterCharacter>(GetPawn());
					if (ContolledCharacter && ContolledCharacter->GetCombat())
					{
						if (bInitializeNades) SetHUDNades(ContolledCharacter->GetCombat()->GetNades());
					}
					
				}
			}
		}
	
	
}


void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ABlasterCharacter* BlasterPlayer = Cast<ABlasterCharacter>(InPawn);
	if (BlasterPlayer)
	{
		SetHUDHealth(BlasterPlayer->GetHealth(), BlasterPlayer->GetMaxHealth());
		
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController()) 
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

//Plays High ping Animation and Shows High ping image
void ABlasterPlayerController::HighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPing_IMG && BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPing_IMG->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation,0.f,4);
	}


}

//Stops High ping Animation and hides High ping image
void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPing_IMG && BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPing_IMG->SetOpacity(0.f);
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation)) 
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
		
	}

}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->Health_Text && BlasterHUD->CharacterOverlay->HealthBar;
	if (bHUDValid) 
	{
		//Progress bar's SetPercent takes value between 0-1
		const float HealthPercent = Health / MaxHealth;
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		BlasterHUD->CharacterOverlay->Health_Text->SetText(FText::FromString(HealthText));
	}
	else 
	{
		bInitializeHealth = true; // Yes ! we have to initalise CharacterOverlay values , when CharacterOverlay will be Valid
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->Shield_Text && BlasterHUD->CharacterOverlay->ShieldBar;
	if (bHUDValid)
	{
		//Progress bar's SetPercent takes value between 0-1
		const float ShieldPercent = Shield / MaxShield;
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		BlasterHUD->CharacterOverlay->Shield_Text->SetText(FText::FromString(ShieldText));

		
	}
	else
	{
		bInitializeShield = true; // Yes ! we have to initalise CharacterOverlay values , when CharacterOverlay will be Valid
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}


void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	//if this is client/server that is not locally controller , its BlasterHUD will be nullptr 
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ScoreAmount_Text;

	if (bHUDValid) 
	{
		FString ScoreString = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount_Text->SetText(FText::FromString(ScoreString));
	}
	else
	{
		bInitializeScore = true; 
		HUDScore = Score;
	}

}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	//if this is client/server that is not locally controller , its BlasterHUD will be nullptr 
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->DefeatsAmount_Text;
	if (bHUDValid) 
	{
		FString DefeatsString = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount_Text->SetText(FText::FromString(DefeatsString));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}

}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	//if this is client/server that is not locally controller , its BlasterHUD will be nullptr 
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->WeaponAmmoAmount_Text;
	if (bHUDValid)
	{
		FString AmmoString = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount_Text->SetText(FText::FromString(AmmoString));
	}
	else
	{
		bInitialiseWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	//if this is client/server that is not locally controller , its BlasterHUD will be nullptr 
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->CarriedAmmoAmount_Text;
	if (bHUDValid)
	{
		FString AmmoString = FString::Printf(TEXT("%d"), CarriedAmmo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount_Text->SetText(FText::FromString(AmmoString));
	}
	else
	{
		bInitialiseCarriedAmmo = true;
		HUDCarriedAmmo = CarriedAmmo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float Time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->MatchCountDown_Text;

	if (bHUDValid)
	{
		if (Time<=0.f) 
		{
			BlasterHUD->CharacterOverlay->MatchCountDown_Text->SetText(FText());
			return;
		}
		int32 Mins = FMath::FloorToInt(Time / 60);
		int32 Secs = Time - (Mins * 60);

		FString TimeString = FString::Printf(TEXT("%02d:%02d"), Mins, Secs);
		BlasterHUD->CharacterOverlay->MatchCountDown_Text->SetText(FText::FromString(TimeString));
	}
}

void ABlasterPlayerController::SetHUDAccouncementCountdown(float Time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->Announcement && BlasterHUD->Announcement->WarmupTime_Text;

	if (bHUDValid)
	{
		if (Time <= 0.f) 
		{
			BlasterHUD->Announcement->WarmupTime_Text->SetText(FText());
			return;
		}
		int32 Mins = FMath::FloorToInt(Time / 60);
		int32 Secs = Time - (Mins * 60);

		FString TimeString = FString::Printf(TEXT("%02d:%02d"), Mins, Secs);
		BlasterHUD->Announcement->WarmupTime_Text->SetText(FText::FromString(TimeString));
	}
}

void ABlasterPlayerController::SetHUDNades(int32 Nades)
{
	//if this is client/server that is not locally controller , its BlasterHUD will be nullptr 
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->Nade_Text;
	if (bHUDValid)
	{
		FString NadeString = FString::Printf(TEXT("%d"), Nades);
		BlasterHUD->CharacterOverlay->Nade_Text->SetText(FText::FromString(NadeString));
	}
	else
	{
		bInitializeNades = true;
		HUDNades = Nades;
	}

}

void ABlasterPlayerController::SetHUDTime() 
{

	float TimeLeft = 0.f;
																		
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CoolDownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	
	if (HasAuthority()) 
	{
		GM = GM == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : GM;
		if (GM) 
		{
			SecondsLeft = FMath::CeilToInt(GM->GetCountTime() + LevelStartingTime);
		}
	}
	


	//helps do not update Time HUD every frame but each second (since SecondsLeft and CountDownInt are both ints)
	//if SecondsLeft and CountDownInt are not equal means some time passed(1 sec ideally) so we got to update Time HUD 
	if (CountDownInt != SecondsLeft) 
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown) 
		{
			SetHUDAccouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountDownInt = SecondsLeft;
}

void ABlasterPlayerController::Server_RequestServerTime_Implementation(float TimeOfClientRequest)
{
	float CurrentServerTime = GetWorld()->GetTimeSeconds();
	Client_ReportServerTime(TimeOfClientRequest, CurrentServerTime);
}

void ABlasterPlayerController::Client_ReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedRequest)
{
	float RoundtripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;//time took to send info to the server and come back
	SingleTripTime = RoundtripTime / 2.f;

	//server time + time took to reach from server to client(not considering time took from client to server).Cheack out notes if something isn't clear
	//time to reach from client to server and come back from server to client are not exactly the same but , for our purpouse this small inaccuracy will work fine
	float CurrentServerTime = TimeServerRecievedRequest + (RoundtripTime / 2.f);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

//Time since Entire Server is running and  not a particular level 
float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) 
	{
		return GetWorld()->GetTimeSeconds();
	}
	else 
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
	
}

//called only on the server (GM calls it)
void ABlasterPlayerController::OnMatchStateSet(FName State,bool bTeamMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress) 
	{
		HandleMatchHasStarted(bTeamMatch);
	}
	else if (MatchState == MatchState::Cooldown) 
	{
		HandleCoolDownState();
	}

}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCoolDownState();
	}
	
}

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamMatch) 
{
	if(HasAuthority())
		bShowTeamScores = bTeamMatch;

	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay(); // only add CharacterOverlay into viewport only when MatchState is set to "InProgress" (in Warmups you want see CharacterOverlay widget)
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamMatch) 
		{
			InitTeamScores();
		}
		else 
		{
			HideTeamScores();
		}
	}
}

void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}


void ABlasterPlayerController::HandleCoolDownState()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		if (BlasterHUD->Announcement && BlasterHUD->Announcement->Announcement_Text && BlasterHUD->Announcement->Info_Text)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnounceText = Announcement::NewMatchStartsIn;
			BlasterHUD->Announcement->Announcement_Text->SetText(FText::FromString(AnnounceText));


			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			
			if (BlasterGameState && BlasterPlayerState) 
			{
				TArray<ABlasterPlayerState*>TopPlayers = BlasterGameState->TopScoringPlayers;

				//constructs correct winner string based if its team match or not
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayers);
				
				BlasterHUD->Announcement->Info_Text->SetText(FText::FromString(InfoTextString));
			}

		
		}
	}
	ABlasterCharacter* BlastCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlastCharacter && BlastCharacter->GetCombat())
	{
		BlastCharacter->bDisableGamePlay = true;
		BlastCharacter->GetCombat()->FireButtonPressed(false);
	}

}

//return correct info text for situation (if not team match)
FString ABlasterPlayerController::GetInfoText(const TArray<class ABlasterPlayerState*>& Players)
{
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == BlasterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

//retrun correct Info Text for situation if team match
FString ABlasterPlayerController::GetTeamsInfoText(ABlasterGameState* BlasterGameState)
{
	if (BlasterGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0) // both teams with 0 score
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)//multiple winner teams
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)/// Red Winner
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore) // blue winner
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}


void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->TXT_RedTeamScore &&
		BlasterHUD->CharacterOverlay->TXT_BlueTeamScore &&
		BlasterHUD->CharacterOverlay->TXT_ScoreSpacer;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->TXT_RedTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->TXT_BlueTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->TXT_ScoreSpacer->SetText(FText());
	}
}

void ABlasterPlayerController::InitTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->TXT_RedTeamScore &&
		BlasterHUD->CharacterOverlay->TXT_BlueTeamScore &&
		BlasterHUD->CharacterOverlay->TXT_ScoreSpacer;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		BlasterHUD->CharacterOverlay->TXT_RedTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->TXT_BlueTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->TXT_ScoreSpacer->SetText(FText::FromString(Spacer));
	}
}

//changes score of Red Team
void ABlasterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->TXT_RedTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlasterHUD->CharacterOverlay->TXT_RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}
//Changes score of blue team
void ABlasterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->TXT_BlueTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlasterHUD->CharacterOverlay->TXT_BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}


void ABlasterPlayerController::Server_CheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CoolDownTime = GameMode->CooldownTime;

		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
			BlasterHUD->AddAnouncement();

		Client_JoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime,CoolDownTime);
	}

}

void ABlasterPlayerController::Client_JoinMidGame_Implementation(FName StateOfMatch,float Warmup,float Match,float StartingLevel,float CoolDown)
{
	WarmupTime = Warmup;
	MatchState = StateOfMatch;
	LevelStartingTime = StartingLevel;
	MatchTime = Match;
	CoolDownTime = CoolDown;

	OnMatchStateSet(MatchState);

	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		BlasterHUD->AddAnouncement();
}


