// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/EliminationAnnouncement.h"
#include "Components/TextBlock.h"

void UEliminationAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s killed %s!"), *AttackerName, *VictimName);
	if (TXT_Announcement)
	{
		TXT_Announcement->SetText(FText::FromString(ElimAnnouncementText));
	}
}