// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage 
{
	GENERATED_BODY()

		class UTexture2D* CrosshairCenter;
		UTexture2D* CrosshairTop;
		UTexture2D* CrosshairBottom;
		UTexture2D* CrosshairRight;
		UTexture2D* CrosshairLeft;
		float CrosshairSpread;
		FLinearColor CrosshairColor;
};
UCLASS()
class STEAMSHOOTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD()override;
	void AddCharacterOverlay();
	void AddAnouncement();
	void AddElimAnouncement(FString Attacker, FString Victim);
	
private:

	class APlayerController* OwningPlayer = nullptr;
	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D ScreenCenter,FVector2D Spread,FLinearColor CrosshairColor);
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UEliminationAnnouncement> ElimAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementLifetime = 3.f; // after 3 secs text will be removed from killfeed

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UEliminationAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UEliminationAnnouncement*> ElimMessages; // list of all elim announcements on the killfeed "Kama killed Barrikel"

protected:
	virtual void BeginPlay()override;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; };

	UPROPERTY(EditAnywhere,Category="Player Stats")
	TSubclassOf<class UUserWidget>CharacterOverlayClass;

	class UCharacterOverlay* CharacterOverlay = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget>AnnouncementClass;

	class UAnnouncement* Announcement = nullptr;
};
