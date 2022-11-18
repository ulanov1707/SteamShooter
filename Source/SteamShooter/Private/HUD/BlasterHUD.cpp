// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "SteamShooter/Public/HUD/CharacterOverlay.h"
#include "SteamShooter/Public/HUD/Announcement.h"
#include "SteamShooter/Public/HUD/EliminationAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"




void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PC = GetOwningPlayerController();
	if (PC && CharacterOverlayClass && CharacterOverlay==nullptr) 
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PC,CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnouncement()
{
	APlayerController* PC = GetOwningPlayerController();
	if (PC && AnnouncementClass && Announcement == nullptr)
	{
		Announcement = CreateWidget<UAnnouncement>(PC, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::AddElimAnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && ElimAnnouncementClass) 
	{
		UEliminationAnnouncement* ElimAnnouncenmentWidget = CreateWidget<UEliminationAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if(ElimAnnouncenmentWidget)
		{
			ElimAnnouncenmentWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncenmentWidget->AddToViewport();

			//pushes all previous killfeed texts up 
			for (UEliminationAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->HBOX_Announcements)
				{
					//canvas slot alllow stuff like for editing  position , size, setanchor etc
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->HBOX_Announcements);

					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(
							CanvasSlot->GetPosition().X,
							Position.Y - CanvasSlot->GetSize().Y
						);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}


			ElimMessages.Add(ElimAnnouncenmentWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;


			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncenmentWidget);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,
				ElimAnnouncementLifetime,
				false
			);
		}
	}
}
void ABlasterHUD::ElimAnnouncementTimerFinished(UEliminationAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}


void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine) 
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		//Max Spread value will be multiplied by this value(0 - 2.27) 
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairCenter) 
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter,ViewportCenter,Spread,HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f,-SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}

}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ScreenCenter, FVector2D Spread,FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	FVector2D TextureDrawPoint(ScreenCenter.X -(TextureWidth / 2.f) + Spread.X, ScreenCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(Texture,TextureDrawPoint.X,TextureDrawPoint.Y,TextureWidth,TextureHeight,0.f,0.f,1.f,1.f,CrosshairColor);
	
}


