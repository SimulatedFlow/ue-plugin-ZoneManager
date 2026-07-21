// Copyright 2026 Simulated Flow All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Fonts/SlateFontInfo.h"

/**
 * Lightweight on-screen title/subtitle banner. Renders the zone's Title (large) and Subtitle
 * (smaller) using the font & color configured on the DataTable row, over a subtle translucent
 * backing. Pure Slate — no UMG asset required — so it works in any project out of the box. Teams
 * that want a bespoke look can instead bind UZoneManagerSubsystem::OnActiveZoneChanged and drive
 * their own UMG widget.
 */
class ZONEMANAGER_API SZoneBanner : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZoneBanner) {}
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, Subtitle)
		SLATE_ARGUMENT(FSlateFontInfo, Font)
		SLATE_ARGUMENT(FLinearColor, TextColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
