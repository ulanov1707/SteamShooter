#pragma once

UENUM(BlueprintType)
enum class ECombatState :uint8 
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),//не занятый 
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_ThrowingGranede UMETA(DisplayName = "ThrowingGranede"),
	ECS_SwapingWeapons UMETA(DisplayName = "SwapingWeapons"),

	ECS_DefaultMax UMETA(DisplayName = "DegaultMax")
};