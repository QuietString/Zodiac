// the.quiet.string@gmail.com

#pragma once

#include "Containers/Array.h"
#include "DesignerExtension.h"
#include "Input/Reply.h"
#include "Templates/SharedPointer.h"

/**
 * 
 */
class FSemiUniformGridSlotExtension : public FDesignerExtension
{
public:
	FSemiUniformGridSlotExtension();

	virtual ~FSemiUniformGridSlotExtension() override { Designer = nullptr; }

	virtual bool CanExtendSelection(const TArray< FWidgetReference >& Selection) const override;
	virtual void ExtendSelection(const TArray< FWidgetReference >& Selection, TArray< TSharedRef<FDesignerSurfaceElement> >& SurfaceElements) override;

private:

	FReply HandleShiftRow(int32 ShiftAmount);
	FReply HandleShiftColumn(int32 ShiftAmount);
	
	void ShiftRow(UWidget* Widget, int32 ShiftAmount);
	void ShiftColumn(UWidget* Widget, int32 ShiftAmount);

	FReply HandleExpandRow(int32 ExpandAmount);
	FReply HandleExpandColumn(int32 ExpandAmount);

	void ExpandRow(UWidget* Widget, int32 ExpandAmount);
	void ExpandColumn(UWidget* Widget, int32 ExpandAmount);
};
