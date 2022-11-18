// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class STEAMSHOOTER_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UPROPERTY(EditAnywhere)
	float RotateRate = 20.f;

private:

	UPROPERTY(EditAnywhere)
		class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
		class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* PickupMesh;

	// Effect that will be attached into actor and will stay there/
	UPROPERTY(VisibleAnywhere)
		class UNiagaraComponent* PickupEffectComponent;

	// just a small FX that will popped up and destroyed when Health pickup picked up
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* DestroyPickupEffect;


	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();
	
public:
};
