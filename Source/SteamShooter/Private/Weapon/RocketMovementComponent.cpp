// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/RocketMovementComponent.h"

//by default Projectile MOvement Component is stops flying when it hits something
//Setting EHandleBlockingHitResult::AdvanceNextSubstep means move to next frame movment simulation
URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{

	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta,SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

//hanlles EHandleBlockingHitResult returned by HandleBlockingHit()
//since we made it empty it won't do anything and POrjectile continues moving 
void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//Rockets should not stop , only expode when "CollsisionBox" detects collision
}
