// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

private:
	//we will spawn 2 types of Projectile : Replicated and Not Replicated(will use ServerSideRewind)
	

	UPROPERTY(EditAnywhere,Category = "Weapon Properties")
	TSubclassOf<class AProjectile> ProjectileClass;

	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;

public:
	virtual void Fire(const FVector& HitTarget)override;
	

};
