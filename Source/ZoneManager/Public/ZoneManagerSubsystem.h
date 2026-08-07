// Copyright 2026 Silvan Teufel All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ZoneManagerTypes.h"
#include "ZoneManagerSubsystem.generated.h"

class AZoneVolume;
class APawn;
class UAudioComponent;
class USoundBase;
class SWidget;
class SZoneBanner;

/** Broadcast whenever the active (highest-priority) zone changes. NewZone/NewZoneData describe the
 *  zone that just became active; both are empty when the player left all zones. Bind this to drive
 *  your own UMG banner instead of (or alongside) the built-in Slate banner. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActiveZoneChanged, AZoneVolume*, NewZone, const FZoneDefinitionRow&, NewZoneData);

/** Broadcast when the local player leaves the last remaining zone (no zone is active anymore). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftAllZones);

/**
 * The central manager. One instance per world. It receives enter/exit notifications from every
 * AZoneVolume, tracks the set of zones the LOCAL player is currently inside, resolves the single
 * active zone by highest Priority (ties broken by Zone Level, then most-recently entered), and
 * drives the presentation for that zone: the on-screen title/subtitle banner (via a lightweight
 * Slate widget using the row's font & color) and the audio — a sequential, looping music playlist,
 * a one-shot entrance sound, and looping ambience, each with master + per-source volume.
 *
 * Transitions are clean: when the active zone changes, the previous zone's audio and banner are
 * stopped before the new zone's begin.
 */
UCLASS()
class ZONEMANAGER_API UZoneManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	/** Called by an AZoneVolume when a player character enters (first box touched). */
	void NotifyZoneEntered(AZoneVolume* Zone, APawn* Pawn);

	/** Called by an AZoneVolume when a player character leaves all of the zone's boxes (or the zone ends play). */
	void NotifyZoneExited(AZoneVolume* Zone, APawn* Pawn = nullptr);

	/** The currently active (highest-priority) zone the local player is inside, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "ZoneManager")
	AZoneVolume* GetActiveZone() const { return ActiveZone; }

	/** True if the local player is inside at least one zone. */
	UFUNCTION(BlueprintPure, Category = "ZoneManager")
	bool IsInsideAnyZone() const { return ActiveZone != nullptr; }

	/** Copy of the active zone's DataTable row. Returns false if no zone is active. */
	UFUNCTION(BlueprintCallable, Category = "ZoneManager")
	bool GetActiveZoneData(FZoneDefinitionRow& OutData) const;

	/** Priority of the active zone, or -1 if none is active. */
	UFUNCTION(BlueprintPure, Category = "ZoneManager")
	int32 GetActiveZonePriority() const;

	/** Fired when the active zone changes (including to "none"). */
	UPROPERTY(BlueprintAssignable, Category = "ZoneManager")
	FOnActiveZoneChanged OnActiveZoneChanged;

	/** Fired when the local player leaves the last zone. */
	UPROPERTY(BlueprintAssignable, Category = "ZoneManager")
	FOnLeftAllZones OnLeftAllZones;

private:
	/** True if Pawn is the local player's controlled pawn (the one the presentation follows). */
	bool IsLocalPlayerPawn(APawn* Pawn) const;

	/** Recompute which overlapping zone should be active and transition if it changed. */
	void ReevaluateActiveZone();

	/** Highest-priority zone among the currently overlapping zones (ties: higher level, then latest entry). */
	AZoneVolume* ResolveWinningZone() const;

	/** Stop the current zone's presentation and start NewZone's (either may be null). */
	void TransitionTo(AZoneVolume* NewZone);

	// --- Audio ---------------------------------------------------------------------------------
	void StartAudioForZone(const FZoneDefinitionRow& Row);
	void StopAllAudio(float FadeSeconds);
	void PlayCurrentMusicTrack();

	UFUNCTION()
	void HandleMusicTrackFinished();

	UFUNCTION()
	void HandleAmbientFinished();

	// --- Banner --------------------------------------------------------------------------------
	void ShowBanner(const FZoneDefinitionRow& Row);
	void HideBanner();

	/** Zones the local player currently overlaps, in entry order (last = most recent). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AZoneVolume>> OverlappingZones;

	/** The resolved active zone. */
	UPROPERTY(Transient)
	TObjectPtr<AZoneVolume> ActiveZone = nullptr;

	/** Audio component playing the current music track (recreated per track to chain the playlist). */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MusicAudio = nullptr;

	/** Audio component playing the looping ambience. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AmbientAudio = nullptr;

	/** Cached copies for the active zone so playback survives even if the source row/table changes. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<USoundBase>> ActiveMusicTracks;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> ActiveAmbientSound = nullptr;

	float ActiveMusicVolume = 1.0f;
	float ActiveAmbientVolume = 1.0f;
	float ActiveFadeSeconds = 0.5f;
	int32 CurrentMusicIndex = 0;

	// Slate banner shown on the viewport.
	TSharedPtr<SZoneBanner> BannerWidget;
	TSharedPtr<SWidget> BannerContainer;
	FTimerHandle BannerTimerHandle;
};
