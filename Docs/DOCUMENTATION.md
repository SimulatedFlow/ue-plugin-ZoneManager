# ZoneManager — Documentation

**Data-driven, priority-resolved trigger zones with music, ambience and on-screen titles for Unreal Engine 5.8.**

---

## 1. Installation

1. Copy the `ZoneManager` folder into your project's `Plugins/` directory.
2. Open the project; when prompted, let the editor compile the plugin.
3. Enable it under **Edit → Plugins → ZoneManager** (Code Plugins) if it is not already on, then restart.

Supported engine: **Unreal Engine 5.8**. Module type: **Runtime**. Dependencies are engine-only
(`Core, CoreUObject, Engine, Slate, SlateCore, UMG, InputCore, Projects`) — no third-party code.

---

## 2. Concepts

| Piece | Type | Role |
| --- | --- | --- |
| **Zone DataTable** | `UDataTable` of `FZoneDefinitionRow` | The single authoring surface — one row per zone. |
| **Zone Volume** | `AZoneVolume` (actor) | Placeable trigger made of **one or many** box components; points at a DataTable row. |
| **Zone Manager** | `UZoneManagerSubsystem` (World Subsystem) | Resolves the active zone by priority and drives the banner + audio. |
| **Zone Banner** | `SZoneBanner` (Slate) | Built-in on-screen title/subtitle using the row's font & color. |

A **player is "inside" a zone** while overlapping *any* of the zone's boxes, so a single zone can
span several volumes. When the player overlaps **multiple** zones at once, the zone with the
**highest `Priority`** wins (ties broken by higher `Zone Level`, then by the most recently entered).
On every change the previous zone's audio and banner stop before the new zone's begin.

---

## 3. Quick start

1. **Create the DataTable.** Content Browser → *Miscellaneous → Data Table* → pick row type
   **`ZoneDefinitionRow`**. Add a row per zone and fill in the fields (see §4).
2. **Place a Zone Volume.** Drag an **`AZoneVolume`** (Zone Volume) into the level. In its Details
   panel set **`ZoneData`** → pick your DataTable and the row for this zone. Resize the default box.
3. **(Optional) Make the zone span several boxes.** With the actor selected, **Add Component →
   Box Collision** one or more times and position each box. They are all treated as one zone.
4. **Press Play** and walk your player character in. The entrance sound fires, ambience + the music
   playlist start, and the title/subtitle banner appears. Overlap a higher-priority zone to switch.

The included example map `L_ZoneManagerDemo` shows three overlapping zones with different priorities,
titles, music and sounds, driven by the sample `DT_ZoneDefinitions` table.

---

## 4. `FZoneDefinitionRow` fields

| Category | Field | Type | Meaning |
| --- | --- | --- | --- |
| Identity | `DisplayName` | `FText` | Friendly zone name (logs / tooltips). |
| Identity | `ZoneLevel` | `int32` | Designer tier; also the first tie-breaker. |
| Identity | `Priority` | `int32` | **Higher wins** when zones overlap. |
| Title | `Title` | `FText` | Large on-screen title on entry. |
| Title | `Subtitle` | `FText` | Smaller line under the title. |
| Title | `Font` | `FSlateFontInfo` | Font for the on-screen text (per row). |
| Title | `TextColor` | `FLinearColor` | Title & subtitle color. |
| Title | `BannerDisplaySeconds` | `float` | Seconds the banner stays (0 = until the zone changes). |
| Audio | `MusicTracks` | `TArray<USoundBase*>` | Played **sequentially**; the sequence **loops** while inside. |
| Audio | `EntranceSound` | `USoundBase*` | One-shot on enter. |
| Audio | `AmbientSound` | `USoundBase*` | Looping ambience while inside. |
| Audio | `AudioFadeSeconds` | `float` | Cross-fade time when this zone's audio stops. |
| Volume | `MasterVolume` | `float` | Applied to **all** of the zone's sounds. |
| Volume | `MusicVolume` | `float` | Per-source override for music (× master). |
| Volume | `EntranceVolume` | `float` | Per-source override for the entrance sound (× master). |
| Volume | `AmbientVolume` | `float` | Per-source override for the ambience (× master). |

Effective volume of any source = `MasterVolume × <source>Volume`.

---

## 5. Blueprint / C++ API

### `AZoneVolume`
- `ZoneData` (`FDataTableRowHandle`) — the row that defines this zone.
- `GetZoneDefinition(FZoneDefinitionRow& OutRow) → bool`
- `GetZonePriority() → int32`, `GetZoneLevel() → int32`, `IsPlayerInside() → bool`

### `UZoneManagerSubsystem` (get via `GetWorld()->GetSubsystem<UZoneManagerSubsystem>()`, or the
Blueprint node *Get World Subsystem → ZoneManagerSubsystem*)
- `GetActiveZone() → AZoneVolume*`
- `IsInsideAnyZone() → bool`
- `GetActiveZoneData(FZoneDefinitionRow& OutData) → bool`
- `GetActiveZonePriority() → int32`
- **`OnActiveZoneChanged(AZoneVolume* NewZone, const FZoneDefinitionRow& NewZoneData)`** — assignable
  event; fires whenever the active zone changes (including to *none*). Bind this to drive your **own
  UMG** banner instead of the built-in Slate one.
- **`OnLeftAllZones()`** — assignable event; fires when the player leaves the last zone.

### Custom UI example (C++)
```cpp
if (UZoneManagerSubsystem* Zones = GetWorld()->GetSubsystem<UZoneManagerSubsystem>())
{
    Zones->OnActiveZoneChanged.AddDynamic(this, &AMyHUD::HandleZoneChanged);
}
```

---

## 6. Notes & platform support

- The manager is a **World Subsystem**, created automatically for game/PIE worlds; there is nothing
  to place for it. Presentation (banner + 2D audio) follows the **local player** pawn.
- Audio uses `UGameplayStatics` 2D playback and standard `UAudioComponent`s; the music playlist is
  chained via `OnAudioFinished`. Non-looping ambience assets are re-triggered to guarantee a loop.
- Client-side only (no replication needed); works across Windows/macOS/Linux/mobile/console since it
  uses only cross-platform engine modules.

*© 2026 Simulated Flow. All rights reserved. Support: simulatedflow@gmail.com*
