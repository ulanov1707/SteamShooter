// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * Weapon wroking with LineTrace
 */
UCLASS()
class STEAMSHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget)override;

protected:

	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

private:

	UPROPERTY(EditAnywhere)
	 UParticleSystem* BeamParticle; // niagra smoke trail 

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlashParticle;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;






};
