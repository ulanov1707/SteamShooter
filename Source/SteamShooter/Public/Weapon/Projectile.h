// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class STEAMSHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Tick(float DeltaTime) override;


	/**
	* Used with server-side rewind
	*/

	bool bUseServerSideRewind = false;

	//FVector_NetQuantize doesnt have decimal points
	FVector_NetQuantize TraceStart;

	//FVector_NetQuantize100 contains .2 decimal points ex: 15.32

	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
		float InitialSpeed = 15000;

	

	//Only used for Grenades and Rockets.Bullets will use Weapons "Damage" value
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	// Rockets launcher and grenade launchers  won't use it (no point of having head shots for therm)
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticel;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComp;


	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;

	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	void StartDestroyTimer();
	void DestroyTimerFinished();

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ProjectileMesh; //Childs that need Projectile Mesh has to Construct it by themselfs (CreadtDefaultSubObject)


	//Damage 


	void ApplyExplodeDamage();

	//Radiuses used for Radial Damage

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;


public:	
	virtual void Destroyed()override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;





};
