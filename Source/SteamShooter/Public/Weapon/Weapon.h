// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SteamShooter/Public/Weapon/WeaponTypes.h"
#include "../Types/Teams.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState :uint8 
{

	EWS_InitialState UMETA(DisplayName ="Initial State"),
	EWS_Equipped UMETA(DisplayName="Equipped"),
	EWS_EquippedSecond UMETA(DisplayName = "EquippedSecond"),
	EWS_Dropped UMETA(DisplayName="Dropped"),
	

	EWS_MAX UMETA(DisplayName = "DefaultMax")// This is unreal convention , helps to know how many max elements in the enum 
};


//type of weapon
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class STEAMSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Fire(const FVector&HitTarget);
	virtual void Dropped();
	virtual void OnRep_Owner()override;
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);
	FVector TraceEndWithSpread(const FVector& HitTarget);

	//Enable/Disable custom depth (outline of weapon)
	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon=false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecond();

	
	//Trace End with spread 

	// this is not a distance of linetrace, this is distance where the sphere will be spanwed , actual lintrace length is way more big
	UPROPERTY(EditAnywhere, Category = "Spread")
		float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Spread")
		float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		float HeadShotDamage = 60.f;

	UPROPERTY(Replicated,EditAnywhere)
	bool bUseServerSideRewind = false;

	class ABlasterCharacter* WeaponOwnerCharacter = nullptr;
	class ABlasterPlayerController* WeaponOwnerController = nullptr;

	UFUNCTION()
	void OnPingTooHigh(bool bIsPingTooHigh);
private:
	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickUpWidget;

	UPROPERTY(EditAnywhere,Category = WeaponProperties)
	class UAnimationAsset* FireAnimation;//value will be set in BP

	UPROPERTY(EditAnywhere, Category = WeaponProperties)
	TSubclassOf<class ABulletCasing> CasingClass;
	

	//Zoomed FOV while aimig
	//Each Weapon will have its own FOV
	
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	//Ammo 
	UPROPERTY(EditAnywhere)
	int32 Ammo;//current ammo in the magazine

	UFUNCTION(Client,Reliable)
	void Client_UpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void Client_AddAmmo(int32 AmmoToAdd);

	void SpendBullet();

	//the number of unproccessed server requests for Ammo 
	//incremented in SpendBullet(), Decremented in Client_UpdateAmmo()
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;//max ammo for magazine

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	ETeam Team;
	

public:
	void ShowPickupWidget(bool bShowWidget);
	void SetWeaponState(EWeaponState NewState);
	FORCEINLINE USphereComponent* GetAreaSphere() const{ return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV()const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed()const { return ZoomInterpSpeed; }
	FORCEINLINE bool IsEmpty() { return Ammo <= 0; }
	FORCEINLINE bool IsFull() { return Ammo == MagCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const{ return Ammo ; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage()const { return Damage; }
	FORCEINLINE float GetHeadShotDamage()const { return HeadShotDamage; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickUpWidget; }
	FORCEINLINE ETeam GetTeam() const { return Team; }

	/*
**Textures for the weapon Crosshairs
*/
	UPROPERTY(EditAnywhere, Category = Chrosshair)
		class UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = Chrosshair)
		UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = Chrosshair)
		UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, Category = Chrosshair)
		UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = Chrosshair)
		UTexture2D* CrosshairBottom;

	//Automatc Fire

	UPROPERTY(EditAnywhere, Category = Combat)
		float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = Combat)
		bool bIsAutomatic = true;//is Weapon Automatic 

	UPROPERTY(EditAnywhere, Category = Combat)
		class USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, Category = "Spread")
		bool bUseSpread = false;


};
