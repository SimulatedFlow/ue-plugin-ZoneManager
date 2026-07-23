# ZoneManager — Data-Driven Trigger Zones with Music, Ambience & Title Cards

ZoneManager - trigger zones that actually present themselves. Priority-resolved, multi-box zones that drive on-screen title cards, sequential music playlists, entrance stingers and looping ambience - all authored in a single DataTable row.

Every open-world, adventure and survival game needs trigger zones - yet Unreal's built-in options are single-box Blueprint hacks that only fire an OnBeginOverlap event and leave the hard parts to you: which zone wins when several overlap, stopping the previous zone's music, cross-fading ambience, and showing a styled title card. ZoneManager is a C++ framework that fixes this. A zone is one or many box components; overlaps resolve by an integer Priority; and a full presentation layer - title/subtitle banner with selectable font and color, a sequential looping music playlist, a one-shot entrance sound, and looping ambience with master plus per-source volume - is driven entirely from one DataTable row.

KEY FEATURES

- Multi-box zones - a single Zone Volume spans one or many box components.
- Priority resolution - highest Priority wins on overlap (ties: Zone Level, then most-recent entry), with clean audio and banner transitions.
- DataTable-driven - every setting lives in one FZoneDefinitionRow; no per-level hand-wiring.
- On-screen title cards - title and subtitle banner with per-row font and text color.
- Sequential music playlists, entrance stinger + looping ambience, master + per-source volume.
- Blueprint-exposed World Subsystem + placeable Zone Volume actor with active-zone-changed delegates.
