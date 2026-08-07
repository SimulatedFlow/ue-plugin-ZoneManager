// Copyright 2026 Silvan Teufel All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/** ZoneManager — data-driven, priority-resolved trigger zones with music, ambience & titles (runtime module). */
class FZoneManagerModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
