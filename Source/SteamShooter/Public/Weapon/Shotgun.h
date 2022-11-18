// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/HitScanWeapon.h"
#include "Shotgun.generated.h"


UCLASS()
class STEAMSHOOTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
	
public:
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	void ShotgunTraceEndWithSpread(const FVector& HitTarget ,TArray<FVector_NetQuantize>& HitTargets);
	
private:

	UPROPERTY(EditDefaultsOnly,Category = "Spread")
	int32 NumOfShots = 10;
};
