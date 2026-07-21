// Copyright 2026 Simulated Flow All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Fonts/SlateFontInfo.h"
#include "ZoneManagerTypes.generated.h"

class USoundBase;

/**
 * One row of a Zone DataTable: the complete definition of a single zone.
 *
 * A DataTable of this row type is the single authoring surface for the plugin — designers add one
 * row per logical zone and reference it from an AZoneVolume via an FDataTableRowHandle. The row
 * carries identity (level + priority), the on-screen title/subtitle presentation (text, font,
 * color) and the full audio set (sequential music playlist, entrance one-shot, looping ambience)
 * with a master volume plus per-source volume overrides.
 */
USTRUCT(BlueprintType)
struct FZoneDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	// ---- Identity ------------------------------------------------------------------------------

	/** Human-friendly name of the zone (shown in logs / editor tooltips). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Identity")
	FText DisplayName;

	/** Designer-defined tier for the zone (e.g. 1 = surface, 2 = caverns). Informational / for gameplay logic. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Identity")
	int32 ZoneLevel = 0;

	/** When the player overlaps several zones at once, the zone with the HIGHEST priority wins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Identity", meta = (ClampMin = "0"))
	int32 Priority = 0;

	// ---- On-screen title -----------------------------------------------------------------------

	/** Large title shown when the player enters the zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title")
	FText Title;

	/** Smaller subtitle shown beneath the title. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title")
	FText Subtitle;

	/** Font used for the title text (selectable per row). Subtitle uses the same font at a smaller size. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title")
	FSlateFontInfo Font;

	/** Color of the on-screen title & subtitle text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title")
	FLinearColor TextColor = FLinearColor::White;

	/** How long (seconds) the title/subtitle banner stays on screen after entering. 0 = keep until zone changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title", meta = (ClampMin = "0.0"))
	float BannerDisplaySeconds = 4.0f;

	// ---- Audio sources -------------------------------------------------------------------------

	/** Music tracks played SEQUENTIALLY (one after another); the whole sequence loops while inside the zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Audio")
	TArray<TObjectPtr<USoundBase>> MusicTracks;

	/** One-shot sound played once when the player enters the zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Audio")
	TObjectPtr<USoundBase> EntranceSound = nullptr;

	/** Looping ambience played the entire time the player is inside the zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Audio")
	TObjectPtr<USoundBase> AmbientSound = nullptr;

	/** Cross-fade time (seconds) applied when this zone's audio starts and when it is stopped on a transition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Audio", meta = (ClampMin = "0.0"))
	float AudioFadeSeconds = 0.5f;

	// ---- Volume --------------------------------------------------------------------------------

	/** Master volume applied to ALL of this zone's sounds (multiplied with each per-source override). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Volume", meta = (ClampMin = "0.0", UIMax = "2.0"))
	float MasterVolume = 1.0f;

	/** Per-source override for the music playlist. Effective volume = MasterVolume * MusicVolume. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Volume", meta = (ClampMin = "0.0", UIMax = "2.0"))
	float MusicVolume = 1.0f;

	/** Per-source override for the entrance one-shot. Effective volume = MasterVolume * EntranceVolume. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Volume", meta = (ClampMin = "0.0", UIMax = "2.0"))
	float EntranceVolume = 1.0f;

	/** Per-source override for the looping ambience. Effective volume = MasterVolume * AmbientVolume. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Volume", meta = (ClampMin = "0.0", UIMax = "2.0"))
	float AmbientVolume = 1.0f;

	/** Effective music volume (master * per-source). */
	float GetEffectiveMusicVolume() const { return FMath::Max(0.0f, MasterVolume) * FMath::Max(0.0f, MusicVolume); }

	/** Effective entrance-sound volume (master * per-source). */
	float GetEffectiveEntranceVolume() const { return FMath::Max(0.0f, MasterVolume) * FMath::Max(0.0f, EntranceVolume); }

	/** Effective ambience volume (master * per-source). */
	float GetEffectiveAmbientVolume() const { return FMath::Max(0.0f, MasterVolume) * FMath::Max(0.0f, AmbientVolume); }
};
