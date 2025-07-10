// the.quiet.string@gmail.com


#include "SSemiUniformGridPanel.h"
#include "Layout/LayoutUtils.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLATE_IMPLEMENT_WIDGET(SSemiUniformGridPanel)
void SSemiUniformGridPanel::PrivateRegisterAttributes(FSlateAttributeInitializer& SlateAttributeInitializer)
{
	FSlateWidgetSlotAttributeInitializer Initializer = SLATE_ADD_PANELCHILDREN_DEFINITION(SlateAttributeInitializer, Children);
}

void SSemiUniformGridPanel::FSlot::Construct(const FChildren& SlotOwner, FSlotArguments&& InArgs)
{
	TBasicLayoutWidgetSlot<FSlot>::Construct(SlotOwner, MoveTemp(InArgs));
	if (InArgs._Column.IsSet())
	{
		ColumnParam = FMath::Max(0, InArgs._Column.GetValue());
	}
	if (InArgs._ColumnSpan.IsSet())
	{
		ColumnSpanParam = FMath::Max(1, InArgs._ColumnSpan.GetValue());
	}
	if (InArgs._Row.IsSet())
	{
		RowParam = FMath::Max(0, InArgs._Row.GetValue());
	}
	if (InArgs._RowSpan.IsSet())
	{
		RowSpanParam = FMath::Max(1, InArgs._RowSpan.GetValue());
	}
	if (InArgs._Nudge.IsSet())
	{
		NudgeParam = InArgs._Nudge.GetValue();
	}
}

SSemiUniformGridPanel::SSemiUniformGridPanel()
	: Children(this)
	, SlotPadding(*this, FMargin(0.0))
	, MinDesiredSlotWidth(*this, 0.f)
	, MinDesiredSlotHeight(*this, 0.f)
{
	SetCanTick(false);
}

SSemiUniformGridPanel::FSlot::FSlotArguments SSemiUniformGridPanel::Slot(int32 Column, int32 Row)
{
	return FSlot::FSlotArguments(MakeUnique<FSlot>(Column, Row));
}

void SSemiUniformGridPanel::Construct(const FArguments& InArgs)
{
	SlotPadding.Assign(*this, InArgs._SlotPadding);
	MinDesiredSlotWidth.Assign(*this, InArgs._MinDesiredSlotWidth);
	MinDesiredSlotHeight.Assign(*this, InArgs._MinDesiredSlotHeight);

	Children.AddSlots(MoveTemp(const_cast<TArray<FSlot::FSlotArguments>&>(InArgs._Slots)));
}

void SSemiUniformGridPanel::OnArrangeChildren(const FGeometry& AllottedGeometry,
	FArrangedChildren& ArrangedChildren) const
{
	if (Children.Num() > 0)
	{
		const FVector2D UnitCellSize(MinDesiredSlotWidth.Get(), MinDesiredSlotHeight.Get());
		const FMargin& CurrentSlotPadding(SlotPadding.Get());
		
		for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			const FSlot& Child = Children[ChildIndex];
			const EVisibility ChildVisibility = Child.GetWidget()->GetVisibility();
			if (ArrangedChildren.Accepts(ChildVisibility))
			{
				const int32 ColumnSpan = FMath::Max(Child.GetColumnSpan(), 1);
				const int32 RowSpan = FMath::Max(Child.GetRowSpan(), 1);
				
				AlignmentArrangeResult XAxisResult = AlignChild<Orient_Horizontal>(UnitCellSize.X * ColumnSpan, Child, CurrentSlotPadding);
				AlignmentArrangeResult YAxisResult = AlignChild<Orient_Vertical>(UnitCellSize.Y * RowSpan, Child, CurrentSlotPadding);
				
				ArrangedChildren.AddWidget(ChildVisibility,
					AllottedGeometry.MakeChild(Child.GetWidget(),
					FVector2D(UnitCellSize.X*Child.GetColumn() + XAxisResult.Offset, UnitCellSize.Y*Child.GetRow() + YAxisResult.Offset),
					FVector2D(XAxisResult.Size, YAxisResult.Size)
					));
			}
		}
	}
}

FChildren* SSemiUniformGridPanel::GetChildren()
{
	return &Children;
}

void SSemiUniformGridPanel::SetSlotPadding(TAttribute<FMargin> InSlotPadding)
{
	SlotPadding.Assign(*this, MoveTemp(InSlotPadding));
}

void SSemiUniformGridPanel::SetMinDesiredSlotWidth(TAttribute<float> InMinDesiredSlotWidth)
{
	MinDesiredSlotWidth.Assign(*this, MoveTemp(InMinDesiredSlotWidth));
}

void SSemiUniformGridPanel::SetMinDesiredSlotHeight(TAttribute<float> InMinDesiredSlotHeight)
{
	MinDesiredSlotHeight.Assign(*this, MoveTemp(InMinDesiredSlotHeight));
}

SSemiUniformGridPanel::FScopedWidgetSlotArguments SSemiUniformGridPanel::AddSlot(int32 Column, int32 Row)
{
	TWeakPtr<SSemiUniformGridPanel> WeakPanel = SharedThis(this);
	TUniquePtr<FSlot> NewSlot = MakeUnique<FSlot>(Column, Row);
	NewSlot->Panel = WeakPanel;
	
	return FScopedWidgetSlotArguments{ MoveTemp(NewSlot), Children, INDEX_NONE };
}

bool SSemiUniformGridPanel::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	return Children.Remove(SlotWidget) != INDEX_NONE;
}

void SSemiUniformGridPanel::ClearChildren()
{
	Children.Empty();
}

bool SSemiUniformGridPanel::FindNearestEmptySlot(const FVector2D& InLocalPos, int32 OutColumn, int32 OutRow) const
{
	const float CellW = MinDesiredSlotWidth.Get();
	const float CellH = MinDesiredSlotHeight.Get();

	// Initial guess
	int32 StartCol = FMath::Max(0, FMath::FloorToInt(InLocalPos.X / CellW));
	int32 StartRow = FMath::Max(0, FMath::FloorToInt(InLocalPos.Y / CellH));

	// A simple spiral search up to some max radius
	const int32 MaxDist = 25; // adjust to taste
	for (int32 d = 0; d <= MaxDist; ++d)
	{
		for (int32 dy = -d; dy <= d; ++dy)
		{
			for (int32 dx = -d; dx <= d; ++dx)
			{
				int32 TestCol = StartCol + dx;
				int32 TestRow = StartRow + dy;

				if (TestCol >= 0 && TestRow >= 0)
				{
					if (IsSlotAvailable(TestCol, TestRow))
					{
						OutColumn = TestCol;
						OutRow = TestRow;
						return true;
					}
				}
			}
		}
	}

	// If no empty found, fallback to (0,0)
	OutColumn = 0;
	OutRow = 0;
	return false;
}

bool SSemiUniformGridPanel::IsSlotAvailable(int32 Column, int32 Row) const
{
	for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
	{
		const FSlot& Child = Children[ChildIndex];
		int32 ColumnStart = Child.GetColumn();
		int32 ColumnEnd = ColumnStart + FMath::Max(Child.GetColumnSpan(), 1) - 1;
		int32 RowStart = Child.GetRow();
		int32 RowEnd = RowStart + FMath::Max(Child.GetRowSpan(), 1) - 1;

		bool bColumnOverlapped = (Column >= ColumnStart) && (Column <= ColumnEnd);
		bool bRowOverlapped = Row >= RowStart && Row <= RowEnd;
		if (bColumnOverlapped && bRowOverlapped)
		{
			return false;
		}
	}

	return true;
}

FVector2D SSemiUniformGridPanel::ComputeDesiredSize(float) const
{
	FVector2D MaxChildDesiredSize = FVector2D::ZeroVector;
	const FVector2D SlotPaddingDesiredSize = SlotPadding.Get().GetDesiredSize();
	
	const float CachedMinDesiredSlotWidth = MinDesiredSlotWidth.Get();
	const float CachedMinDesiredSlotHeight = MinDesiredSlotHeight.Get();
	
	int32 NumColumns = 0;
	int32 NumRows = 0;

	for ( int32 ChildIndex=0; ChildIndex < Children.Num(); ++ChildIndex )
	{
		const FSlot& Child = Children[ ChildIndex ];

		if (Child.GetWidget()->GetVisibility() != EVisibility::Collapsed)
		{
			int32 ColumnSpan = FMath::Max(Child.GetColumnSpan(),1);
			int32 RowSpan = FMath::Max(Child.GetRowSpan(), 1);
			
			// A single cell at (N,M) means our grid size is (N+1, M+1)
			// Span expands grid size when it's greater than 1
			NumColumns = FMath::Max(Child.GetColumn() + 1 + (ColumnSpan -1), NumColumns);
			NumRows = FMath::Max(Child.GetRow() + 1 + (RowSpan -1), NumRows);
			
			FVector2D ChildDesiredSize = Child.GetWidget()->GetDesiredSize() + SlotPaddingDesiredSize;

			ChildDesiredSize.X = FMath::Max( ChildDesiredSize.X, CachedMinDesiredSlotWidth);
			ChildDesiredSize.Y = FMath::Max( ChildDesiredSize.Y, CachedMinDesiredSlotHeight);

			MaxChildDesiredSize.X = FMath::Max( MaxChildDesiredSize.X, ChildDesiredSize.X );
			MaxChildDesiredSize.Y = FMath::Max( MaxChildDesiredSize.Y, ChildDesiredSize.Y );
		}
	}

	return FVector2D( NumColumns*MaxChildDesiredSize.X, NumRows*MaxChildDesiredSize.Y );
}

void SSemiUniformGridPanel::NotifySlotChanged(const FSlot* InSlot)
{
	Invalidate(EInvalidateWidgetReason::Layout);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
