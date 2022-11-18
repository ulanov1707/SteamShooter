// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileGranede.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API AProjectileGranede : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileGranede();
	virtual void Destroyed()override;
protected:
	virtual void BeginPlay()override;
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);


private:
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
