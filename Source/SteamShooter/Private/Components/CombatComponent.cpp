// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CombatComponent.h"
#include "SteamShooter/Public/Weapon/Weapon.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include"Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include"Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include"Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"
#include "SteamShooter/Public/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "SteamShooter/Public/Characters/BlasterAnimInstance.h"
#include "SteamShooter/Public/Weapon/Projectile.h"
#include "SteamShooter/Public/Weapon/Shotgun.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWakSpeed = 330.f;

}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character) 
	{
		if (Character->HasAuthority()) 
		{
			InitCarriedAmmo();
		}
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetFollowCamera()) 
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (Character && Character->IsLocallyControlled()) 
	{
		
		FHitResult Hit;
		TraceUnderCrosshair(Hit);
		ProjectileHitLocation = Hit.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquipedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo,COND_OwnerOnly);//server will replicated variable only to owner (Autonomous Proxy)
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Nades);
	DOREPLIFETIME(UCombatComponent, bHoldingFlag);
}

void UCombatComponent::SetAimig(bool Aiming)
{
	if (Character == nullptr || EquipedWeapon == nullptr)
		return;
	// we are leving it. because if we are on the client, "Aiming" gets edited immideatly and we don't wait untill update from Server comes
	//but we edit it on the server any way (just to speed up animation on client side)
	bAiming = Aiming;

	//This will run on the server no matter do we have authority or not (just reducing some code)
	Server_SetAimimg(Aiming);

	if (Character) 
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Aiming ? AimWakSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquipedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper) 
	{
		Character->ShowSniperScopeWidget(bAiming);
	}
		
	//True Aiming State with no LAG
	if (Character->IsLocallyControlled()) 
	{
		bAimButtonPressed = Aiming;
	}
		
	
}

void UCombatComponent::Server_SetAimimg_Implementation(bool Aiming)
{
	bAiming = Aiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Aiming ? AimWakSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled()) 
	{
			bAiming = bAimButtonPressed; //Prediction 
	}

}


void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr || CombatState != ECombatState::ECS_Unoccupied) 
	{
		return;
	}

	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag) // Flag
	{

		Character->Crouch(); // moves slowly 
		bHoldingFlag = true;
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped); // hides Pickup widget/other player can't pick it up from carrieing players hand etc.
		AttachFlagToLeftHand(WeaponToEquip);
		WeaponToEquip->SetOwner(Character);		// just to not crash after SettingWeapon State 
		TheFlag = WeaponToEquip;

	}
	else //Normal Weapons
	{
		if (EquipedWeapon != nullptr && SecondWeapon == nullptr)
		{
			EquipSecondWeapon(WeaponToEquip);
		}
		else
		{
			//if we already have primary weapon it will be dropped 
			EquipPrimaryWeapon(WeaponToEquip);
		}


		//this 2 variables not replicated so we got to call them on clients as well (OnRep_EquippedWEapon)
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}


}

void UCombatComponent::SwapWeapons()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr || !Character->HasAuthority()) return;

	Character->PlaySwapWeaponsMontage();
	CombatState = ECombatState::ECS_SwapingWeapons;
	Character->bFinishedSwapping = false;
	if (SecondWeapon) SecondWeapon->EnableCustomDepth(false);
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)return;
	DropEquippedWeapon();
	EquipedWeapon = WeaponToEquip;
	EquipedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquipedWeapon);

	EquipedWeapon->SetOwner(Character);
	EquipedWeapon->SetHUDAmmo();//updates new owner's HUD with ammount of ammo newly picked up weapon
	UpdateCarriedAmmo();

	PlayEquipWeaponSound(WeaponToEquip);

	ReloadEmptyWeapon();


}

void UCombatComponent::EquipSecondWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)return;
	SecondWeapon = WeaponToEquip;
	SecondWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecond);
	SecondWeapon->SetOwner(Character);

	AttachActorToBackPack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);

}


void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquipedWeapon->IsEmpty())
	{
		Reload();
	}
}


void UCombatComponent::PlayEquipWeaponSound(AWeapon* Weapon)
{
	if (Weapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Weapon->EquipSound, Weapon->GetActorLocation(), 0.3f);
	}
}

// updates carried ammo value and HUD
void UCombatComponent::UpdateCarriedAmmo()
{
		
	if (EquipedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquipedWeapon->GetWeaponType()))
	{
			CarriedAmmo = CarriedAmmoMap[EquipedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::AttachActorToRightHand(AActor * ActorToAttach)
{
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName("RightHandSocket");
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());//replicated , not sure why
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	FName SocketName;
	if (ActorToAttach == nullptr)return;

	//pistol and Farmguns has special sockets for them , cuz they look ugly when attached to "LeftHandSocket"
	if (EquipedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquipedWeapon->GetWeaponType() == EWeaponType::EWT_FarmGun)
	{
		SocketName = FName("PistolSocket");
	}
	else
	{
		SocketName = FName("LefttHandSocket");
	}

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());//replicated , not sure why
	}
}

void UCombatComponent::AttachActorToBackPack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr)return;


	const USkeletalMeshSocket* BackSocket = Character->GetMesh()->GetSocketByName("BackPackSocket");
	if (BackSocket)
	{
		BackSocket->AttachActor(ActorToAttach, Character->GetMesh());//replicated , not sure why
	}
	
}

void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || Flag == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(Flag, Character->GetMesh());
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquipedWeapon)
	{
		EquipedWeapon->Dropped();
	}
}

void UCombatComponent::OnRep_EquipedWeapon()
{
	if (EquipedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		EquipedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		
		AttachActorToRightHand(EquipedWeapon);

		PlayEquipWeaponSound(EquipedWeapon);
		EquipedWeapon->EnableCustomDepth(false);
		EquipedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondWeapon()
{
	if (SecondWeapon && Character) 
	{
		SecondWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecond);
		AttachActorToBackPack(SecondWeapon);
		PlayEquipWeaponSound(SecondWeapon);
	}
}

//Called only on Autonoumus Proxy(Locally Controlled)
void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && !EquipedWeapon->IsFull() && (EquipedWeapon->GetAmmo() < EquipedWeapon->GetMagCapacity()) && !bLocallyReloading)
	{
		Server_Reload();
		HandleReload();
		bLocallyReloading = true;
	}
}

//called every time when new bullet inserted into shotgun during reloading (called by AnimBP AnimNotify)
void UCombatComponent::ShotgunShellReload()
{
	if(Character->HasAuthority())
		UpdateShotGunAmmoValues();
}


void  UCombatComponent::Server_Reload_Implementation() 
{
	if (Character == nullptr || EquipedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;

	if(!Character->IsLocallyControlled())
		HandleReload();


	
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState) 
	{
	case ECombatState::ECS_Reloading:
		if (!Character->IsLocallyControlled()) 
		{
			HandleReload();
		}
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) 
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGranede:
		if (Character && !Character->IsLocallyControlled())//we played montage on Autonomous proxied immideatly(ThrowGranede()) , so dont need to  do it twice 
		{
			Character->PlayThrowGranedeMontage();
			AttachActorToLeftHand(EquipedWeapon);
			ShowAttachedGranede(true);
		}
		break;
	case ECombatState::ECS_SwapingWeapons:
		if (Character && !Character->IsLocallyControlled()) 
		{
			Character->PlaySwapWeaponsMontage();
		}
		break;
	}
}
int32 UCombatComponent::AmountToReload()
{
	if (EquipedWeapon == nullptr)return 0;
	int32 SpaceInMag = EquipedWeapon->GetMagCapacity() - EquipedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquipedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquipedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(SpaceInMag, AmountCarried);
		return FMath::Clamp(SpaceInMag, 0, Least);
	}
	return 0;
}
void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
	
}

void UCombatComponent::FinishedReloading()
{
	if (Character == nullptr)return;
	bLocallyReloading = false;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmo();
	}
	
	if (bFireButtonPressed)//if after reloading player still holds down fire button , then fire
		Fire();

	
}

void UCombatComponent::FinishThrowingGranede()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquipedWeapon);

}

void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (Character) Character->bFinishedSwapping = true;
	if (SecondWeapon) SecondWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachedWeapon()
{
	AWeapon* TempWeapon = EquipedWeapon;
	EquipedWeapon = SecondWeapon;
	SecondWeapon = TempWeapon;

	EquipedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquipedWeapon);
	EquipedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquipedWeapon);

	SecondWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecond);
	AttachActorToBackPack(SecondWeapon);
}

void UCombatComponent::LaunchGranede()
{
	ShowAttachedGranede(false);
	if (Character && Character->IsLocallyControlled()) 
	{
		Server_LaunchGranede(ProjectileHitLocation);
	}

}

void UCombatComponent::Server_LaunchGranede_Implementation(const FVector_NetQuantize& Target)
{
	if (GranedeClass && Character->GetGranedeMesh())
	{
		const FVector StartLocation = Character->GetGranedeMesh()->GetComponentLocation();
		FVector LaunchDirection = Target - StartLocation;
		FRotator Rotation = LaunchDirection.Rotation();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;

		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(GranedeClass, StartLocation, Rotation, SpawnParams);
		}
	}

}

//Handles updating Ammo values after reloading
void UCombatComponent::UpdateAmmo() 
{
	if (Character == nullptr || Character->GetCombat() == nullptr)return;

	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquipedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquipedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquipedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquipedWeapon->AddAmmo(ReloadAmount);
}

//Adds 1 bullet into weapon 
void UCombatComponent::UpdateShotGunAmmoValues()
{
	if (Character == nullptr || Character->GetCombat() == nullptr)return;

	if (CarriedAmmoMap.Contains(EquipedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquipedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquipedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquipedWeapon->AddAmmo(1);
	bCanFire = true;//can shoot after each inserted bullet 

	//Handles Server , client will be handeled inside of Weapon.h/OnRep_Ammo()
	if (EquipedWeapon->IsFull() || CarriedAmmo == 0) 
	{
		JumpToShotgunEnd(); 
	}
}



void UCombatComponent::JumpToShotgunEnd()
{
	UBlasterAnimInstance* AnimInstance = Cast<UBlasterAnimInstance>(Character->GetMesh()->GetAnimInstance());
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}
// called on Autonomous proxies
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	

	if (bFireButtonPressed) 
	{
		Fire();
	}

	
}

void UCombatComponent::Fire()
{

	if (CanFire()) 
	{
		bCanFire = false;
		
		if (EquipedWeapon)
		{
			CrosshairShootFactor = 0.75f;

			switch (EquipedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
					FireProjectileWeapon();
					break;
			case EFireType::EFT_HitScan:
					FireHitScanWeapon();
					break;
			case EFireType::EFT_Shotgun:
					FireShotgun();
					break;
			
			}

		}
		StartFireTimer();
	}
	
}

void UCombatComponent::FireProjectileWeapon()
{
	// projectile weapon will have spread as well
	if (EquipedWeapon)
	{
		ProjectileHitLocation = EquipedWeapon->bUseSpread ? EquipedWeapon->TraceEndWithSpread(ProjectileHitLocation) : ProjectileHitLocation;
		if (!Character->HasAuthority()) 
		{
			LocalFire(ProjectileHitLocation);
		}
		Server_Fire(ProjectileHitLocation);
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquipedWeapon) 
	{
		ProjectileHitLocation = EquipedWeapon->bUseSpread ? EquipedWeapon->TraceEndWithSpread(ProjectileHitLocation) : ProjectileHitLocation;
		if (!Character->HasAuthority())
		{
			LocalFire(ProjectileHitLocation);
		}
		Server_Fire(ProjectileHitLocation);


	}
}

void UCombatComponent::FireShotgun()
{
	
	AShotgun* Shotgun = Cast<AShotgun>(EquipedWeapon);

	if (Shotgun) 
	{
		TArray<FVector_NetQuantize>ShotgunHitLocations;
		Shotgun->ShotgunTraceEndWithSpread(ProjectileHitLocation,ShotgunHitLocations);
		if (!Character->HasAuthority())
		{
			LocalShotGunFire(ShotgunHitLocations);
		}
		Server_ShotgunFire(ShotgunHitLocations);
		 
	}
}
void UCombatComponent::Server_ShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	Multicast_ShotgunFire(TraceHitTargets);
}

void UCombatComponent::Multicast_ShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;

	LocalShotGunFire(TraceHitTargets);
}

void UCombatComponent::LocalShotGunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquipedWeapon);

	if (Shotgun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}

}



bool UCombatComponent::CanFire()
{
	if (EquipedWeapon == nullptr) return false;

	if (bCanFire == true && !EquipedWeapon->IsEmpty() && CombatState == ECombatState::ECS_Reloading && EquipedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) // shotgun can fire during reloading
		return true;
	
	if (bLocallyReloading) return false;

	if (bCanFire == true && !EquipedWeapon->IsEmpty() && CombatState == ECombatState::ECS_Unoccupied)
		return true;
	else
		return false;
}

void UCombatComponent::StartFireTimer()
{
	if (!EquipedWeapon || !Character) 
	{
		bCanFire = true;
		return;
	}
	
	Character->GetWorldTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&UCombatComponent::FireTimerFinished,
		EquipedWeapon->FireDelay);

}

void UCombatComponent::FireTimerFinished()
{
	if (!EquipedWeapon) return;

	bCanFire = true;
	if (bFireButtonPressed && EquipedWeapon->bIsAutomatic)
	{
		Fire();
	}
	ReloadEmptyWeapon();// if weapon is emty after shot it will reload automatically
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	//if during reloading a shot gun , we have no carried ammo for shotgun 
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading && EquipedWeapon != nullptr
		&& EquipedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun
		&& CarriedAmmo == 0;
	if (bJumpToShotgunEnd) 
	{
		JumpToShotgunEnd();
	}
}

//decides how much ammo will be carried with player and what types of ammo . at round start
void UCombatComponent::InitCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncer, StartRLAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_FarmGun, StartFarmGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GranedeLauncher, StartGranedeLauncherAmmo);
}

void UCombatComponent::Server_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	Multicast_Fire(TraceHitTarget);
}

void UCombatComponent::Multicast_Fire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;

	LocalFire(TraceHitTarget);
	
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquipedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquipedWeapon->Fire(TraceHitTarget);//plays anim of weapon fire and spawn projectile 
	}
}




//function will work only on Autonomous Proxy, cuz it uses Viewport data for caluclations . And only autonomous proxy will have valid values for it 
//Does a linetrace from camera
void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}
	//middle of viewport in screen space(2D space)
	FVector2D CrosshairLocation = FVector2D(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Converts location(crosshair) from 2D to 3D ,and also gives a direction 
	bool bScreenToWorldSuccess = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation,//location to convert 
		CrosshairWorldPosition,
		CrosshairWorldDirection

	);

	if (bScreenToWorldSuccess) 
	{
		FVector Start = CrosshairWorldPosition;

		//we chould start linetrace is a little forward from chacrater , so the linetrace never hits character
		float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();//distace between chacrter and camera 
		Start += (CrosshairWorldDirection * (DistanceToCharacter + 100.f));

		FVector End = Start + (CrosshairWorldDirection * TRACE_LENGTH);

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		
		//if we Didn't hit anything 
		/** bBlockingHit - Indicates if this hit was a result of blocking collision. If false, there was no hit or it was an overlap/touch instead. */
		if (!TraceHitResult.bBlockingHit) 
		{	
			TraceHitResult.ImpactPoint = End;
		}
		else //if we hit 
		{
			if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>()) 
			{
				HUDPackage.CrosshairColor = FLinearColor::Red;
			}
			else 
			{
				HUDPackage.CrosshairColor = FLinearColor::White;
			}
		}
	}


}

//Handles Dynamic Crosshair
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{

	if (Character == nullptr || Character->Controller==nullptr)
		return;


	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;

	if (PlayerController) 
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(PlayerController->GetHUD()) : HUD;
		if (HUD) 
		{
			
			if (EquipedWeapon) 
			{
				HUDPackage.CrosshairCenter = EquipedWeapon->CrosshairCenter;
				HUDPackage.CrosshairTop = EquipedWeapon->CrosshairTop;
				HUDPackage.CrosshairRight = EquipedWeapon->CrosshairRight;
				HUDPackage.CrosshairLeft = EquipedWeapon->CrosshairLeft;
				HUDPackage.CrosshairBottom = EquipedWeapon->CrosshairBottom;

			}
			else 
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairBottom = nullptr;

			}

			//Calculate Crosshair Spread
			//we want crosshair to spread depending on a player veclocty

			//we want [0-600] convert(map) to [0-1] ex: if we got 300 it means mapped value will be 0.5

			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplyer(0.f,1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			//Takes speed and converts from [0-600] to [0-1] value
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,VelocityMultiplyer,Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling()) 
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else 
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming) 
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else 
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			//speed of coming back of crosshair to initial position 
			CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor,0,DeltaTime,10.f);

			//this much our crosshair will move depending 
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootFactor;



			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::ThrowGranede()
{
	if (Nades == 0)return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquipedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_ThrowingGranede;
	if (Character) 
	{
		Character->PlayThrowGranedeMontage();
		AttachActorToLeftHand(EquipedWeapon);
		ShowAttachedGranede(true);
	}
	if (!Character->HasAuthority())
	{
		Server_ThrowGranede();
	}
	if (Character->HasAuthority())
	{
		Nades = FMath::Clamp(Nades - 1, 0, MaxNades);
		UpdateHUDNades();
	}

}

void UCombatComponent::Server_ThrowGranede_Implementation()
{
	if (Nades == 0)return;
	CombatState = ECombatState::ECS_ThrowingGranede;

	if (Character)
	{
		Character->PlayThrowGranedeMontage();
		AttachActorToLeftHand(EquipedWeapon);
		ShowAttachedGranede(true);
	}

	Nades = FMath::Clamp(Nades - 1, 0, MaxNades);
	UpdateHUDNades();
}

void UCombatComponent::UpdateHUDNades()
{
	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController) 
	{
		PlayerController->SetHUDNades(Nades);
	}
	
}
bool UCombatComponent::ShouldSwapWeapons()
{
	return EquipedWeapon && SecondWeapon;
}


void UCombatComponent::OnRep_Nades()
{
	UpdateHUDNades();
}


void UCombatComponent::ShowAttachedGranede(bool bShowMesh)
{
	if (Character && Character->GetGranedeMesh()) 
	{
		Character->GetGranedeMesh()->SetVisibility(bShowMesh);
	}
}

//Server 
void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType)) 
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	if (EquipedWeapon && EquipedWeapon->IsEmpty() && EquipedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}

}


void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquipedWeapon == nullptr)return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV,EquipedWeapon->GetZoomedFOV(),DeltaTime,EquipedWeapon->GetZoomInterpSpeed());
		
	}
	else 
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UCombatComponent::OnRep_HoldingTheFlag()
{
	if (bHoldingFlag && Character && Character->IsLocallyControlled())
	{
		Character->Crouch(); 
	}
}