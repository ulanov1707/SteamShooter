// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "SteamShooter/Public/Components/LagCompensationComponent.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"


void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return;

	AController* InstigatorController = OwnerPawn->Controller; // won't be valid on Simulated Proxies

	const USkeletalMeshSocket* MuzzleFlashSoket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSoket)
	{
		const FTransform SocketTransform = MuzzleFlashSoket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// Who we Hit     // how may times we hited 
		TMap<ABlasterCharacter*, uint32>HitMap; // contains chacretrs that we hit (we may hit 2 different chrcaters at 1 shot)
		TMap<ABlasterCharacter*, uint32>HeadShotHitMap;

		for (FVector_NetQuantize HitTarget : HitTargets) 
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterChar = Cast<ABlasterCharacter>(FireHit.GetActor());

			//did we hit a character
			if (BlasterChar)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				if (bHeadShot) // is head shot
				{
					if (HeadShotHitMap.Contains(BlasterChar)) //if we already hitted this chacrter before
					{
						//increase number of hitted bullets
						HeadShotHitMap[BlasterChar]++;
					}
					else
					{
						HeadShotHitMap.Emplace(BlasterChar, 1);
					}
				}
				else // body shot
				{
					if (HitMap.Contains(BlasterChar)) //if we already hitted this chacrter before
					{
						//increase number of hitted bullets
						HitMap[BlasterChar]++;
					}
					else
					{
						HitMap.Emplace(BlasterChar, 1);
					}
				}
			



				//Spawn HitSound/HitParticle/FireSound/Particle at gun barrel
				if (ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}

				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.3f, FMath::FRandRange(-0.5f, 0.5f));

				}
			}


		}

		TArray<ABlasterCharacter*>HitCharaters;

		//contains total damage to each character both from bodyshots + Headshots
		TMap<ABlasterCharacter*, float>DamageMap;

		// for NOT SSR

		//calauclate body shot damage  
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)// damage should be applied only on the server 
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharaters.AddUnique(HitPair.Key);
			}
		}

		//calucaltes headshot damage 
		for (auto HeadshoHitPair : HitMap)
		{

			if (HeadshoHitPair.Key)
			{
				if (DamageMap.Contains(HeadshoHitPair.Key))
				{
				
					DamageMap[HeadshoHitPair.Key]+= HeadshoHitPair.Value *HeadShotDamage;
				}
				else
				{
					DamageMap.Emplace(HeadshoHitPair.Key, HeadshoHitPair.Value * HeadShotDamage);
				}

				HitCharaters.AddUnique(HeadshoHitPair.Key);

			}
		}

		//aplises calculated total damage to each hitted chacrater
		for (auto DamagePair : DamageMap) 
		{
			if (DamagePair.Key && InstigatorController) // damage should be applied only on the server 
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();

				if (HasAuthority() && bCauseAuthDamage) // we are on the  server and server never has a lag
				{
					//damage applied only on the server
					UGameplayStatics::ApplyDamage(DamagePair.Key,
						DamagePair.Value,  //Damage calulated in the 2 for loops above
						InstigatorController,
						this,
						UDamageType::StaticClass());
				}
			}
		}


		// for SSR
		if (!HasAuthority() && bUseServerSideRewind)
		{
			WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : WeaponOwnerController;
			WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : WeaponOwnerCharacter;

			if (WeaponOwnerCharacter && WeaponOwnerController && WeaponOwnerCharacter->GetLagCompensation() && WeaponOwnerCharacter->IsLocallyControlled())
			{
				//We are on client always dealing with data from the past
				//ex:Server sent Character update to all clients at 1:10 and client recieved this data at 1:20 so it took 10 secs to to reach client
				//so when we hit someone we hit someone's 10 secs before version and when we are sendig hit time we got to subtract that 10 secs 
				float HitTime = WeaponOwnerController->GetServerTime() - WeaponOwnerController->SingleTripTime;
				WeaponOwnerCharacter->GetLagCompensation()->ServerShotgunCheckHitWithRewind(HitCharaters, Start, HitTargets, HitTime);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithSpread(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{

	const USkeletalMeshSocket* MuzzleFlashSoket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSoket == nullptr)return;

	const FTransform SocketTransform = MuzzleFlashSoket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	//Direction to target
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//just normalizes but a bit safer

	//Getting Random point inside of sphere

	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;



	for (int32 i = 0; i < NumOfShots; i++) 
	{
		//if you use only this line it will work fine , but it has some unnecessary extra lines so for effieciencey we gonna reimplenent it 
		//HitTargets.Add(TraceEndWithSpread(HitTarget));

		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * (FMath::FRandRange(0.f, SphereRadius));
		FVector EndLoc = SphereCenter + RandVec; // visualise it as a point not a vector(if didnt get it)

		FVector ToEndLoc = EndLoc - TraceStart; //vector form start to RandomPoint of sphere
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	
	}

}
