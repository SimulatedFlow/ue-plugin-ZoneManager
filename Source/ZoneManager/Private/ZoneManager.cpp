// Copyright 2026 Simulated Flow All Rights Reserved.

#include "ZoneManager.h"
#include "ZoneManagerLog.h"

DEFINE_LOG_CATEGORY(LogZoneManager);

#define LOCTEXT_NAMESPACE "FZoneManagerModule"

void FZoneManagerModule::StartupModule()
{
	UE_LOG(LogZoneManager, Log, TEXT("ZoneManager started."));
}

void FZoneManagerModule::ShutdownModule()
{
	UE_LOG(LogZoneManager, Log, TEXT("ZoneManager shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZoneManagerModule, ZoneManager)
