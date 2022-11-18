// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"
#include"Engine/SkeletalMeshSocket.h"
#include "SteamShooter/Public/Weapon/Projectile.h"


void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket * MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket && InstigatorPawn) 
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();//Vector from GunBarrel to HitLocation

		/****************************/
		//THIS ONE CONVETS VECTOR TO ROTATOR AND GIVES ROTATION OF VECTOR
		FRotator TargetRotation = ToTarget.Rotation();
		/****************************/


		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();//sets owner of newly spawned actor 
		SpawnParams.Instigator = InstigatorPawn;


		AProjectile* SpawnedProjectile = nullptr;
		//spawning a projectile 
		if (bUseServerSideRewind) // Weapon using server side Rewind
		{
			if (InstigatorPawn->HasAuthority()) //server
			{
				if (InstigatorPawn->IsLocallyControlled())  // Server locallyContorlled player(Host) - Spawn Replicated Projectile
				{

					SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage; // since damage only aplied on the server ,replicated projectile will have Damage set
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				} 
				else // Server Simulated Proxy player - spawn Not Replicated Projectile  ,with  Server Side Rewind 
				{
					SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			else								// client , use Server side rewind  
			{
				if (InstigatorPawn->IsLocallyControlled())  // Locally Controlled  clinet - Spawn Not Replicated projectile with Server Side Rewind 
				{
					SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;

				}
				else  // NOT locally controlled Client - spawn Not replicated projectile with NO Server Side Rewind
				{
					SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
				
			}
		}
		else // weapon not using server side rewind 
		{
			if (InstigatorPawn->HasAuthority()) 
			{
				SpawnedProjectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage; 
				SpawnedProjectile->HeadShotDamage = HeadShotDamage;
			}
		}

	}
}
