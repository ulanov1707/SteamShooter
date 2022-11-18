// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SteamShooter/Public/Types/TurningInPlace.h"
#include "SteamShooter/Public/Types/Teams.h"
#include "SteamShooter/Public/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "SteamShooter/Public/Types/CombatState.h"
#include "BlasterCharacter.generated.h"

//When player left game
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class STEAMSHOOTER_API ABlasterCharacter : public ACharacter,public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:

	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void PostInitializeComponents()override;
	virtual void OnRep_ReplicatedMovement()override;
	virtual void Destroyed()override;
	
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_Eliminated(bool bPlayerLeftGame);//when player gets eliminated 
	void Eliminate(bool bPlayerLeftGame);

	

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	void UpdateHUDHealth();//updates Health info of the HUD
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void SpawnDefaultWeapon();


	//contains all HitBoxes of this character , by saving them all in one container we can run Rangebased-for loop ann access each hitbox , without manually accessing each hitbox by name
	UPROPERTY()
	TMap<FName,class UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = false;


	//Tells the GM to leave this player from game
	UFUNCTION(Server, Reliable)
		void Server_LeaveGame();

	//when player left game delegate ( UReturnMainMenu will bind to it)
	FOnLeftGame OnLeftGameDelegate;


	//Handles spawning Crown FX on leading player
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_GainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_LostTheLead();

	//sets team for this player
	void SetTeamColor(ETeam Team);

	ETeam GetTeam();

protected:

	virtual void BeginPlay() override;
	void MoveFroward(float value);
	void MoveRight(float value);
	void LookUp(float value);
	void Turn(float value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();//Charcater on Simulated Proxies gets shaking effect , this function  fixes it
	virtual void Jump()override;
	void FireButtonPressed();
	void FireButtonReleased();
	void GranedeButtonPressed();
	void DropDestroyWeapon(AWeapon* Weapon);

	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage,const UDamageType *DamageType,class AController*InstigatorController,AActor* DamageCauser);

	UFUNCTION(Server,reliable)
	void Server_EquipWeaponPressed();

	//Poll for any relevant classes and initialise our HUD
	void PollInit();
	void RotateInPlace(float DeltaTime);

	void SetSpawnPoint();// sets 
	void OnPlayerStateInitialized();


	/**
	* Hit boxes used for server-side rewind
	*/

	UPROPERTY(EditAnywhere)
		class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
		UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
		UBoxComponent* blanket; // second part of backpack

	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_r;

	

private:
	UPROPERTY(VisibleAnywhere,Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappedWeapon)
	class AWeapon* OverlappedWeapon;//Repnotify

	UFUNCTION()
	void OnRep_OverlappedWeapon(AWeapon * LastWeapon);//called on Client Only

	/// 
	/// Character's components
	/// 
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComponent;

	
	//So the logic here is that we shoot locally and we have to check wether we hit or not? 
	//there are always delay between client and server so Server versions of each character will store it own information at particular time
	//The information stored is loaction/rotation/boxExtent of each hitbox at particular time 
	//so when client shoots someone to check a hit , we sent time when we shoot,linetrace info of shot and character that we hit locally to the Server
	//then based on this info Server goes through saved inforation ,finds where at that time Hited Chracter was
	//changes hitboxes of Hitted character to position it was at that time, does lintrace and tells wether we hit or not
	//and then brings back Hitboxes to its place 

	//Method used for lag compensation called Server-Side Rewind 
	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensationComponent;


	float AO_Yaw,AO_Pitch;
	float InterpAO_Yaw;

	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	//Animation Montages

	UPROPERTY(EditAnywhere,Category=Combat)
	class UAnimMontage* FireWeaponMontage;//Anim Montage value will be set in BP

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;//Anim Montage value will be set in BP

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* EliminationMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGranedeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapWeaponsMontage;


	//hide character and weapon when you close to wall 
	void HideCharacterIfCameraClose();

	UPROPERTY(EditAnywhere, Category = Camera)
	float HideCameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;//current rotation
	float ProxyYaw;

	float TimeSinceLastMovementReplication;
	float CalculateSpeed();


	//Player Health
	// 
	//usually player related variables must be inside of Player State Class but we are handling Health here because 
	//we want to see health change as fast as possible , however Player State has slower net update than Pawn 
	//so Health is handled inside of Pawn in this case
	UPROPERTY(EditAnywhere,Category = "Player Stats")
	float MaxHealth= 100.f;

	UPROPERTY(ReplicatedUsing =OnRep_Health,VisibleAnywhere,Category = "Player Stats")
	float Health=100.f;//current hp

	UFUNCTION()
	void OnRep_Health(float LastHealth);


	//Player Shield 
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")// if you want on Start you can have 100 shield and pick it up
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);


	class ABlasterPlayerController* PC = nullptr;
	class ABlasterGameMode* GM = nullptr;

	//Player Elimination

	bool bEliminated = false;

	FTimerHandle ElimTimerHandle;
	void ElimTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	bool bLeftGame = false;


	//Dissolve Effect 

	
	FOnTimelineFloat DissolveTrack;//Delegeate called when timeline updated(we bind a function to it)

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;//cpp version of timeline
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere,Category = Eliminated)
	UMaterialInstanceDynamic *DynamicDissolveMaterialInstance;//Dynamic Instace that we can change at run time


	UPROPERTY(VisibleAnywhere,Category = Eliminated)
	//Material Instance for dissolvig ,used with dynamic material instacne (Internal)
	UMaterialInstance* DissolveMaterialInstance = nullptr;

	/**
	* Team colors
	*/

	UPROPERTY(EditAnywhere, Category = Eliminated)
		UMaterialInstance* RedDissolveMatInst;   // dissolve material for Red Team players

	UPROPERTY(EditAnywhere, Category = Eliminated)
		UMaterialInstance* RedMaterial;			// Charcter Mesh's material for Red Team

	UPROPERTY(EditAnywhere, Category = Eliminated)
		UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Eliminated)
		UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Eliminated)
		UMaterialInstance* OriginalMaterial; // Original Material of the Character's mesh (Not Dissolve Material)



	//Elim Bot 
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;//when FX spawned on runtime we will store spawned FX here

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem; // Leading Player will have cwond FX on head

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;



	//Granded 

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGranede;

	//Default Weapon
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;


public:	

	void SetWeapon(AWeapon* weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon* GetEquippedWeapon();

	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch()const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace()const { return TurningInPlace; }
	FVector GetBulletHitLocation()const;
	FORCEINLINE UCameraComponent* GetFollowCamera()const { return FollowCamera; }
	FORCEINLINE UAnimMontage* GetReloadMontage()const { return ReloadMontage; }


	//Play Montages
	void PlayFireMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayEliminationMontage();
	void PlayReloadMontage();
	void PlayThrowGranedeMontage();
	void PlaySwapWeaponsMontage();

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated()const { return bEliminated; }
	FORCEINLINE float GetHealth()const { return Health; }
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	FORCEINLINE float GetShield()const { return Shield; }
	FORCEINLINE float GetMaxShield()const { return MaxShield; }
	FORCEINLINE UStaticMeshComponent* GetGranedeMesh()const { return AttachedGranede; }

	class ABlasterPlayerState* PlayerState = nullptr;

	 ECombatState GetCombatState()const;
		
	 UPROPERTY(Replicated)
	 bool bDisableGamePlay = false;//disables some inputs not all 

	 FORCEINLINE UCombatComponent* GetCombat() const{ return CombatComponent; }
	 FORCEINLINE bool GetDisableGameplay()const { return bDisableGamePlay; }
	 FORCEINLINE UBuffComponent* GetBuff()const { return BuffComponent; }
	 FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	 FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	 bool IsLocallyReloading();

	 FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensationComponent; }

	 FORCEINLINE bool IsHoldingFlag()const;

	 void SetHoldingTheFlag(bool bHolding);
	
};
