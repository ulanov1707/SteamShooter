// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STEAMSHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UBuffComponent();

	//now Blaster Charcater will have full access to all private/protected/public variables and functions of this component 
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Heal(float HealAmount, float HealTime);
	void Shield(float ShieldAmount, float ShieldTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);
protected:
	
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);//постепенно хилится 
	void ShieldRampUp(float DeltaTime);//постепенно хилится 


private:

	class ABlasterCharacter* Character = nullptr;

	//Health Buff
	bool bHealing = false;
	float HealingRate = 0.f;// (How much to heal per second)
	float AmountToHeal = 0.f;

	//Shield
	bool bShielding = false;
	float ShieldRate = 0.f;// (How much to heal per second)
	float AmountToShield = 0.f;


	//Speed Buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed);


/**
* Jump buff
*/
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
};
