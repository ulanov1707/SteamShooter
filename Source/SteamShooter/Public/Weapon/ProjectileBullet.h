// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * Projectile that can apply damage 
 */
UCLASS()
class STEAMSHOOTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileBullet();

//only compiled when on editor , won't exist during gameplay
#if WITH_EDITOR

	//Called whenever variable with UPROPERTY() changed
	//solves issue when blueprint constructor overrides c++ constructor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif


protected:
	 virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)override;

	 virtual void BeginPlay() override;
};
