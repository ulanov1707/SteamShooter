// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include "SteamShooter/Public/Components/CombatComponent.h"
#include"Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "SteamShooter/Public/Weapon/BulletCasing.h"
#include"Engine/SkeletalMeshSocket.h"
#include "SteamShooter/Public/PlayerControllers/BlasterPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AWeapon::AWeapon()
{

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMesh->SetupAttachment(GetRootComponent());
	SetRootComponent(WeaponMesh);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	PickUpWidget->SetupAttachment(WeaponMesh);


	//Collision will be enabled on BeginPlay() 
 	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN); // color of Weapon outline 
	WeaponMesh->MarkRenderStateDirty(); 
	EnableCustomDepth(true);//all weapons will be outlined at the start

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area Sphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	


}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	
	if (PickUpWidget) 
	{
		ShowPickupWidget(false);
	}
}
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind,COND_OwnerOnly);
}

//server 
void AWeapon::SetWeaponState(EWeaponState NewState)
{
	WeaponState = NewState;
	OnWeaponStateSet();
}

//client
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}


//Handles chages of Weapon State
void AWeapon::OnWeaponStateSet()
{
	if (WeaponState == EWeaponState::EWS_Equipped)
	{
		OnEquipped();
	}
	else if (WeaponState == EWeaponState::EWS_Dropped)
	{
		OnDropped();
	}
	else if (WeaponState == EWeaponState::EWS_EquippedSecond) 
	{
		OnEquippedSecond();
	}

}

//When Weapon state set to EWS_Equipped
void AWeapon::OnEquipped()
{

	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//EnableCustomDepth(false);
	
	//Binding HighPing Delegate
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : WeaponOwnerCharacter;
	if (WeaponOwnerCharacter && bUseServerSideRewind)
	{
		WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<ABlasterPlayerController>(WeaponOwnerCharacter->Controller) : WeaponOwnerController;
		if (WeaponOwnerController && HasAuthority() && !WeaponOwnerController->HighPingDelegate.IsBound())
		{
			WeaponOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}
//When Weapon state set to EWS_Dropped
void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN); // color of Weapon outline 
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);//all weapons will be outlined at the start

	//Unbinding HighPing Delegate
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : WeaponOwnerCharacter;
	if (WeaponOwnerCharacter && bUseServerSideRewind)
	{
		WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<ABlasterPlayerController>(WeaponOwnerCharacter->Controller) : WeaponOwnerController;
		if (WeaponOwnerController && HasAuthority() && WeaponOwnerController->HighPingDelegate.IsBound())
		{
			WeaponOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecond()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN); // color of Weapon outline 
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	//Unbinding HighPing Delegate
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : WeaponOwnerCharacter;
	if (WeaponOwnerCharacter && bUseServerSideRewind)
	{
		WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<ABlasterPlayerController>(WeaponOwnerCharacter->Controller) : WeaponOwnerController;
		if (WeaponOwnerController && HasAuthority() && WeaponOwnerController->HighPingDelegate.IsBound())
		{
			WeaponOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//since we binded callback only on the server, this callback function will be always called only on the server
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter * Char = Cast<ABlasterCharacter>(OtherActor);
	if (Char) 
	{
		
	
		if (WeaponType == EWeaponType::EWT_Flag && Char->GetTeam() == Team) return; // can't pick up own teams flag only other team's 
		if (Char->IsHoldingFlag())return; // can't pick up weapons/flags if already holding a flag 
		Char->SetWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* Char = Cast<ABlasterCharacter>(OtherActor);
	if (Char) 
	{
		if (WeaponType == EWeaponType::EWT_Flag && Char->GetTeam() == Team) return; 
		if (Char->IsHoldingFlag())return;
		Char->SetWeapon(nullptr);
	}
	
}


void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickUpWidget) 
	{
		PickUpWidget->SetVisibility(bShowWidget);
	}
}

//Childs will add additional functionality 
void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation) 
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass) 
	{
		const USkeletalMeshSocket* CasingSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (CasingClass) 
		{
		
				FTransform SocketTransform = CasingSocket->GetSocketTransform(WeaponMesh);
				
				//ABulletCashing actor is NOT Replicatied , but it will be visible on both clients and server why ? 
				//Fire() function is also NOT an RPC ,but ABulletCashing actor is still works on client and server why ?
				//Because Fire() function is called inside of  Multicast RPC in CombatComponent.h(which will run Fire() on server and all clients ) 
				//So technically Fire() is called on Server and all clients , so Server and All clients will spawn ABulletCasing 
				//but keep in mind that ABulletCasing(just cosmetics) and  not replicated ,so  its independent from Server
				// since its independent each machine might have a little different movement of casing ,and each client is in response to handle destruction of it as well
				GetWorld()->SpawnActor<ABulletCasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
					);

			}
		
	}

	SpendBullet();

	
}

//updates Ammo text on the HUD
void AWeapon::SetHUDAmmo() 
{
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : WeaponOwnerCharacter; 
		if (WeaponOwnerCharacter)
		{
			WeaponOwnerController = WeaponOwnerController == nullptr ? Cast<ABlasterPlayerController>(WeaponOwnerCharacter->Controller) : WeaponOwnerController;
			if (WeaponOwnerController)
			{
				WeaponOwnerController->SetHUDWeaponAmmo(Ammo);
			}
		}
}


void AWeapon::SpendBullet()
{
	Ammo = FMath::Clamp(Ammo - 1,0,MagCapacity);
	SetHUDAmmo();
	if (HasAuthority()) 
	{
		Client_UpdateAmmo(Ammo);
	}
	else //Clinet
	{
		++Sequence;
	}
}

void AWeapon::Client_UpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return; // not neccessary just in case

	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

//only called on Server 
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();//updats ammo hud on server
	Client_AddAmmo(AmmoToAdd);
}

void AWeapon::Client_AddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return; // not neccessary just in case

	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : WeaponOwnerCharacter;
	if (WeaponOwnerCharacter && WeaponOwnerCharacter->GetCombat() && IsFull()) 
	{
		WeaponOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}


void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr) 
	{
		WeaponOwnerCharacter = nullptr;
		WeaponOwnerController = nullptr;
	}
	else 
	{
		WeaponOwnerCharacter = WeaponOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : WeaponOwnerCharacter;
		if(WeaponOwnerCharacter && WeaponOwnerCharacter->GetEquippedWeapon() && WeaponOwnerCharacter->GetEquippedWeapon() == this) //if we equipped a primary weapon we update hud , if secondary no need to update
			SetHUDAmmo();
	}

}
void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//just some stupit detachment rules that DetachFromComponent() needs. no worries
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	WeaponOwnerCharacter = nullptr;
	WeaponOwnerController = nullptr;
}


void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh) 
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}


//returns EndPoint for line trace , for the weapons that uses bUseSpread (shotgun)

FVector AWeapon::TraceEndWithSpread(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSoket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSoket == nullptr)return FVector();

	FTransform SocketTransform = MuzzleFlashSoket->GetSocketTransform(GetWeaponMesh());
	FVector TraceStart = SocketTransform.GetLocation();

	//Direction to target
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//just normalizes but a bit safer

	//Getting Random point inside of sphere

	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * (FMath::FRandRange(0.f, SphereRadius));
	FVector EndLoc = SphereCenter + RandVec; // visualise it as a point not a vector(if didnt get it)

	FVector ToEndLoc = EndLoc - TraceStart; //vector form start to RandomPoint of sphere

	//To visually see on which direction bullets are going
	/*
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red,true);
	DrawDebugSphere(GetWorld(), EndLoc, 10.f, 5.f, FColor::Blue, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Green, true);
	*/


	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());//division at the end is just to not oveflow "double" datatype's size

}



//call back to HighPingDelegate of BlasterPlayerController
//we don't do SSR on high ping
void AWeapon::OnPingTooHigh(bool bIsPingTooHigh)
{
	if (bIsPingTooHigh == true) 
	{
		bUseServerSideRewind = false;
	}
	else 
	{
		bUseServerSideRewind = true;
	}
}
