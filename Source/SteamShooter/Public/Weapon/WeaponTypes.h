#pragma once

//length of shooting linetrace 
#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252


UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	EWT_AssaultRifle UMETA(DisplayName="AssaultRifle"),
	EWT_RocketLauncer UMETA(DisplayName = "RocketLauncer"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_FarmGun UMETA(DisplayName = "FarmGun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_Sniper UMETA(DisplayName = "Sniper"),
	EWT_GranedeLauncher UMETA(DisplayName = "GranedeLauncher"),
	EWT_Flag UMETA(DisplayName = "Flag"),

	EWT_DefaultMax UMETA(DisplayName = "DefaultMax")
};

