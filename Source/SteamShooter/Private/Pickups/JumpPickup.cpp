// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/JumpPickup.h"
#include "SteamShooter/Public/Characters/BlasterCharacter.h"
#include"SteamShooter/Public/Components/CombatComponent.h"
#include"SteamShooter/Public/Components/BuffComponent.h"



void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{

		UBuffComponent* Buff = BlasterCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffJump(JumpZVelocityBuff, JumpBuffTime);
		}
		else 
			GEngine->AddOnScreenDebugMessage(-1, 22, FColor::Red, "AJumpPickup::Buff Comp NOT Valid");
		
	}
	

	Destroy();
}