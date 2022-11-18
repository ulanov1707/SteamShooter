// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SteamShooter/Public/Components/LagCompensationComponent.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Proj Move Comp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->SetIsReplicated(true);
}



void AProjectileBullet::BeginPlay()
{

	Super::BeginPlay();
	/*
	
	//Something Similar to Spawn Params
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true; // use Trace Channel 
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility; //which trace channel to use
	PathParams.bTraceWithCollision = true; // generate HitEvents , if false it want be a trace just a visual representation of simulation
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration; // Draw debuger and use lifetime
	PathParams.DrawDebugTime = 5.f; //lifetime
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;  // iniial Velocity
	PathParams.MaxSimTime = 4.f; // will do trace for 4secs bullet fly 
	PathParams.ProjectileRadius = 5.f;	
	PathParams.SimFrequency = 30.f;  // Frequence between each simulation (one sphere and another)
	PathParams.StartLocation = GetActorLocation();
	PathParams.ActorsToIgnore.Add(this);


	//something similar to HitResult
	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	*/
}



void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (OwnerCharacter) 
	{
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);

		if (OwnerController) 
		{

			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind) 
			{

				const float DamageToApply = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(OtherActor, DamageToApply, OwnerController, this, UDamageType::StaticClass());
				//calling super last cuz it will destroy this actor 
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensation()->ServerProjectileCheckHitWithRewind(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		}
		
	}


	//calling super last cuz it will destroy this actor 
	Super::OnHit(HitComp,OtherActor,OtherComp,NormalImpulse,Hit);
}


// whenever InnitialSpeed changed on the editor it will change ProjectileMovementComponnet's InitialSpeed and MaxSpeed
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	//Each UPROPERTY() variable has FName Associated with that variable
	// GET_MEMBER_NAME_CHECKED(ClasseWhereVariableExists, VariableName)  - returns FName Associated with variable


	//this will give you that FName of changed UPROPERY()
	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	
	//checking , is changed variabele the one that  we are interested
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComp)
		{
			ProjectileMovementComp->InitialSpeed = InitialSpeed;
			ProjectileMovementComp->MaxSpeed = InitialSpeed;
		}
	}
}
#endif