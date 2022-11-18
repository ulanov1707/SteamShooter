// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BlasterCharacter.h"
#include"GameFramework/SpringArmComponent.h"
#include"Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include"Net/UnrealNetwork.h"
#include "SteamShooter/Public/Weapon/Weapon.h"
#include"SteamShooter/Public/Components/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SteamShooter/Public/Characters/BlasterAnimInstance.h"
#include "SteamShooter/SteamShooter.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"
#include "SteamShooter/Public/GameModes/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "SteamShooter/Public/PlayerStates/BlasterPlayerState.h"
#include "SteamShooter/Public/Weapon/WeaponTypes.h"
#include "SteamShooter/Public/Components/BuffComponent.h"
#include "Components/BoxComponent.h"
#include "SteamShooter/Public/Components/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SteamShooter/Public/GameStates/BlasterGameState.h"
#include "SteamShooter/Public/PlayerStart/TeamPlayerStart.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Spawns character no matter what ,will spawn even if there is a collision on spawn point , but on a bit different location
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CamerBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera= CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamer"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead Widget"));
	OverheadWidget->SetupAttachment(GetRootComponent());

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("MyCombatComponent")); 
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("MyBuffComponent"));
	BuffComponent->SetIsReplicated(true);

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 860.f);
	GetMovementComponent()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	
	NetUpdateFrequency = 66.f;//how often per sec character gets updates from server
	MinNetUpdateFrequency = 33.f;//how often per sec character gets updates from server  when replicated properties are changing infrequently
	
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Dissolve Timeline Component"));

	AttachedGranede = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Granede Mesh"));
	AttachedGranede->SetupAttachment(GetMesh(), FName("GranedeSocket"));
	AttachedGranede->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));



	/**
	* Hit boxes for server-side rewind 
	* Don't need collision since we only care for location,box extent(scale),rotation 
	* we will enable collision just for a momemnt for linetrace during server-side rewind, and turn off back
	*/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);


	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value) 
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}			
	}
}
void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent) 
	{
		CombatComponent->Character = this;
	}
	if (BuffComponent) 
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	
	}
	else 
		GEngine->AddOnScreenDebugMessage(-1, 22, FColor::Red, "Charcater::Buff not valid");

	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this;
		if (Controller)
		{
			LagCompensationComponent->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
	else 
		GEngine->AddOnScreenDebugMessage(-1, 22, FColor::Red, "Charcater::LagCompensation Component not valid");
}

//called  on clients when MovenentComponents gets updats from server
void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();

	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	UpdateHUDAmmo();

	UpdateHUDHealth();
	UpdateHUDShield();

	if (HasAuthority()) 
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::RecieveDamage);
	}
	if (AttachedGranede) 
	{
		AttachedGranede->SetVisibility(false);
	}
		
}
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotateInPlace(DeltaTime);
	HideCharacterIfCameraClose();
	PollInit();

}

void ABlasterCharacter::RotateInPlace(float DeltaTime) 
{

	if (CombatComponent && CombatComponent->bHoldingFlag)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (bDisableGamePlay) 
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		//if Authonomous Proxy , Or Server contolled by client 
		AimOffset(DeltaTime);
	}
	else
	{
		//Movement Gets replicated only when actor is moving
		//To update turning even if we are not moving we are manually calling OnRep_ReplicatedMovement() if time since last replicaiton reaches 0.5 secs
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	
}


//called When player state initialised
void ABlasterCharacter::OnPlayerStateInitialized()
{
	PlayerState->AddToScore(0.f);
	PlayerState->AddToDefeats(0);
	SetTeamColor(PlayerState->GetTeam());
	SetSpawnPoint(); // spawn you on a correct PlayerStart based on team if its team match

}

//finds a player start belonging to our team and moves player there
void ABlasterCharacter::SetSpawnPoint()
{
	//player has team (it is team match other)
	if (HasAuthority() && PlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;

		for (auto Start : PlayerStarts)//fill array with neccessary only our team's player starts
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == PlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
		}
	}

}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", EInputEvent::IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGranede", EInputEvent::IE_Pressed, this, &ABlasterCharacter::GranedeButtonPressed);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveFroward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappedWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGamePlay);
}


//Replicated - will be called on Server and also on Clients
void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	GM = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgressState = GM && GM->GetMatchState() != MatchState::InProgress;//are we playing game right now or not 

	//Cant destroy weapon when game is still on 
	if (CombatComponent && CombatComponent->EquipedWeapon && bMatchNotInProgressState)
	{
		CombatComponent->EquipedWeapon->Destroy();
	}
	

}
// initialise our HUD with correct values for score/defeats etc ..  when respawned or newly started playing
void ABlasterCharacter::PollInit()
{
	if (PlayerState == nullptr) 
	{
		PlayerState = GetPlayerState<ABlasterPlayerState>();
		if (PlayerState) 
		{
		
			OnPlayerStateInitialized();
			ABlasterGameState* GameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));

			if (GameState && GameState->TopScoringPlayers.Contains(PlayerState)) //if died and still Leader
			{
				Multicast_GainedTheLead();
			}
		}
	}
}
void ABlasterCharacter::MoveFroward(float value)
{
	if (bDisableGamePlay) return;
	if (Controller != nullptr && value != 0.f) 
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, value);
	}
}
void ABlasterCharacter::MoveRight(float value)
{
	if (bDisableGamePlay) return;
	if (Controller != nullptr && value != 0.f)
	{
		//GetControlRotation()- gives possesed charcater's aiming rotation (You can say cameras world rotation(to visualise better))
		//here we are taking Yaw of ControllRotaion - which is rotation on Z axis 
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//gives a FVector of Y Basis Vector (Direction of Y axis of character)
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, value);
	}
}
void ABlasterCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}
void ABlasterCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}
void ABlasterCharacter::Jump()
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (bDisableGamePlay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}
void ABlasterCharacter::CrouchButtonPressed()
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (bDisableGamePlay) return;
	if (!bIsCrouched && !GetCharacterMovement()->IsFalling())
	{
		Crouch();
	}
	else 
	{
		UnCrouch();
	}
}
void  ABlasterCharacter::ReloadButtonPressed() 
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (bDisableGamePlay) return;
	if (CombatComponent)
		CombatComponent->Reload();
}
void ABlasterCharacter::AimButtonPressed()
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (bDisableGamePlay) return;
	if (CombatComponent)
		CombatComponent->SetAimig(true);
	
}
void ABlasterCharacter::AimButtonReleased()
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (bDisableGamePlay) return;
	if (CombatComponent)
		CombatComponent->SetAimig(false);
}
float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}
void ABlasterCharacter::AimOffset(float DeltaTime)
{

	if (CombatComponent == nullptr || CombatComponent->EquipedWeapon == nullptr)
	{
		return;
	}
	
	if (GetCharacterMovement() == nullptr)
	{
		return;
	}
	


	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (CalculateSpeed() == 0.f && !bIsInAir) // Not moving and Not jumping 
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);

		
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning) 
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (CalculateSpeed() > 0 || bIsInAir)  // running or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}
void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90 && !IsLocallyControlled()) //Solves issue with UE compressions (When u sent FRotator/FVector frequently over network, values got compressed and negative values became possitive from 270 to 360)
	{
		//map pitch from [270,360) to [-90,0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquipedWeapon == nullptr) return;

	bRotateRootBone = false;

	if (CalculateSpeed() > 0.f) 
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation,ProxyRotationLastFrame).Yaw;

	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold) 
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else 
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}
void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f) 
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning) 
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw,0.f,DeltaTime,4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f) 
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGamePlay) return;
	if(CombatComponent) 
	{
		if (CombatComponent->bHoldingFlag == true)return;


		if(CombatComponent->CombatState == ECombatState::ECS_Unoccupied) // prevents spamming
			Server_EquipWeaponPressed();
		
		//immideatly play swap weapon animation on LocallyControled player so we don't wait untill Server Does it 
		//keep in mind we can't use swaped weapon untill server accepts this action , so to not experience disabled feeling untill we get anser from Server
		// at least we play SwapWeapon Animation , while we are waiting for Server Responce
		if (CombatComponent->ShouldSwapWeapons() && !HasAuthority() && OverlappedWeapon == nullptr &&CombatComponent->CombatState == ECombatState::ECS_Unoccupied/*Prevents spamming swaping */)
		{
			PlaySwapWeaponsMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwapingWeapons;
			bFinishedSwapping = false;
		}
	}



}
void ABlasterCharacter::Server_EquipWeaponPressed_Implementation()
{
	if (OverlappedWeapon)
	{
		CombatComponent->EquipWeapon(OverlappedWeapon);
	}
	else if(CombatComponent->ShouldSwapWeapons())//if don't overlapping weapon but still pressing Equip weapon (we swap weapons)
	{
		CombatComponent->SwapWeapons();
	}
}



//will be called on Client Only 
//This Repnotify is not get called for some reasons 
void ABlasterCharacter::OnRep_OverlappedWeapon(AWeapon* LastWeapon)
{

	if (OverlappedWeapon) 
	{
		OverlappedWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon) //will be called only when OverlappedWeapon will be null , then we will have value of last valid Overlapped Weapon
	{ 
		LastWeapon->ShowPickupWidget(false);
	}
	
}
void ABlasterCharacter::FireButtonPressed()
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (bDisableGamePlay) return;
	if (CombatComponent) 
	{
		CombatComponent->FireButtonPressed(true);
	}
	
}
void ABlasterCharacter::FireButtonReleased()
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (bDisableGamePlay) return;
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void ABlasterCharacter::GranedeButtonPressed()
{
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return;

	if (CombatComponent) 
	{
		CombatComponent->ThrowGranede();
	}
}

//callback when Character gets any damge applied(usually by projectiles) .Called only on the server
void ABlasterCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController/*The One who Damaged us*/, AActor* DamageCauser)
{
	
	GM = GM == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : GM;

	if (bEliminated && GM == nullptr) return;

	//Validates damage : if its team match and shooted team mate Damage will be 0
	Damage = GM->CalculateDamage(InstigatorController, Controller, Damage);

	float DamageToHealth = Damage;//how much Heath to take
	if (Shield > 0.f) 
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0, MaxShield);
			DamageToHealth = 0; // do dmaage to HEalth only to Shield
		}
		else 
		{
			DamageToHealth = FMath::Abs(Shield - Damage);
			Shield = 0.f;
		}

	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	
	
	//Will be called only on the server,clients will call it in  OnRep_Health()
	PlayHitReactMontage();
	UpdateHUDHealth();
	UpdateHUDShield();

	//Logic For Player Elimination
	
	if (GM) 
	{
		if (Health == 0.f) 
		{
			PC = PC == nullptr ? Cast<ABlasterPlayerController>(Controller) : PC;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			GM->PlayerEleminated(this,PC,AttackerController);
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	
	//Play it only when Health decreased(damaged) and not increased
	if (Health < LastHealth) 
	{
		PlayHitReactMontage();
	}
	
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();

	//Play it only when Shield decreased(damaged) and not increased
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	PC = PC == nullptr ? Cast<ABlasterPlayerController>(Controller) : PC;

	if (PC)
	{
		PC->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	PC = PC == nullptr ? Cast<ABlasterPlayerController>(Controller) : PC;

	if (PC)
	{
		PC->SetHUDShield(Shield, MaxShield);
	}
	
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	PC = PC == nullptr ? Cast<ABlasterPlayerController>(Controller) : PC;

	if (PC && CombatComponent && CombatComponent->EquipedWeapon)
	{
		PC->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		PC->SetHUDWeaponAmmo(CombatComponent->EquipedWeapon->GetAmmo());
	}
}


void ABlasterCharacter::SpawnDefaultWeapon()
{
	 GM = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

	UWorld* World = GetWorld();
	if (GM && World && !bEliminated && DefaultWeaponClass) 
	{
		AWeapon * StartWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (CombatComponent) 
		{
			CombatComponent->EquipWeapon(StartWeapon);
			CombatComponent->EquipedWeapon->bDestroyWeapon = true;
		}
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquipedWeapon == nullptr)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) 
	{
		//playing Shooting Anim MPntage
		AnimInstance->Montage_Play(FireWeaponMontage);
		//AM_FireWeapon has 2 sections aka parts(when aiming and not) we are setting here appropirate section name 
		FName SectionName = bAiming ?FName("AimRifle") :FName("RifleHip");
		//playing correct section of anim montage
		AnimInstance->Montage_JumpToSection(SectionName);
	}

}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquipedWeapon == nullptr)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		//playing Shooting Anim MPntage
		AnimInstance->Montage_Play(HitReactMontage);
		//AM_FireWeapon has 2 sections aka parts(when aiming and not) we are setting here appropirate section name 
		FName SectionName = FName("FromFront");
		//playing correct section of anim montage
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayEliminationMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EliminationMontage)
	{
		
		AnimInstance->Montage_Play(EliminationMontage);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquipedWeapon == nullptr)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (CombatComponent->EquipedWeapon->GetWeaponType())
		{

		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncer:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_FarmGun:
			SectionName = FName("Pistol"); //farm gun will play same reload anim as pistol
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("ShotGun");
			break;
		case EWeaponType::EWT_Sniper:
			SectionName = FName("Sniper");
			break;
		case EWeaponType::EWT_GranedeLauncher:
			SectionName = FName("GranedeLauncher");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayThrowGranedeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGranedeMontage)
	{
		AnimInstance->Montage_Play(ThrowGranedeMontage);
	}

}

void ABlasterCharacter::PlaySwapWeaponsMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapWeaponsMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponsMontage);
	}
}

FVector ABlasterCharacter::GetBulletHitLocation()const 
{ 
	if(CombatComponent)
		return CombatComponent->ProjectileHitLocation; 

	return FVector();

}
void ABlasterCharacter::SetWeapon(AWeapon* weapon)
{
	if (OverlappedWeapon) //if we are Server and Ended Overlapping 
	{
		OverlappedWeapon->ShowPickupWidget(false);
	}
	
	OverlappedWeapon = weapon;
	if (IsLocallyControlled()) //true - if this function called on the character that actually controlled by player
	{
		if (OverlappedWeapon)
		{
			OverlappedWeapon->ShowPickupWidget(true);
		}
	}
}
bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquipedWeapon);//return true if Combat and Equipped Weapon is valid
}
bool ABlasterCharacter::IsAiming() 
{
	return (CombatComponent && CombatComponent->bAiming);
}
 AWeapon* ABlasterCharacter::GetEquippedWeapon()
 { 
	 if(CombatComponent == nullptr)
	 {
		 return nullptr;
	 }
		

	 return CombatComponent->EquipedWeapon;
 }

//hide chacrer and weapon when you close to wall 
void ABlasterCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < HideCameraThreshold) 
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent->EquipedWeapon && CombatComponent->EquipedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquipedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//owner won't see the weapon mesh , other will see just fine
		}
		if (CombatComponent && CombatComponent->SecondWeapon && CombatComponent->SecondWeapon->GetWeaponMesh())
		{
			CombatComponent->SecondWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else 
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent->EquipedWeapon && CombatComponent->EquipedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquipedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//owner won't see the weapon mesh , other will see just fine
		}
		if (CombatComponent && CombatComponent->SecondWeapon && CombatComponent->SecondWeapon->GetWeaponMesh())
		{
			CombatComponent->SecondWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}

}

//Game Mode Calls it,thats why it will be always called on the server (since GM exists only on the server)
void ABlasterCharacter::Eliminate(bool bPlayerLeftGame)
{
	if (CombatComponent)  
	{
		//Drop Or Destroy weapon after Elimination

		if (CombatComponent->EquipedWeapon) 
		{
			DropDestroyWeapon(CombatComponent->EquipedWeapon);
			
		}
		if (CombatComponent->SecondWeapon) 
		{
			DropDestroyWeapon(CombatComponent->SecondWeapon);
		}
		if (CombatComponent->TheFlag)
		{
			CombatComponent->TheFlag->Dropped();
		}
			
	}
	Multicast_Eliminated(bPlayerLeftGame);
}

void ABlasterCharacter::DropDestroyWeapon(AWeapon *Weapon)
{
	if (Weapon == nullptr)return;

	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}
void ABlasterCharacter::Multicast_Eliminated_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if (PC) 
	{
		PC->SetHUDWeaponAmmo(0);
	}

	bEliminated = true;//since it will run on All Clients ,so all clients will change bEliminated variable so it doesnt't need to be replicated var
	PlayEliminationMontage();

	//Start dissolve effect on character
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow Intensity"), 200.f);
	}
	StartDissolve();

	//Disbale Chacrater movement
	
	bDisableGamePlay = true;
	GetCharacterMovement()->DisableMovement();//disables movement from W/S/A/D
    //GetCharacterMovement()->StopMovementImmediately();//diables rotation as well


	if (CombatComponent) 
	{
		CombatComponent->FireButtonPressed(false);
	}

	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGranede->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn ElimBot FX
	if (ElimBotEffect) 
	{
		FVector ElimBotSpawnPoint = FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
		if (ElimBotSound) 
		{
			UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, ElimBotSpawnPoint);
		}
	}
	if (IsLocallyControlled() && CombatComponent->bAiming && CombatComponent->EquipedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent) 
	{
		CrownComponent->DestroyComponent();
	}


	GetWorldTimerManager().SetTimer(ElimTimerHandle, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}



//only will be called on the server only
void ABlasterCharacter::ElimTimerFinished()
{
	
	 GM = GetWorld()->GetAuthGameMode<ABlasterGameMode>();

	if (bLeftGame == false) 
	{
		if (GM)
		{
			GM->RequestRespawn(this, this->Controller);
		}
	}
	else if(bLeftGame == true && IsLocallyControlled())
	{
		//if player left the game don't respawn him

		//inform that player left the game 
		OnLeftGameDelegate.Broadcast();
	}
	

}

void ABlasterCharacter::Server_LeaveGame_Implementation()
{
	 GM = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	PlayerState = PlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : PlayerState;
	if (GM)
	{
		//Hadles Eliminating a player 
		GM->PlayerLeftGame(PlayerState);
	}

}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{

	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindUFunction(this, FName("UpdateDissolveMaterial"));

	if (DissolveCurve && DissolveTimeline) 
	{
	
		//tells to timeline to use DissolveCurve and associate that curve with dissolve track which has callback function bound 
		//so when timeline plays It will start using curve and call our callback boud to our track
		DissolveTimeline->AddInterpFloat(DissolveCurve,DissolveTrack);
		//plays timeline
		DissolveTimeline->Play();
	}
}


//Sets appropirate colored material to a player based on which team he is in , also sets correct  Dissolve material
void ABlasterCharacter::SetTeamColor(ETeam Team)
{

	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial); // sets material to Chacter's Mesh
		DissolveMaterialInstance = BlueDissolveMatInst; // when we don't have a team we are using BlueTeams Dissolve material , we can set to any 
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	}

}

ETeam ABlasterCharacter::GetTeam()
{
	PlayerState = PlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : PlayerState;
	if (PlayerState == nullptr) return ETeam::ET_NoTeam;

	return PlayerState->GetTeam();
	
}

ECombatState ABlasterCharacter::GetCombatState()const
{
	if (CombatComponent)
		return CombatComponent->CombatState;
	else
		return ECombatState::ECS_DefaultMax;
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (CombatComponent == nullptr) return false;

	return CombatComponent->bLocallyReloading;

	
}



void ABlasterCharacter::Multicast_GainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;

	if (CrownComponent == nullptr) //first time spawning
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false);
	}

	if (CrownComponent) //if already spawned
	{
		CrownComponent->Activate(); //activates FX you can also Deactivate , but we will just destroy
	}
}
void ABlasterCharacter::Multicast_LostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

}


bool ABlasterCharacter::IsHoldingFlag() const
{
	if (!CombatComponent) return false;
	
	return CombatComponent->bHoldingFlag;
	
}


void ABlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if (CombatComponent == nullptr) return;
	CombatComponent->bHoldingFlag = bHolding;
}