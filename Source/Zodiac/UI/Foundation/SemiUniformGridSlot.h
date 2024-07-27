// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "SSemiUniformGridPanel.h"
#include "Components/GridSlot.h"
#include "SemiUniformGridSlot.generated.h"

class SUniformGridPanel;
/**
 * 
 */
UCLASS()
class ZODIAC_API USemiUniformGridSlot : public UPanelSlot
{
	GENERATED_BODY()

public:

	USemiUniformGridSlot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Gets the row index of the slot */
	int32 GetRow() const;

	/** Sets the row index of the slot, this determines what cell the slot is in the panel */
	UFUNCTION(BlueprintCallable, Category="Layout|SemiUniform Grid Slot")
	void SetRow(int32 InRow);

	int32 GetRowSpan() const;

	UFUNCTION(BlueprintCallable, Category="Layout|SemiUniform Grid Slot")
	void SetRowSpan(int32 InRowSpan);
	
	/** Gets the column index of the slot. */
	int32 GetColumn() const;
	
	/** Sets the column index of the slot, this determines what cell the slot is in the panel */
	UFUNCTION(BlueprintCallable, Category="Layout|SemiUniform Grid Slot")
	void SetColumn(int32 InColumn);

	/** Gets how many columns this slot spans over */
	int32 GetColumnSpan() const;

	/** How many columns this slot spans over */
	UFUNCTION(BlueprintCallable, Category="Layout|SemiUniformGrid Slot")
	void SetColumnSpan(int32 InColumnSpan);

	EHorizontalAlignment GetHorizontalAlignment() const;

	UFUNCTION(BlueprintCallable, Category="Layout|SemiUniform Grid Slot")
	void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	EVerticalAlignment GetVerticalAlignment() const;

	UFUNCTION(BlueprintCallable, Category="Layout|SemiUniform Grid Slot")
	void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

public:

	//~ UPanelSlot interface
	virtual void SynchronizeProperties() override;
	//~ End of UPanelSlot interface

	/** Builds the underlying FSlot for the Slate layout panel. */
	void BuildSlot(TSharedRef<SSemiUniformGridPanel> GridPanel);

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	void SynchronizeFromPreview(const UPanelSlot* const PreviewSlot);
	
#if WITH_EDITOR
	virtual bool NudgeByDesigner(const FVector2D& NudgeDirection, const TOptional<int32>& GridSnapSize) override;
	virtual void SynchronizeFromTemplate(const UPanelSlot* const TemplateSlot) override;
#endif

protected:
	/** The alignment of the object horizontally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Setter, BlueprintSetter="SetHorizontalAlignment", Category="Layout|SemiUniform Grid Slot")
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;

	/** The alignment of the object vertically. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Setter, BlueprintSetter="SetVerticalAlignment", Category="Layout|SemiUniform Grid Slot")
	TEnumAsByte<EVerticalAlignment> VerticalAlignment;
	
	/** The row index of the cell this slot is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Setter, BlueprintSetter="SetRow", meta=( UIMin = "0" ), Category="Layout|SemiUniform Grid Slot")
	int32 Row;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Setter, BlueprintSetter="SetRowSpan", meta=(UIMin = "1"), Category="Layout|SemiUniformGrid Slot")
	int32 RowSpan;
	
	/** The column index of the cell this slot is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Setter, BlueprintSetter="SetColumn", meta=( UIMin = "0" ), Category="Layout|SemiUniform Grid Slot")
	int32 Column;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Setter, BlueprintSetter="SetColumnSpan", meta=(UIMin = "1"), Category="Layout|Grid Slot")
	int32 ColumnSpan;
	
private:
	/** A raw pointer to the slot to allow us to adjust the size, padding...etc at runtime. */
	SSemiUniformGridPanel::FSlot* Slot;
};
