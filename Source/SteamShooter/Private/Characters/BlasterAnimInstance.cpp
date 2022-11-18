// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BlasterAnimInstance.h"
#include "../Public/Characters/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SteamShooter/Public/Weapon/Weapon.h"
#include "SteamShooter/Public/Types/CombatState.h"

//like begin play
void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr) 
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;//if even after line above Character not found then return 

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;

	speed = Velocity.Size();
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsInAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bIsWeaponEquiped = BlasterCharacter->IsWeaponEquipped();
	EquipedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;//ACharacter native variable 
	bIsAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bEliminated = BlasterCharacter->IsEliminated();
	bHoldingTheFlag = BlasterCharacter->IsHoldingFlag();

	//Offset Yaw For Strafing
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());	
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,DeltaRot,DeltaTime,8.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator  Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	//For Correct Left hand position with gun
	if (bIsWeaponEquiped && EquipedWeapon && EquipedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh()) 
	{
		LeftHandTransform = EquipedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"),ERelativeTransformSpace::RTS_World);
		FVector OutPostion;
		FRotator OutRotation;

		//	Transform a location/rotation from world space to bone relative space.
		//This is handy if you know the location in world space for a bone attachment, as AttachComponent takes location / rotation in bone - relative space.
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("Hand_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPostion, OutRotation);
		LeftHandTransform.SetLocation(OutPostion);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		

		//We don't want to relplicate this stuff to other players 
		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControled = true;
			FTransform RightHandTransform = EquipedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			 
			//Finds correct rotation for Hand_R bone to face Crosshair(BUG:not working as expected)
			FRotator LookAtRotator = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(),
				RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetBulletHitLocation()));//"Hand_R" bone is facing opposite direction and to have a correct lookAtDirection we are makig this with end location (it kind of reverses a BulletHitLocation)
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotator, DeltaTime, 30.f);
		}
		/*****
		//gives transfrom of MuzzleFlash Socket in world space
		FTransform SocketTransform = EquipedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//gives X basis vector of "MuzzleFlash" socket as unit vector 
		FVector Direction(FRotationMatrix(SocketTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		DrawDebugLine(GetWorld(), SocketTransform.GetLocation(), SocketTransform.GetLocation() + Direction * 1000.f,FColor::Red);
		DrawDebugLine(GetWorld(), SocketTransform.GetLocation(), BlasterCharacter->GetBulletHitLocation(), FColor::Yellow);
		****/
	}

	//if not reloading use FABRIK , if reloading NOT use FABRIK
	bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride = BlasterCharacter->IsLocallyControlled() &&
		BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGranede /* &&
		BlasterCharacter->bFinishedSwapping*/;

	if (bFABRIKOverride)
	{
		bUseFABRIK = !BlasterCharacter->IsLocallyReloading();
	}
	bUseAimOffset = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
	bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
}
