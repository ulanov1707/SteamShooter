// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileGranede.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGranede::AProjectileGranede()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Granede Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Proj Move Comp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->SetIsReplicated(true);
	ProjectileMovementComp->bShouldBounce = true;


}

void AProjectileGranede::BeginPlay()
{
	//Dont need Projectile.h BeginPlay
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem();

	ProjectileMovementComp->OnProjectileBounce.AddDynamic(this,&AProjectileGranede::OnBounce);
}

void AProjectileGranede::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound) 
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	}
}

void AProjectileGranede::Destroyed() 
{

	ApplyExplodeDamage();
	Super::Destroyed();
}