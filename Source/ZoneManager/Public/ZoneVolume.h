// Copyright 2026 Silvan Teufel All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ZoneVolume.generated.h"

class UBoxComponent;
class APawn;
struct FZoneDefinitionRow;

/**
 * A placeable trigger zone. A single AZoneVolume can span ONE OR MULTIPLE UBoxComponents: it always
 * has a default root box, and any additional UBoxComponents you add to the actor in the editor are
 * gathered automatically at BeginPlay and treated as part of the same zone. A player is considered
 * "inside" this zone while overlapping ANY of its boxes; the zone only reports "exited" once the
 * player has left every one of its boxes.
 *
 * All of the zone's settings (title, music, ambience, priority, volume ...) come from a DataTable
 * row of type FZoneDefinitionRow, referenced through the ZoneData handle. Overlap enter/exit is
 * reported to the UZoneManagerSubsystem, which resolves the highest-priority active zone and drives
 * the on-screen banner and audio.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Zone Volume"))
class ZONEMANAGER_API AZoneVolume : public AActor
{
	GENERATED_BODY()

public:
	AZoneVolume();

	/** DataTable row (of type FZoneDefinitionRow) that fully defines this zone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZoneManager",
		meta = (RowType = "/Script/ZoneManager.ZoneDefinitionRow"))
	FDataTableRowHandle ZoneData;

	/** The default root box. Add more UBoxComponents to this actor to make the zone span several volumes. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ZoneManager")
	TObjectPtr<UBoxComponent> PrimaryBox;

	/** Resolve this zone's definition from the DataTable. Returns false if the handle is unset/invalid. */
	UFUNCTION(BlueprintCallable, Category = "ZoneManager")
	bool GetZoneDefinition(FZoneDefinitionRow& OutRow) const;

	/** This zone's priority (from its DataTable row), or 0 if the row is unresolved. */
	UFUNCTION(BlueprintPure, Category = "ZoneManager")
	int32 GetZonePriority() const;

	/** This zone's level (from its DataTable row), or 0 if the row is unresolved. */
	UFUNCTION(BlueprintPure, Category = "ZoneManager")
	int32 GetZoneLevel() const;

	/** True while at least one player character is inside this zone (overlapping any of its boxes). */
	UFUNCTION(BlueprintPure, Category = "ZoneManager")
	bool IsPlayerInside() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/** True if Actor is a player-controlled pawn (covers ACharacter). */
	static bool IsPlayerCharacter(const AActor* Actor);

	/** Number of this zone's boxes the given pawn currently overlaps. */
	int32 CountBoxOverlaps(APawn* Pawn) const;

	/** Every UBoxComponent on this actor (primary + editor-added), bound for overlap at BeginPlay. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UBoxComponent>> AllBoxes;

	/** Player pawns currently inside this zone (overlapping at least one box). */
	TSet<TWeakObjectPtr<APawn>> PlayersInside;
};
