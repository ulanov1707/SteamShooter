// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()

public:
	

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:

	UPROPERTY(EditAnywhere)
		float ShieldAmount = 100.f;

	//player won't be healed immideatly full health , it takes some time 
	UPROPERTY(EditAnywhere)
		float ShieldTime = 5.f;

	
};
