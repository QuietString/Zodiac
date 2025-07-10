// the.quiet.string@gmail.com

#include "MenuTileBase.h"

#include "CommonTextBlock.h"
#include "Actions/AsyncAction_PushContentToLayerForPlayer.h"
#include "Components/Image.h"

void UMenuTileBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	if (TileImage)
	{
		TileImage->SetBrush(TileData.Brush);
	}

	if (TileText)
	{
		TileText->SetText(TileData.Name);
	}
}

void UMenuTileBase::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	OnClicked().Clear();
}

void UMenuTileBase::SetStyleFromPanel(const TSubclassOf<UCommonButtonStyle> InStyle)
{
	if (!bOverrideStyle)
	{
		SetStyle(InStyle);
	}
}

TSharedRef<SWidget> UMenuTileBase::RebuildWidget()
{
	TSharedRef<SWidget> Result = Super::RebuildWidget();
	
	OnClicked().AddUObject(this, &ThisClass::HandleOnClicked);
	
	return Result;
}

void UMenuTileBase::HandleOnClicked()
{
	if (!TileData.WidgetToPush.IsValid())
	{
		return;
	}
	
	UAsyncAction_PushContentToLayerForPlayer* AsyncAction = UAsyncAction_PushContentToLayerForPlayer::PushContentToLayerForPlayer(GetOwningPlayer(), TileData.WidgetToPush, TileData.Layer, true);
	AsyncAction->Activate();
}