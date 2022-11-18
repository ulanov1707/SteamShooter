// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//contains Location/Rotation/BoxExtent of individual hitbox(bone)
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};

//contains info(location/rotation/boxextent) of each hitbox at particular time
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

		UPROPERTY()
		float Time;

		//TMap<NameofBone(since FName is cheaper than BoxComponent),InfoOfThatHitBox(location/rotation/boxextent) >
		UPROPERTY()
		TMap<FName, FBoxInformation> HitBoxInfo;

		UPROPERTY()
		ABlasterCharacter* Character;//Character who's History we are checking(used only for shotgun)
};

//tells result of Rewind(hit check) - wether we hit or not ,and was it headshot if we hitted
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

//tells result of Rewind(hitCheck) - Which character and how many times hitted and headshoted
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

		UPROPERTY()
		TMap<ABlasterCharacter*, uint32>HeadShots; //TMap<HittedCharacterToHead,HowManyTimeHittedToHead>

		UPROPERTY()
		TMap<ABlasterCharacter*, uint32>BodyShots;
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STEAMSHOOTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULagCompensationComponent();
	friend class ABlasterCharacter;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	//SSR for HitScan Weapons
	FServerSideRewindResult ServerSideRewind(
		class ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime);

	//called on clinet by player to check wether we hit or not . if hit apply damage(hitscan weapon)
	UFUNCTION(Server, Reliable)
	void ServerCheckHitWithRewind(
			ABlasterCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation,
			float HitTime
		);


	//SSR for Shothun
	//same as ServerSideRewind but for shothun , since hsotgun can shot 2-3 players at one shot
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	

	UFUNCTION(Server, Reliable)
	void ServerShotgunCheckHitWithRewind(
			const TArray<ABlasterCharacter*>& HitCharacters,
			const FVector_NetQuantize& TraceStart,
			const TArray<FVector_NetQuantize>& HitLocations,
			float HitTime
		);

	//SSR for Projectile Weapon
	FServerSideRewindResult ProjectileServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);



	UFUNCTION(Server, Reliable)
		void ServerProjectileCheckHitWithRewind(
			ABlasterCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize100& InitialVelocity,
			float HitTime
		);

	
protected:

	virtual void BeginPlay() override;
	//fills up "Saved Info" of each bone at certain time 
	void SaveFramePackage(FFramePackage& Package);

	//Since we almost never get exact hit time in our records and the value we need will be between 2 records
	// finds and returns FramePackage(record of loc/rot/scale each hitbox) at exact hit time by interpolating between 2 records
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutPackage);
	
	//moves hitcharcater's hitboxes					//		where to move
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);

	//moves back hitboxes and disables collision
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);

	//Enable/Disable Hit Character's MESH's collision
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	void HandleSavingFramePackage();

	//gives correct FFramePackage to check for Character Hit  
	FFramePackage GetFrameToCheck(class ABlasterCharacter* HitCharacter, float HitTime);


	//check wether we hit or not (Hitscan Weapon)
	FServerSideRewindResult ConfirmHit(const FFramePackage& PackageToCheck, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	//
	//Shotgun
	
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages,const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocations);


	//Projectile Weapon
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package,ABlasterCharacter* HitCharacter,const FVector_NetQuantize& TraceStart,const FVector_NetQuantize100& InitialVelocity,float HitTime);
private:

	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	//Sorest info of each hitbox at particular time, where THIS charaacter was at particular time 
	// And other's when they hit us in their local machine based on this will check wether they really hit us On Server
	TDoubleLinkedList<FFramePackage>FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f; // max amount of time to store Hitboxinfo after that info older than this time gets removed

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
