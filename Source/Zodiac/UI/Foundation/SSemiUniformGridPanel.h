// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Layout/Visibility.h"
#include "SlotBase.h"
#include "Layout/Margin.h"
#include "Widgets/SWidget.h"
#include "Layout/Children.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SPanel.h"

class FArrangedChildren;

/**
 * 
 */
class SSemiUniformGridPanel : public SPanel
{
	SLATE_DECLARE_WIDGET_API(SSemiUniformGridPanel, SPanel, ZODIAC_API)

public:
	/** Stores the per-child info for this panel type */
	class FSlot : public TBasicLayoutWidgetSlot<FSlot>
	{
		friend SSemiUniformGridPanel;

		public:
			/** Default values for a slot. */
			FSlot(int32 Column, int32 Row)
				: TBasicLayoutWidgetSlot<FSlot>(HAlign_Fill, VAlign_Fill)
				, ColumnParam( Column )
				, ColumnSpanParam(1)
				, RowParam( Row )
				, RowSpanParam(1)
				, NudgeParam( FVector2D::ZeroVector )
			{
			}

			SLATE_SLOT_BEGIN_ARGS(FSlot, TBasicLayoutWidgetSlot<FSlot>)
				/** Which column in the grid this cell belongs to */
				SLATE_ARGUMENT(TOptional<int32>, Column)
				/** How many columns this slot spans over */
				SLATE_ARGUMENT(TOptional<int32>, ColumnSpan)
				/** Which row in the grid this cell belongs to */
				SLATE_ARGUMENT(TOptional<int32>, Row)
				/** How many rows this this slot spans over */
				SLATE_ARGUMENT(TOptional<int32>, RowSpan)
				/** Offset this slot's content by some amount; positive values offset to lower right*/
				SLATE_ARGUMENT(TOptional<FVector2D>, Nudge)
			SLATE_SLOT_END_ARGS()

			void Construct(const FChildren& SlotOwner, FSlotArguments&& InArgs);

		public:
			/** Which column in the grid this cell belongs to */
			int32 GetColumn() const
			{
				return ColumnParam;
			}

			void SetColumn(int32 Column)
			{
				Column = FMath::Max(0, Column);
				if (Column != ColumnParam)
				{
					ColumnParam = Column;
					NotifySlotChanged();
				}
			}

			/** How many columns this slot spans over */
			int32 GetColumnSpan() const
			{
				return ColumnSpanParam;
			}

			void SetColumnSpan(int32 ColumnSpan)
			{
				// clamp span to a sensible size, otherwise computing slot sizes can slow down dramatically
				ColumnSpan = FMath::Clamp(ColumnSpan, 1, 100);
				if (ColumnSpan != ColumnSpanParam)
				{
					ColumnSpanParam = ColumnSpan;
					NotifySlotChanged();
				}
			}

			/** Which row in the grid this cell belongs to */
			int32 GetRow() const
			{
				return RowParam;
			}

			void SetRow(int32 Row)
			{
				Row = FMath::Max(0, Row);
				if (Row != RowParam)
				{
					RowParam = Row;
					NotifySlotChanged();
				}
			}

			/** How many rows this this slot spans over */
			int32 GetRowSpan() const
			{
				return RowSpanParam;
			}

			void SetRowSpan(int32 RowSpan)
			{
				// clamp span to a sensible size, otherwise computing slots sizes can slow down dramatically
				RowSpan = FMath::Clamp(RowSpan, 1, 100);
				if (RowSpan != RowSpanParam)
				{
					RowSpanParam = RowSpan;
					NotifySlotChanged();
				}
			}
		
			/** Offset this slot's content by some amount; positive values offset to lower right */
			FVector2D GetNudge() const
			{
				return NudgeParam;
			}

			void SetNudge(const FVector2D& Nudge)
			{
				NudgeParam = Nudge;
				Invalidate(EInvalidateWidgetReason::Paint);
			}

		private:
			/** The panel that contains this slot */
			TWeakPtr<SSemiUniformGridPanel> Panel;

			int32 ColumnParam;
			int32 ColumnSpanParam;
			int32 RowParam;
			int32 RowSpanParam;
			FVector2D NudgeParam;

			/** Notify that the slot was changed */
			FORCEINLINE void NotifySlotChanged(bool bSlotLayerChanged = false)
			{
				if (Panel.IsValid())
				{
					Panel.Pin()->NotifySlotChanged(this);
				}
			}
	};

	SSemiUniformGridPanel();

	/**
	 * Used by declarative syntax to create a Slot in the specified Column, Row.
	 */
	static FSlot::FSlotArguments Slot(int32 Column, int32 Row);
	
	SLATE_BEGIN_ARGS(SSemiUniformGridPanel)
		: _SlotPadding(FMargin(0.0f))
		, _MinDesiredSlotWidth(0.0f)
		, _MinDesiredSlotHeight(0.0f)
		{
			_Visibility = EVisibility::SelfHitTestInvisible;
		}

		/** Slot type supported by this panel */
		SLATE_SLOT_ARGUMENT(FSlot, Slots)

		/** Padding given to each slot */
		SLATE_ATTRIBUTE(FMargin, SlotPadding)

		/** The minimum desired width of the slots */
		SLATE_ATTRIBUTE(float, MinDesiredSlotWidth)

		/** The minimum desired height of the slots */
		SLATE_ATTRIBUTE(float, MinDesiredSlotHeight)
	
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	//~ Begin SPanel Interface	
	virtual void OnArrangeChildren( const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren ) const override;
	virtual FChildren* GetChildren() override;
	//~ End SPanel Interface
	
	/** See SlotPadding attribute */
	void SetSlotPadding(TAttribute<FMargin> InSlotPadding);

	void SetMinDesiredSlotWidth(TAttribute<float> InMinDesiredSlotWidth);
	void SetMinDesiredSlotHeight(TAttribute<float> InMinDesiredSlotHeight);

	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;

	FScopedWidgetSlotArguments AddSlot(int32 Column, int32 Row);

	bool RemoveSlot( const TSharedRef<SWidget>& SlotWidget );

	/** Removes all slots from the panel */
	void ClearChildren();

	bool IsSlotAvailable(int32 Column, int32 Row);
	
protected:
	// Begin SWidget overrides.
	virtual FVector2D ComputeDesiredSize(float) const override;
	// End SWidget overrides.

	void NotifySlotChanged(const FSlot* InSlot);
	
private:
	TPanelChildren<FSlot> Children;
	TSlateAttribute<FMargin, EInvalidateWidgetReason::Layout> SlotPadding;
	
	TSlateAttribute<float, EInvalidateWidgetReason::Layout> MinDesiredSlotWidth;
	TSlateAttribute<float, EInvalidateWidgetReason::Layout> MinDesiredSlotHeight;
};
