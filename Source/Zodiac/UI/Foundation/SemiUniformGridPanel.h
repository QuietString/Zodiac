// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/PanelWidget.h"
#include "SemiUniformGridPanel.generated.h"

class UCommonButtonStyle;
class SSemiUniformGridPanel;
class USemiUniformGridSlot;

/**
 * allow square-like slot. e.g, 1 by 1 shape or 1 by 2 shape.
 */
UCLASS(MinimalAPI)
class USemiUniformGridPanel : public UPanelWidget
{
	GENERATED_BODY()

public:

	USemiUniformGridPanel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/** Padding given to each slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, BlueprintSetter="SetSlotPadding", Category="Child Layout")
	FMargin SlotPadding;

	/** The minimum desired width of the slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, BlueprintSetter="SetMinDesiredSlotWidth", Category="Child Layout")
	float MinDesiredSlotWidth;

	/** The minimum desired height of the slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Getter, Setter, BlueprintSetter="SetMinDesiredSlotHeight", Category="Child Layout")
	float MinDesiredSlotHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Child Layout")
	TSubclassOf<UCommonButtonStyle> Style;
	
public:

	FMargin GetSlotPadding() const;

	UFUNCTION(BlueprintCallable, Category="Child Layout")
	void SetSlotPadding(FMargin InSlotPadding);

	/** */
	float GetMinDesiredSlotWidth() const;
	
	/**  */
	UFUNCTION(BlueprintCallable, Category="Child Layout")
	void SetMinDesiredSlotWidth(float InMinDesiredSlotWidth);

	/** */
	float GetMinDesiredSlotHeight() const;
	
	/**  */
	UFUNCTION(BlueprintCallable, Category="Child Layout")
	void SetMinDesiredSlotHeight(float InMinDesiredSlotHeight);

	ZODIAC_API bool IsSlotAvailable(int32 Column, int32 Row) const;
public:

	//~ UWidget interface
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	//~ End of UWidget interface

protected:

	//~UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	//~End UPanelWidget

protected:

	TSharedPtr<SSemiUniformGridPanel> MySemiUniformGridPanel;

protected:
	//~ UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End of UWidget interface
};
