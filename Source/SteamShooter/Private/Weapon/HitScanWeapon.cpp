// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "SteamShooter/Public/Weapon/WeaponTypes.h"
#include "SteamShooter/Public/Components/LagCompensationComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);


	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return;

	AController* InstigatorController = OwnerPawn->Controller; // won't be valid on Simulated Proxies

	const USkeletalMeshSocket* MuzzleFlashSoket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSoket) 
	{
		FTransform SocketTransform = MuzzleFlashSoket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		//do we hit a character if yes apply damage 

		ABlasterCharacter* HitChar = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (HitChar && InstigatorController) // damage should be applied only on the server 
		{
			
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage) // server doesn't need ServerSideRewind cuz it has no lag
			{

				//did we hit a head if yes , apply headshot damage
				const float DamageToApply = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				
				//damage applied only on the server
				UGameplayStatics::ApplyDamage(HitChar, DamageToApply, InstigatorController, this, UDamageType::StaticClass());
			}
			else if(!HasAuthority() && bUseServerSideRewind) // if we are client 
			{
				WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : WeaponOwnerController;
				WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : WeaponOwnerCharacter;
				
				if (WeaponOwnerCharacter && WeaponOwnerController && WeaponOwnerCharacter->GetLagCompensation() && WeaponOwnerCharacter->IsLocallyControlled()) 
				{
					//We are on client always dealing with data from the past
					//ex:Server sent Character update to all clients at 1:10 and client recieved this data at 1:20 so it took 10 secs to to reach client
					//so when we hit someone we hit someone's 10 secs before version and when we are sendig hit time we got to subtract that 10 secs 
					float HitTime = WeaponOwnerController->GetServerTime() - WeaponOwnerController->SingleTripTime;
					WeaponOwnerCharacter->GetLagCompensation()->ServerCheckHitWithRewind(HitChar, Start, HitTarget, HitTime);
				}
			}

		}
		
		//Spawn HitSound/HitParticle/FireSound/Particle at gun barrel

		if (ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}

		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}
		
		
		if (MuzzleFlashParticle) 
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashParticle, SocketTransform);
		}
		if (FireSound) 
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

	}

}

//do linesrace with spread(shotgun/mac10) 
void AHitScanWeapon::WeaponTraceHit(const FVector&TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{

	UWorld* World = GetWorld();

	if (World) 
	{
		
		//do linesrace with spread(shotgun) or not (MAC10/Pistol)
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f; 

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		FVector BeamEnd = End;

		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else 
		{
			OutHit.ImpactPoint = End;
		}

		if (BeamParticle)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticle, TraceStart, FRotator::ZeroRotator, true);
			if (Beam)
				Beam->SetVectorParameter(FName("Target"), BeamEnd); //set variable "Target" of niagara
		}
	}
}


