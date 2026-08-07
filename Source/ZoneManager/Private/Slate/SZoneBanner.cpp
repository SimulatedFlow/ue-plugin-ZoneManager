// Copyright 2026 Simulated Flow All Rights Reserved.

#include "Slate/SZoneBanner.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/CoreStyle.h"

void SZoneBanner::Construct(const FArguments& InArgs)
{
	// Title font from the row; fall back to a sensible default if the designer left it unset.
	FSlateFontInfo TitleFont = InArgs._Font;
	if (!TitleFont.HasValidFont())
	{
		TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 40);
	}
	else if (TitleFont.Size <= 0)
	{
		TitleFont.Size = 40;
	}

	// Subtitle: use the explicitly supplied font when there is one, otherwise the same
	// typeface at roughly 55 % of the title size.
	FSlateFontInfo SubtitleFont = InArgs._SubtitleFont;
	if (!SubtitleFont.HasValidFont() || SubtitleFont.Size <= 0)
	{
		SubtitleFont = TitleFont;
		SubtitleFont.Size = FMath::Max(10, FMath::RoundToInt(TitleFont.Size * 0.55f));
	}

	const FLinearColor TextColor = InArgs._TextColor;
	const bool bHasSubtitle = !InArgs._Subtitle.IsEmpty();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.45f))
		.Padding(FMargin(40.0f, 18.0f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(InArgs._Title)
				.Font(TitleFont)
				.ColorAndOpacity(FSlateColor(TextColor))
				.Justification(ETextJustify::Center)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.0f, bHasSubtitle ? 6.0f : 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Visibility(bHasSubtitle ? EVisibility::Visible : EVisibility::Collapsed)
				.Text(InArgs._Subtitle)
				.Font(SubtitleFont)
				.ColorAndOpacity(FSlateColor(TextColor))
				.Justification(ETextJustify::Center)
			]
		]
	];
}
