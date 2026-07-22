<!--
  Fab Marketplace store listing for ZoneManager.
  Copy the DESCRIPTION block into Fab's "Product Description" and the TECHNICAL DETAILS block into
  "Technical Details".
-->

# ZoneManager - Data-Driven Trigger Zones with Music, Ambience & On-Screen Titles - Fab Store Listing

## Headline

**ZoneManager - trigger zones that actually present themselves.**
Priority-resolved, multi-box zones that drive on-screen title cards, sequential music playlists,
entrance stingers and looping ambience - all authored in a single DataTable row.

## Pitch (1 paragraph)

Every open-world, adventure and survival game needs trigger zones - yet Unreal's built-in options are
single-box Blueprint hacks that only fire an OnBeginOverlap event and leave all the hard parts to you:
which zone wins when several overlap, stopping the previous zone's music, cross-fading ambience, and
showing a styled title card. **ZoneManager** is a C++ framework that fixes this. A zone is one OR many
box components; overlaps resolve by an integer Priority; and a full presentation layer - a title and
subtitle banner with selectable font and color, a sequential looping music playlist, a one-shot
entrance sound, and looping ambience with master plus per-source volume - is driven entirely from one
DataTable row. Place a Zone Volume, point it at a row, and the World Subsystem handles the rest with
clean transitions between zones.

## Feature Bullets

- **Multi-box zones** - a single Zone Volume spans one OR many box components; the player is "inside"
  while overlapping any box.
- **Priority resolution** - when zones overlap, the highest Priority wins (ties broken by Zone Level,
  then most-recent entry), with clean audio and banner transitions.
- **DataTable-driven** - every setting lives in one FZoneDefinitionRow; no per-level hand-wiring.
- **On-screen title cards** - title and subtitle banner with per-row font and text color.
- **Sequential music playlists** - an array of tracks plays one after another and loops while inside.
- **Entrance stinger + looping ambience** - a one-shot on enter plus continuous ambience.
- **Master + per-source volume** - one master multiplied by per-source music/entrance/ambient overrides.
- **Blueprint-exposed API** - World Subsystem plus a placeable Zone Volume actor, with delegates for
  active-zone changes so you can drive your own UMG.
- **Submission-ready** - one clean runtime module, full C++ source, engine-only dependencies, no
  third-party libs.

## Technical Specs

| | |
|---|---|
| **Engine version** | Unreal Engine 5.8 |
| **Type** | C++ Code Plugin (full source included) |
| **Modules** | ZoneManager (Runtime) |
| **Runtime platforms** | Win64 |
| **Build targets** | Development & Shipping |
| **Dependencies** | Engine modules (Slate, SlateCore, UMG, InputCore, Projects); no extra plugin |
| **Content** | Blueprint-exposed API; no mandatory content |
| **Third-party libs** | None |

## Target Audience

- Open-world, adventure, RPG, survival and exploration developers.
- Any team that has hand-wired trigger volumes per level and wants priority resolution, data-driven
  authoring, and a built-in audio plus title-card presentation layer.

## Suggested Price

**EUR 44.99** (self-serve tier).

## Suggested Tags / Keywords

Trigger Zone - Trigger Volume - Music - Ambience - Audio - Title Card - DataTable - World Subsystem -
Open World - Framework - C++

---

# ==================== TECHNICAL DETAILS (Fab form) ====================

**Features:**

- Multi-box trigger zones (one Zone Volume spans one or many box components)
- Overlap resolution by integer Priority (ties: Zone Level, then most-recent entry)
- DataTable-driven authoring: one FZoneDefinitionRow fully defines a zone
- On-screen title/subtitle banner with per-row font and text color (Slate widget)
- Sequential, looping music playlist per zone
- One-shot entrance sound plus looping ambience
- Master volume plus per-source music/entrance/ambient overrides
- Clean transitions: previous zone's audio and banner stop before the new zone begins
- Blueprint-exposed World Subsystem and placeable Zone Volume actor, with active-zone-changed delegates

**Code Modules:** ZoneManager (Runtime)
**Number of Blueprints:** 0 (C++ plugin; example DataTable assets shown in documentation)
**Number of C++ Classes:** placeable actor (AZoneVolume), world subsystem (UZoneManagerSubsystem), Slate banner widget (SZoneBanner), plus the DataTable row struct (FZoneDefinitionRow) and active-zone delegates
**Network Replicated:** No (local-player presentation)
**Supported Development Platforms:** Windows
**Supported Target Build Platforms:** Windows
**Supported Engine Versions:** 5.8
**Documentation:** https://github.com/SimulatedFlow/ue-plugin-ZoneManager
**Support:** simulatedflow@gmail.com

*ZoneManager - (c) 2026 Simulated Flow. All rights reserved.*
