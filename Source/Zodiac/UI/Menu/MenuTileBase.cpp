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
	//UE_LOG(LogTemp, Warning, TEXT("release resource: %s"), *GetName());

	OnClicked().Clear();
}

TSharedRef<SWidget> UMenuTileBase::RebuildWidget()
{
	//UE_LOG(LogTemp, Warning, TEXT("rebuild: %s"), *GetName());

	TSharedRef<SWidget> Result = Super::RebuildWidget();
	
	OnClicked().AddUObject(this, &ThisClass::HandleOnClicked);
	
	return Result;
}

void UMenuTileBase::HandleOnClicked()
{
	//UE_LOG(LogTemp, Warning, TEXT("HandleOnClicked"));

	if (!TileData.WidgetToPush.IsValid())
	{
		//UE_LOG(LogTemp, Warning, TEXT("widgettopush not valid"));

		return;
	}
	
	UAsyncAction_PushContentToLayerForPlayer* AsyncAction = UAsyncAction_PushContentToLayerForPlayer::PushContentToLayerForPlayer(GetOwningPlayer(), TileData.WidgetToPush, TileData.Layer, true);
	AsyncAction->Activate();
}