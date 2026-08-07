// Copyright 2026 Silvan Teufel All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
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

	/**
	 * Font asset for the on-screen text (a UFont). Leave empty to use the engine default font.
	 *
	 * NOTE — why this is a plain font asset + size and NOT an FSlateFontInfo (fixed 2026-08-02):
	 * FSlateFontInfo ships a custom property-type customization (FSlateFontInfoStructCustomization).
	 * Inside a DataTable row that customization builds a custom detail node in the row editor, and
	 * the DataTable editor cannot service it — the editor fires
	 * `Ensure condition failed: bTickable` in FDetailItemNode::Tick and takes the editor down when
	 * a row is added. Fab's technical review hit exactly this ("When you try to add after the Data
	 * Table is created, the project crashes, making it impossible to follow the documentation")
	 * and our own editor log shows the same ensure 0.6 s after opening DT_ZoneDefinitions.
	 * A UFont reference plus two integers carries the same information, is plain data, and the
	 * FSlateFontInfo is assembled at display time in SZoneBanner where it belongs.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title")
	TObjectPtr<UObject> TitleFontAsset = nullptr;

	/** Point size of the title text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title", meta = (ClampMin = "1", UIMax = "128"))
	int32 TitleFontSize = 40;

	/** Point size of the subtitle text. 0 = derive from the title size (55 %). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Title", meta = (ClampMin = "0", UIMax = "128"))
	int32 SubtitleFontSize = 0;

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
