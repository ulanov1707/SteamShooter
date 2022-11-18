// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SteamShooter/Public/HUD/BlasterHUD.h"
#include "SteamShooter/Public/Weapon/WeaponTypes.h"
#include "SteamShooter/Public/Types/CombatState.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STEAMSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UCombatComponent();

	//now Blaster Charcater will have full access to all private/protected/public variables and functions of this component 
	friend class ABlasterCharacter;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	void EquipWeapon(class AWeapon*WeaponToEquip);
	void SwapWeapons();
	void FireButtonPressed(bool bPressed);
	void Fire();

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotGunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	void Reload();
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();//called every time when new bullet inserted into shotgun during reloading (called by AnimBP AnimNotify)
	void JumpToShotgunEnd();//jumps reload montage to Shotgun end section

	UFUNCTION(BlueprintCallable)
	void FinishedReloading();//we will cal this function in AnimNotify when reloading animation is finished

	UFUNCTION(BlueprintCallable)
	void FinishThrowingGranede();//we will cal this function in AnimNotify when throwing Granede animation is finished

	UFUNCTION(BlueprintCallable)
	void FinishSwap(); //called when Swap Weapon Anim Montage finished (call back for animnotify)

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachedWeapon(); //called when its right place of animation to swap a weapon (when right hand get close to Second weapon)

	UFUNCTION(BlueprintCallable)
	void LaunchGranede();//we will cal this function in AnimNotify 

	UFUNCTION(Server, Reliable)
	void Server_LaunchGranede(const FVector_NetQuantize& Target);

	FORCEINLINE int32 GetNades()const { return Nades; }

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);//called when picked up Ammo

	bool bLocallyReloading = false; //mostly to solve issue with fabrik delay (Predicting)


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void SetAimig(bool Aiming);

	UFUNCTION(Server,Unreliable)
	void Server_SetAimimg(bool Aiming);

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	int32 AmountToReload();

	void HandleReload();

	UFUNCTION()
	void OnRep_EquipedWeapon();

	UFUNCTION()
	void OnRep_SecondWeapon();

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& TraceHitTarget);

	//shotgun has a bit different fire
	UFUNCTION(Server, Reliable)
	void Server_ShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	//Does a linetrace from viewport crosshair 
	void TraceUnderCrosshair(FHitResult& TraceHitResult);


	void SetHUDCrosshairs(float DeltaTime);

	void ThrowGranede();

	UFUNCTION(Server, Reliable)
	void Server_ThrowGranede();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile>GranedeClass;

	void DropEquippedWeapon();//drops equipped weapon
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);//used when throwing a granede
	void AttachActorToBackPack(AActor* ActorToAttach);
	void AttachFlagToLeftHand(AWeapon* Flag);

	void UpdateCarriedAmmo(); // updates carried ammo value and HUD
	void PlayEquipWeaponSound(AWeapon * Weapon);
	void ReloadEmptyWeapon();
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondWeapon(AWeapon* WeaponToEquip);


private:

	UPROPERTY(ReplicatedUsing = OnRep_EquipedWeapon)
	class AWeapon* EquipedWeapon = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_SecondWeapon)
	class AWeapon* SecondWeapon = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;//Represents true local state of aimimg (used for Prediction)

	UFUNCTION()
	void OnRep_Aiming();

	class ABlasterCharacter* Character = nullptr;
	class ABlasterPlayerController* PlayerController = nullptr;
	class ABlasterHUD* HUD = nullptr;

	float BaseWalkSpeed,AimWakSpeed;
	
	bool bFireButtonPressed;

	/**
	*HUD and Crosshairs
	**/
		
	float CrosshairVelocityFactor;//Enlarges crosshair When moving 
	float CrosshairInAirFactor;//Enlarges crosshair When moving 
	float CrosshairAimFactor;//shrinks crosshair when aiming 
	float CrosshairShootFactor;

	 FHUDPackage HUDPackage;

	//location of line trace impact point , done from crosshair 
	FVector	ProjectileHitLocation;



	//Aiming and FOV
	
	//FOV When not Aiming;Set to the camera's base FOV in BeginPlay()
	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere,Category=Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	
	//handles Zooming
	void InterpFOV(float DeltaTime);



	//Automatic Fire
	FTimerHandle FireTimerHandle;

	bool bCanFire=true;
	bool CanFire();

	void StartFireTimer();
	void FireTimerFinished();

	//carried ammo for currently equipped weapon (ex:30 in the weapon and 120 carried with player)
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32>CarriedAmmoMap;//doesn't support replication

	int32 MaxCarriedAmmo = 500;

	UPROPERTY(EditAnywhere)
	int32 StartARAmmo = 30; // Assault Rifle ammo carried with player 

	UPROPERTY(EditAnywhere)
	int32 StartRLAmmo = 15; // Rocket Launcher ammo carried with player 


	UPROPERTY(EditAnywhere)
	int32 StartPistolAmmo = 50; // Pistol ammo carried with player 

	UPROPERTY(EditAnywhere)
	int32 StartFarmGunAmmo = 120; // Pistol ammo carried with player 

	UPROPERTY(EditAnywhere)
	int32 StartShotGunAmmo = 35; // Pistol ammo carried with player 

	UPROPERTY(EditAnywhere)
	int32 StartSniperAmmo = 30; // Pistol ammo carried with player 

	UPROPERTY(EditAnywhere)
	int32 StartGranedeLauncherAmmo = 30; // Pistol ammo carried with player 

	//intializes Carried ammo for each weapon type
	void InitCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	
	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmo();
	void UpdateShotGunAmmoValues();
	void ShowAttachedGranede(bool bShowMesh);

	UPROPERTY(ReplicatedUsing = OnRep_Nades)
	int32 Nades = 4;

	UFUNCTION()
	void OnRep_Nades();

	UPROPERTY(EditAnywhere)
	int32 MaxNades = 4;

	void UpdateHUDNades();

	//If we have 2 weapons and not overlapping any weapons we can swap 
	bool ShouldSwapWeapons();

	//used for Capture a flag GM
	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingFlag = false;

	UFUNCTION()
	void OnRep_HoldingTheFlag();

	UPROPERTY()
	AWeapon* TheFlag;

};
