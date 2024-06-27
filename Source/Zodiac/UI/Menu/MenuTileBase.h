// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonButtonBase.h"
#include "GameplayTagContainer.h"
#include "MenuTileBase.generated.h"


class UImage;
class UCommonTextBlock;

USTRUCT(BlueprintType)
struct FZodiacMenuTileData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FSlateBrush Brush;
	
	UPROPERTY(EditAnywhere)
	FText Name = FText::FromString("Tile Name");

	UPROPERTY(EditAnywhere, meta=(Categories="UI.Layer"))
	FGameplayTag Layer;
	
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UCommonActivatableWidget> WidgetToPush;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class ZODIAC_API UMenuTileBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UWidget Interface

	void SetStyleFromPanel(TSubclassOf<UCommonButtonStyle> InStyle);
	
protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

	UFUNCTION()
	void HandleOnClicked();

public:
	UPROPERTY(EditAnywhere)
	FZodiacMenuTileData TileData;

protected:
	UPROPERTY(meta=(BindWidget))
	UCommonTextBlock* TileText;

	UPROPERTY(meta=(BindWidget))
	UImage* TileImage;

	// If false, it uses a style selected from outer panel. 
	UPROPERTY(EditAnywhere, Category=Style)
	bool bOverrideStyle = false;
};