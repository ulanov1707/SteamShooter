// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EliminationAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API UEliminationAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetElimAnnouncementText(FString AttackerName, FString VictimName);

	UPROPERTY(meta = (BindWidget))
		class UHorizontalBox* HBOX_Announcements;

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* TXT_Announcement;

	
};
