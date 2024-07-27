// the.quiet.string@gmail.com


#include "SemiUniformGridSlot.h"

#include "Components/Widget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SemiUniformGridSlot)

USemiUniformGridSlot::USemiUniformGridSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Slot(nullptr)
{
	HorizontalAlignment = HAlign_Fill;
	VerticalAlignment = VAlign_Fill;
}

int32 USemiUniformGridSlot::GetRow() const
{
	return Row;
}

void USemiUniformGridSlot::SetRow(int32 InRow)
{
	Row = InRow;
	if (Slot)
	{
		Slot->SetRow(InRow);
	}
}

int32 USemiUniformGridSlot::GetRowSpan() const
{
	return Slot ? Slot->GetRowSpan() : RowSpan;
}

void USemiUniformGridSlot::SetRowSpan(int32 InRowSpan)
{
	RowSpan = InRowSpan;
	if ( Slot )
	{
		Slot->SetRowSpan(InRowSpan);
	}
}

int32 USemiUniformGridSlot::GetColumn() const
{
	return Column;
}

void USemiUniformGridSlot::SetColumn(int32 InColumn)
{
	Column = InColumn;
	if ( Slot )
	{
		Slot->SetColumn(InColumn);
	}
}

int32 USemiUniformGridSlot::GetColumnSpan() const
{
	return Slot ? Slot->GetColumnSpan() : ColumnSpan;
}

void USemiUniformGridSlot::SetColumnSpan(int32 InColumnSpan)
{
	ColumnSpan = InColumnSpan;
	if ( Slot )
	{
		Slot->SetColumnSpan(InColumnSpan);
	}
}

EHorizontalAlignment USemiUniformGridSlot::GetHorizontalAlignment() const
{
	return HorizontalAlignment;
}

void USemiUniformGridSlot::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment)
{
	HorizontalAlignment = InHorizontalAlignment;
	if (Slot)
	{
		Slot->SetHorizontalAlignment(InHorizontalAlignment);
	}
}

EVerticalAlignment USemiUniformGridSlot::GetVerticalAlignment() const
{
	return VerticalAlignment;
}

void USemiUniformGridSlot::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment)
{
	VerticalAlignment = InVerticalAlignment;
	if (Slot)
	{
		Slot->SetVerticalAlignment(InVerticalAlignment);
	}
}

void USemiUniformGridSlot::SynchronizeProperties()
{
	SetRow(Row);
	SetColumn(Column);
	SetRowSpan(RowSpan);
	SetColumnSpan(ColumnSpan);
	
	SetHorizontalAlignment(HorizontalAlignment);
	SetVerticalAlignment(VerticalAlignment);
}

void USemiUniformGridSlot::BuildSlot(TSharedRef<SSemiUniformGridPanel> GridPanel)
{
	GridPanel->AddSlot(Column, Row)
		.Expose(Slot)
		.RowSpan(RowSpan)
		.ColumnSpan(ColumnSpan)
		.HAlign(HorizontalAlignment)
		.VAlign(VerticalAlignment)
		[
			Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
		];
}

void USemiUniformGridSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	
	Slot = nullptr;
}

void USemiUniformGridSlot::SynchronizeFromPreview(const UPanelSlot* const PreviewSlot)
{
	const ThisClass* const PreviewGridSlot = CastChecked<ThisClass>(PreviewSlot);
	
	SetColumn(PreviewGridSlot->GetColumn());
	SetRow(PreviewGridSlot->GetRow());
	SetColumnSpan(PreviewGridSlot->GetColumnSpan());
	SetRowSpan(PreviewGridSlot->GetRowSpan());
}

#if WITH_EDITOR

bool USemiUniformGridSlot::NudgeByDesigner(const FVector2D& NudgeDirection, const TOptional<int32>& GridSnapSize)
{
	const FVector2D ClampedDirection = NudgeDirection.ClampAxes(-1.0f, 1.0f);
	const int32 NewColumn = GetColumn() + ClampedDirection.X;
	const int32 NewRow = GetRow() + ClampedDirection.Y;

	if (NewColumn < 0 || NewRow < 0 || (NewColumn == GetColumn() && NewRow == GetRow()))
	{
		return false;
	}

	Modify();

	SetRow(NewRow);
	SetColumn(NewColumn);

	return true;
}

void USemiUniformGridSlot::SynchronizeFromTemplate(const UPanelSlot* const TemplateSlot)
{
	const ThisClass* const TemplateUniformGridSlot = CastChecked<ThisClass>(TemplateSlot);
	
	SetColumn(TemplateUniformGridSlot->GetColumn());
	SetRow(TemplateUniformGridSlot->GetRow());
	SetColumnSpan(TemplateUniformGridSlot->GetColumnSpan());
	SetRowSpan(TemplateUniformGridSlot->GetRowSpan());
}
#endif
