// Copyright 2026 Simulated Flow All Rights Reserved.

#include "ZoneManagerSubsystem.h"
#include "ZoneVolume.h"
#include "ZoneManagerLog.h"
#include "Slate/SZoneBanner.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/Layout/SBox.h"
#include "Styling/CoreStyle.h"   // FCoreStyle::GetDefaultFontStyle fuer den Banner-Fallback
#include "TimerManager.h"

bool UZoneManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only meaningful in an actual game/PIE world — skip editor preview / inactive worlds.
	if (const UWorld* World = Cast<UWorld>(Outer))
	{
		return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
	}
	return false;
}

void UZoneManagerSubsystem::Deinitialize()
{
	StopAllAudio(0.0f);
	HideBanner();
	OverlappingZones.Reset();
	ActiveZone = nullptr;

	Super::Deinitialize();
}

// --------------------------------------------------------------------------------------------------
// Overlap notifications from AZoneVolume
// --------------------------------------------------------------------------------------------------

void UZoneManagerSubsystem::NotifyZoneEntered(AZoneVolume* Zone, APawn* Pawn)
{
	if (!Zone || !IsLocalPlayerPawn(Pawn))
	{
		return;
	}

	// Re-add at the end so entry order is preserved (last entry wins a full priority/level tie).
	OverlappingZones.Remove(Zone);
	OverlappingZones.Add(Zone);
	ReevaluateActiveZone();
}

void UZoneManagerSubsystem::NotifyZoneExited(AZoneVolume* Zone, APawn* Pawn)
{
	if (!Zone)
	{
		return;
	}

	// Pawn is null when a zone reports its own EndPlay — accept that unconditionally.
	if (Pawn && !IsLocalPlayerPawn(Pawn))
	{
		return;
	}

	OverlappingZones.Remove(Zone);
	ReevaluateActiveZone();
}

// --------------------------------------------------------------------------------------------------
// Active-zone resolution
// --------------------------------------------------------------------------------------------------

bool UZoneManagerSubsystem::IsLocalPlayerPawn(APawn* Pawn) const
{
	return Pawn != nullptr && Pawn->IsPlayerControlled() && Pawn->IsLocallyControlled();
}

void UZoneManagerSubsystem::ReevaluateActiveZone()
{
	OverlappingZones.RemoveAll([](const TObjectPtr<AZoneVolume>& Z) { return Z == nullptr; });

	AZoneVolume* Winner = ResolveWinningZone();
	if (Winner != ActiveZone)
	{
		TransitionTo(Winner);
	}
}

AZoneVolume* UZoneManagerSubsystem::ResolveWinningZone() const
{
	AZoneVolume* Best = nullptr;
	int32 BestPriority = 0;
	int32 BestLevel = 0;

	for (const TObjectPtr<AZoneVolume>& Zone : OverlappingZones)
	{
		if (!Zone)
		{
			continue;
		}

		const int32 P = Zone->GetZonePriority();
		const int32 L = Zone->GetZoneLevel();

		// Iterating in entry order: on a full tie the later (more recent) zone replaces the earlier.
		const bool bWins = (Best == nullptr)
			|| (P > BestPriority)
			|| (P == BestPriority && L > BestLevel)
			|| (P == BestPriority && L == BestLevel);

		if (bWins)
		{
			Best = Zone;
			BestPriority = P;
			BestLevel = L;
		}
	}

	return Best;
}

void UZoneManagerSubsystem::TransitionTo(AZoneVolume* NewZone)
{
	if (NewZone == ActiveZone)
	{
		return;
	}

	// Stop the outgoing zone's presentation first for a clean hand-over.
	StopAllAudio(ActiveFadeSeconds);
	HideBanner();

	ActiveZone = NewZone;

	if (NewZone)
	{
		FZoneDefinitionRow Row;
		if (NewZone->GetZoneDefinition(Row))
		{
			StartAudioForZone(Row);
			ShowBanner(Row);
			OnActiveZoneChanged.Broadcast(NewZone, Row);
			UE_LOG(LogZoneManager, Verbose, TEXT("Active zone -> '%s' (priority %d)."),
				*NewZone->GetName(), Row.Priority);
		}
		else
		{
			UE_LOG(LogZoneManager, Warning, TEXT("ZoneVolume '%s' has no valid DataTable row."), *NewZone->GetName());
		}
	}
	else
	{
		// Left every zone.
		FZoneDefinitionRow Empty;
		OnActiveZoneChanged.Broadcast(nullptr, Empty);
		OnLeftAllZones.Broadcast();
		UE_LOG(LogZoneManager, Verbose, TEXT("Active zone -> none (left all zones)."));
	}
}

bool UZoneManagerSubsystem::GetActiveZoneData(FZoneDefinitionRow& OutData) const
{
	return ActiveZone != nullptr && ActiveZone->GetZoneDefinition(OutData);
}

int32 UZoneManagerSubsystem::GetActiveZonePriority() const
{
	return ActiveZone ? ActiveZone->GetZonePriority() : -1;
}

// --------------------------------------------------------------------------------------------------
// Audio
// --------------------------------------------------------------------------------------------------

void UZoneManagerSubsystem::StartAudioForZone(const FZoneDefinitionRow& Row)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ActiveFadeSeconds = FMath::Max(0.0f, Row.AudioFadeSeconds);

	// Entrance one-shot.
	if (Row.EntranceSound)
	{
		UGameplayStatics::PlaySound2D(World, Row.EntranceSound, Row.GetEffectiveEntranceVolume());
	}

	// Looping ambience (re-triggered on finish so non-looping assets still loop).
	ActiveAmbientSound = Row.AmbientSound;
	ActiveAmbientVolume = Row.GetEffectiveAmbientVolume();
	if (ActiveAmbientSound)
	{
		AmbientAudio = UGameplayStatics::SpawnSound2D(World, ActiveAmbientSound, ActiveAmbientVolume,
			1.0f, 0.0f, nullptr, /*bPersistAcrossLevelTransition*/ false, /*bAutoDestroy*/ false);
		if (AmbientAudio)
		{
			AmbientAudio->OnAudioFinished.AddDynamic(this, &UZoneManagerSubsystem::HandleAmbientFinished);
		}
	}

	// Sequential, looping music playlist.
	ActiveMusicTracks = Row.MusicTracks;
	ActiveMusicVolume = Row.GetEffectiveMusicVolume();
	CurrentMusicIndex = 0;
	PlayCurrentMusicTrack();
}

void UZoneManagerSubsystem::PlayCurrentMusicTrack()
{
	UWorld* World = GetWorld();
	if (!World || ActiveMusicTracks.Num() == 0)
	{
		return;
	}

	// Tear down the previous track's component first (binding removed to avoid re-entrant chaining).
	if (MusicAudio)
	{
		MusicAudio->OnAudioFinished.RemoveDynamic(this, &UZoneManagerSubsystem::HandleMusicTrackFinished);
		MusicAudio->Stop();
		MusicAudio->DestroyComponent();
		MusicAudio = nullptr;
	}

	// Find the next non-null track, wrapping around the playlist.
	const int32 Num = ActiveMusicTracks.Num();
	USoundBase* Track = nullptr;
	for (int32 Step = 0; Step < Num; ++Step)
	{
		const int32 Index = (CurrentMusicIndex + Step) % Num;
		if (ActiveMusicTracks[Index])
		{
			CurrentMusicIndex = Index;
			Track = ActiveMusicTracks[Index];
			break;
		}
	}

	if (!Track)
	{
		return; // Playlist is entirely empty entries.
	}

	MusicAudio = UGameplayStatics::SpawnSound2D(World, Track, ActiveMusicVolume,
		1.0f, 0.0f, nullptr, /*bPersistAcrossLevelTransition*/ false, /*bAutoDestroy*/ false);
	if (MusicAudio)
	{
		MusicAudio->OnAudioFinished.AddDynamic(this, &UZoneManagerSubsystem::HandleMusicTrackFinished);
	}
}

void UZoneManagerSubsystem::HandleMusicTrackFinished()
{
	// Advance to the next track; modulo in PlayCurrentMusicTrack loops the whole sequence.
	CurrentMusicIndex = (CurrentMusicIndex + 1);
	if (ActiveMusicTracks.Num() > 0)
	{
		CurrentMusicIndex %= ActiveMusicTracks.Num();
	}
	PlayCurrentMusicTrack();
}

void UZoneManagerSubsystem::HandleAmbientFinished()
{
	// Guarantee a continuous loop even if the ambient asset is not itself a looping sound.
	if (AmbientAudio && ActiveAmbientSound)
	{
		AmbientAudio->Play(0.0f);
	}
}

void UZoneManagerSubsystem::StopAllAudio(float FadeSeconds)
{
	if (MusicAudio)
	{
		MusicAudio->OnAudioFinished.RemoveDynamic(this, &UZoneManagerSubsystem::HandleMusicTrackFinished);
		if (FadeSeconds > 0.0f)
		{
			MusicAudio->FadeOut(FadeSeconds, 0.0f);
		}
		else
		{
			MusicAudio->Stop();
			MusicAudio->DestroyComponent();
		}
		MusicAudio = nullptr;
	}

	if (AmbientAudio)
	{
		AmbientAudio->OnAudioFinished.RemoveDynamic(this, &UZoneManagerSubsystem::HandleAmbientFinished);
		if (FadeSeconds > 0.0f)
		{
			AmbientAudio->FadeOut(FadeSeconds, 0.0f);
		}
		else
		{
			AmbientAudio->Stop();
			AmbientAudio->DestroyComponent();
		}
		AmbientAudio = nullptr;
	}

	ActiveMusicTracks.Reset();
	ActiveAmbientSound = nullptr;
	CurrentMusicIndex = 0;
}

// --------------------------------------------------------------------------------------------------
// On-screen banner
// --------------------------------------------------------------------------------------------------

void UZoneManagerSubsystem::ShowBanner(const FZoneDefinitionRow& Row)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameViewportClient* Viewport = World->GetGameViewport();
	if (!Viewport)
	{
		return;
	}

	// Nothing to show if there is neither a title nor a subtitle.
	if (Row.Title.IsEmpty() && Row.Subtitle.IsEmpty())
	{
		return;
	}

	HideBanner();

	// Die Schrift wird ERST HIER zusammengesetzt, nicht in der DataTable-Zeile abgelegt:
	// ein FSlateFontInfo als Zeilenfeld bringt den DataTable-Editor beim Zeilen-Hinzufuegen
	// zu Fall (siehe Kommentar an FZoneDefinitionRow::TitleFontAsset). Die Zeile traegt nur
	// noch Font-Asset + Groessen — reine Daten.
	const int32 TitleSize = FMath::Max(1, Row.TitleFontSize);
	const int32 SubSize = Row.SubtitleFontSize > 0
		? Row.SubtitleFontSize
		: FMath::Max(10, FMath::RoundToInt(TitleSize * 0.55f));

	FSlateFontInfo TitleFont = Row.TitleFontAsset
		? FSlateFontInfo(Row.TitleFontAsset, TitleSize)
		: FCoreStyle::GetDefaultFontStyle("Bold", TitleSize);
	FSlateFontInfo SubtitleFont = Row.TitleFontAsset
		? FSlateFontInfo(Row.TitleFontAsset, SubSize)
		: FCoreStyle::GetDefaultFontStyle("Bold", SubSize);

	BannerWidget = SNew(SZoneBanner)
		.Title(Row.Title)
		.Subtitle(Row.Subtitle)
		.Font(TitleFont)
		.SubtitleFont(SubtitleFont)
		.TextColor(Row.TextColor);

	BannerContainer =
		SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.0f, 120.0f, 0.0f, 0.0f))
		[
			BannerWidget.ToSharedRef()
		];

	Viewport->AddViewportWidgetContent(BannerContainer.ToSharedRef(), 100);

	// Auto-hide after the configured time; 0 keeps the banner until the active zone changes.
	if (Row.BannerDisplaySeconds > 0.0f)
	{
		World->GetTimerManager().SetTimer(BannerTimerHandle,
			FTimerDelegate::CreateUObject(this, &UZoneManagerSubsystem::HideBanner),
			Row.BannerDisplaySeconds, /*bLoop*/ false);
	}
}

void UZoneManagerSubsystem::HideBanner()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BannerTimerHandle);

		if (UGameViewportClient* Viewport = World->GetGameViewport())
		{
			if (BannerContainer.IsValid())
			{
				Viewport->RemoveViewportWidgetContent(BannerContainer.ToSharedRef());
			}
		}
	}

	BannerContainer.Reset();
	BannerWidget.Reset();
}
