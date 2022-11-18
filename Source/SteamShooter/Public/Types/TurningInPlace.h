#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace :uint8 
{
	ETIP_Left UMETA(DisplayName = "turning left"),
	ETIP_Right UMETA(DisplayName = "turning right"),
	ETIP_NotTurning UMETA(DisplayName = "Not turning"),


	ETIP_MAX UMETA(DisplayName = "Default Max")
};