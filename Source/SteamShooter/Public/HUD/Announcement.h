// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

/**
 * 
 */
UCLASS()
class STEAMSHOOTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
	
	
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarmupTime_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Announcement_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Info_Text;

};
