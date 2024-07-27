// the.quiet.string@gmail.com


#include "SemiUniformGridPanel.h"

#include "SemiUniformGridSlot.h"
#include "SSemiUniformGridPanel.h"
#include "UI/Menu/MenuTileBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SemiUniformGridPanel)

#define LOCTEXT_NAMESPACE "UMG"

USemiUniformGridPanel::USemiUniformGridPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);
}

FMargin USemiUniformGridPanel::GetSlotPadding() const
{
	return SlotPadding;
}

void USemiUniformGridPanel::SetSlotPadding(FMargin InSlotPadding)
{
	SlotPadding = InSlotPadding;
	if ( MySemiUniformGridPanel.IsValid() )
	{
		MySemiUniformGridPanel->SetSlotPadding(InSlotPadding);
	}
}

float USemiUniformGridPanel::GetMinDesiredSlotWidth() const
{
	return MinDesiredSlotWidth;
}

void USemiUniformGridPanel::SetMinDesiredSlotWidth(float InMinDesiredSlotWidth)
{
	MinDesiredSlotWidth = InMinDesiredSlotWidth;
	if ( MySemiUniformGridPanel.IsValid() )
	{
		MySemiUniformGridPanel->SetMinDesiredSlotWidth(InMinDesiredSlotWidth);
	}
}

float USemiUniformGridPanel::GetMinDesiredSlotHeight() const
{
	return MinDesiredSlotHeight;
}

void USemiUniformGridPanel::SetMinDesiredSlotHeight(float InMinDesiredSlotHeight)
{
	MinDesiredSlotHeight = InMinDesiredSlotHeight;
	if ( MySemiUniformGridPanel.IsValid() )
	{
		MySemiUniformGridPanel->SetMinDesiredSlotHeight(InMinDesiredSlotHeight);
	}
}

bool USemiUniformGridPanel::IsSlotAvailable(int32 Column, int32 Row)
{
	if (MySemiUniformGridPanel.IsValid())
	{
		return MySemiUniformGridPanel->IsSlotAvailable(Column, Row);
	}
	
	return false;
}

void USemiUniformGridPanel::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (!MySemiUniformGridPanel.IsValid())
	{
		return;
	}

	MySemiUniformGridPanel->SetSlotPadding(SlotPadding);
	MySemiUniformGridPanel->SetMinDesiredSlotWidth(MinDesiredSlotWidth);
	MySemiUniformGridPanel->SetMinDesiredSlotHeight(MinDesiredSlotHeight);

	if (Style)
	{
		for (UPanelSlot* PanelSlot : Slots)
		{
			if (UMenuTileBase* MenuTile = Cast<UMenuTileBase>(PanelSlot->Content))
			{
				MenuTile->SetStyleFromPanel(Style);
			}
		}
	}
}

void USemiUniformGridPanel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MySemiUniformGridPanel.Reset();
}

UClass* USemiUniformGridPanel::GetSlotClass() const
{
	return USemiUniformGridSlot::StaticClass();
}

void USemiUniformGridPanel::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if ( MySemiUniformGridPanel.IsValid() )
	{
		CastChecked<USemiUniformGridSlot>(InSlot)->BuildSlot(MySemiUniformGridPanel.ToSharedRef());

		if (Style)
		{
			if (UMenuTileBase* MenuTile = Cast<UMenuTileBase>(InSlot->Content))
			{
				MenuTile->SetStyleFromPanel(Style);
			}
		}
	}
}

void USemiUniformGridPanel::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if (MySemiUniformGridPanel.IsValid() && InSlot->Content)
	{
		TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if (Widget.IsValid())
		{
			MySemiUniformGridPanel->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

TSharedRef<SWidget> USemiUniformGridPanel::RebuildWidget()
{
	MySemiUniformGridPanel = SNew(SSemiUniformGridPanel);

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (USemiUniformGridSlot* TypedSlot = Cast<USemiUniformGridSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MySemiUniformGridPanel.ToSharedRef());
			if (Style)
			{
				if (UMenuTileBase* MenuTile = Cast<UMenuTileBase>(TypedSlot->Content))
				{
					MenuTile->SetStyleFromPanel(Style);
				}	
			}
		}
	}
	
	return MySemiUniformGridPanel.ToSharedRef();
}

#if WITH_EDITOR

const FText USemiUniformGridPanel::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}

#endif

#undef LOCTEXT_NAMESPACE
