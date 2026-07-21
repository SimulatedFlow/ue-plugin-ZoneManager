// Copyright 2026 Simulated Flow All Rights Reserved.

#include "ZoneVolume.h"
#include "ZoneManagerTypes.h"
#include "ZoneManagerSubsystem.h"
#include "ZoneManagerLog.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

AZoneVolume::AZoneVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	PrimaryBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PrimaryBox"));
	SetRootComponent(PrimaryBox);
	PrimaryBox->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
	PrimaryBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PrimaryBox->SetCollisionObjectType(ECC_WorldStatic);
	PrimaryBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	PrimaryBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PrimaryBox->SetGenerateOverlapEvents(true);
	PrimaryBox->ShapeColor = FColor(64, 160, 255);
}

void AZoneVolume::BeginPlay()
{
	Super::BeginPlay();

	// Gather the primary box plus any additional box components added to this actor in the editor,
	// so a single zone can span several volumes.
	AllBoxes.Reset();
	TArray<UBoxComponent*> Boxes;
	GetComponents<UBoxComponent>(Boxes);
	for (UBoxComponent* Box : Boxes)
	{
		if (!Box)
		{
			continue;
		}
		Box->SetGenerateOverlapEvents(true);
		Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Box->OnComponentBeginOverlap.AddDynamic(this, &AZoneVolume::HandleBeginOverlap);
		Box->OnComponentEndOverlap.AddDynamic(this, &AZoneVolume::HandleEndOverlap);
		AllBoxes.Add(Box);
	}

	if (AllBoxes.Num() == 0)
	{
		UE_LOG(LogZoneManager, Warning, TEXT("ZoneVolume '%s' has no box components."), *GetName());
	}
}

void AZoneVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Make sure the manager forgets this zone if it is destroyed while a player is inside.
	if (PlayersInside.Num() > 0)
	{
		if (UWorld* World = GetWorld())
		{
			if (UZoneManagerSubsystem* Manager = World->GetSubsystem<UZoneManagerSubsystem>())
			{
				Manager->NotifyZoneExited(this);
			}
		}
	}
	PlayersInside.Reset();

	Super::EndPlay(EndPlayReason);
}

void AZoneVolume::HandleBeginOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (!IsPlayerCharacter(OtherActor))
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	const bool bAlreadyInside = PlayersInside.Contains(Pawn);
	PlayersInside.Add(Pawn);

	// First box of this zone that the player touches => the player entered the zone.
	if (!bAlreadyInside)
	{
		if (UWorld* World = GetWorld())
		{
			if (UZoneManagerSubsystem* Manager = World->GetSubsystem<UZoneManagerSubsystem>())
			{
				Manager->NotifyZoneEntered(this, Pawn);
			}
		}
	}
}

void AZoneVolume::HandleEndOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	if (!IsPlayerCharacter(OtherActor))
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!PlayersInside.Contains(Pawn))
	{
		return;
	}

	// The player only leaves the zone once it no longer overlaps ANY of this zone's boxes.
	if (CountBoxOverlaps(Pawn) > 0)
	{
		return;
	}

	PlayersInside.Remove(Pawn);
	if (UWorld* World = GetWorld())
	{
		if (UZoneManagerSubsystem* Manager = World->GetSubsystem<UZoneManagerSubsystem>())
		{
			Manager->NotifyZoneExited(this, Pawn);
		}
	}
}

bool AZoneVolume::GetZoneDefinition(FZoneDefinitionRow& OutRow) const
{
	if (!ZoneData.DataTable || ZoneData.RowName.IsNone())
	{
		return false;
	}

	static const FString Context(TEXT("AZoneVolume::GetZoneDefinition"));
	if (const FZoneDefinitionRow* Found = ZoneData.DataTable->FindRow<FZoneDefinitionRow>(ZoneData.RowName, Context, /*bWarnIfMissing*/ false))
	{
		OutRow = *Found;
		return true;
	}
	return false;
}

int32 AZoneVolume::GetZonePriority() const
{
	FZoneDefinitionRow Row;
	return GetZoneDefinition(Row) ? Row.Priority : 0;
}

int32 AZoneVolume::GetZoneLevel() const
{
	FZoneDefinitionRow Row;
	return GetZoneDefinition(Row) ? Row.ZoneLevel : 0;
}

bool AZoneVolume::IsPlayerInside() const
{
	for (const TWeakObjectPtr<APawn>& Weak : PlayersInside)
	{
		if (Weak.IsValid())
		{
			return true;
		}
	}
	return false;
}

bool AZoneVolume::IsPlayerCharacter(const AActor* Actor)
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn != nullptr && Pawn->IsPlayerControlled();
}

int32 AZoneVolume::CountBoxOverlaps(APawn* Pawn) const
{
	if (!Pawn)
	{
		return 0;
	}

	int32 Count = 0;
	for (const TObjectPtr<UBoxComponent>& Box : AllBoxes)
	{
		if (Box && Box->IsOverlappingActor(Pawn))
		{
			++Count;
		}
	}
	return Count;
}
