// the.quiet.string@gmail.com


#include "SemiUniformGridSlotExtension.h"

#include "Components/Widget.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Layout/Margin.h"
#include "Math/UnrealMathSSE.h"
#include "Math/Vector2D.h"
#include "Misc/Attribute.h"
#include "Templates/Casts.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "WidgetBlueprint.h"
#include "WidgetReference.h"
#include "UI/Foundation/SemiUniformGridPanel.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SButton.h"
#include "UI/Foundation/SemiUniformGridSlot.h"

#define LOCTEXT_NAMESPACE "UMG"

FSemiUniformGridSlotExtension::FSemiUniformGridSlotExtension()
{
	ExtensionId = FName(TEXT("SemiUniformGridSlot"));
}

bool FSemiUniformGridSlotExtension::CanExtendSelection(const TArray<FWidgetReference>& Selection) const
{
	for ( const FWidgetReference& Widget : Selection )
	{
		if ( !Widget.GetTemplate()->Slot || !Widget.GetTemplate()->Slot->IsA(USemiUniformGridSlot::StaticClass()) )
		{
			return false;
		}
	}

	return Selection.Num() == 1;}

void FSemiUniformGridSlotExtension::ExtendSelection(const TArray<FWidgetReference>& Selection,
	TArray<TSharedRef<FDesignerSurfaceElement>>& SurfaceElements)
{
	SelectionCache = Selection;

	TSharedRef<SButton> UpArrow = SNew(SButton)
		.Text(LOCTEXT("UpArrow", "\u2191"))
		.ContentPadding(FMargin(6, 2))
		.OnClicked(this, &FSemiUniformGridSlotExtension::HandleShiftRow, -1);
	
	TSharedRef<SButton> DownArrow = SNew(SButton)
		.Text(LOCTEXT("DownArrow", "\u2193"))
		.ContentPadding(FMargin(6, 2))
		.OnClicked(this, &FSemiUniformGridSlotExtension::HandleShiftRow, 1);
	
	TSharedRef<SButton> LeftArrow = SNew(SButton)
		.Text(LOCTEXT("LeftArrow", "\u2190"))
		.ContentPadding(FMargin(2, 6))
		.OnClicked(this, &FSemiUniformGridSlotExtension::HandleShiftColumn, -1);

	TSharedRef<SButton> RightArrow = SNew(SButton)
		.Text(LOCTEXT("RightArrow", "\u2192"))
		.ContentPadding(FMargin(2, 6))
		.OnClicked(this, &FSemiUniformGridSlotExtension::HandleShiftColumn, 1);

	UpArrow->SlatePrepass();
	DownArrow->SlatePrepass();
	LeftArrow->SlatePrepass();
	RightArrow->SlatePrepass();
	
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(LeftArrow, EExtensionLayoutLocation::CenterLeft, FVector2D(-LeftArrow->GetDesiredSize().X, LeftArrow->GetDesiredSize().Y * -0.5f))));
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(RightArrow, EExtensionLayoutLocation::CenterRight, FVector2D(0, RightArrow->GetDesiredSize().Y * -0.5f))));
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(UpArrow, EExtensionLayoutLocation::TopCenter, FVector2D(UpArrow->GetDesiredSize().X * -0.5f, -UpArrow->GetDesiredSize().Y))));
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(DownArrow, EExtensionLayoutLocation::BottomCenter, FVector2D(DownArrow->GetDesiredSize().X * -0.5f, 0))));

	TSharedRef<SButton> UpArrow_Span = SNew(SButton)
			.Text(LOCTEXT("UpArrow_Span", "\u21A5"))
			.ContentPadding(FMargin(6, 2))
			.OnClicked(this, &FSemiUniformGridSlotExtension::HandleExpandRow, -1);

	TSharedRef<SButton> DownArrow_Span = SNew(SButton)
		.Text(LOCTEXT("DownArrow_Span", "\u21A7"))
		.ContentPadding(FMargin(6, 2))
		.OnClicked(this, &FSemiUniformGridSlotExtension::HandleExpandRow, 1);

	TSharedRef<SButton> LeftArrow_Span = SNew(SButton)
			.Text(LOCTEXT("LeftArrow_Span", "\u21A4"))
			.ContentPadding(FMargin(2, 6))
			.OnClicked(this, &FSemiUniformGridSlotExtension::HandleExpandColumn, -1);

	TSharedRef<SButton> RightArrow_Span = SNew(SButton)
		.Text(LOCTEXT("RightArrow_Span", "\u21A6"))
		.ContentPadding(FMargin(2, 6))
		.OnClicked(this, &FSemiUniformGridSlotExtension::HandleExpandColumn, 1);
	
	UpArrow_Span->SlatePrepass();
	DownArrow_Span->SlatePrepass();
	LeftArrow_Span->SlatePrepass();
	RightArrow_Span->SlatePrepass();
	
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(UpArrow_Span, EExtensionLayoutLocation::TopCenter, FVector2D(UpArrow->GetDesiredSize().X * -0.5f, -UpArrow->GetDesiredSize().Y * 3))));
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(DownArrow_Span, EExtensionLayoutLocation::BottomCenter, FVector2D(DownArrow->GetDesiredSize().X * -0.5f, DownArrow->GetDesiredSize().Y * 2))));
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(LeftArrow_Span, EExtensionLayoutLocation::CenterLeft, FVector2D(-LeftArrow->GetDesiredSize().X * 3, LeftArrow_Span->GetDesiredSize().Y * -0.5f))));
	SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(RightArrow_Span, EExtensionLayoutLocation::CenterRight, FVector2D(RightArrow->GetDesiredSize().X * 2, RightArrow_Span->GetDesiredSize().Y * -0.5f))));
}

FReply FSemiUniformGridSlotExtension::HandleShiftRow(int32 ShiftAmount)
{
	BeginTransaction(LOCTEXT("MoveWidget", "Move Widget"));

	for ( FWidgetReference& Selection : SelectionCache )
	{
		ShiftRow(Selection.GetPreview(), ShiftAmount);
		ShiftRow(Selection.GetTemplate(), ShiftAmount);
	}

	EndTransaction();

	if (UWidgetBlueprint* BlueprintPtr = Blueprint.Get())
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr);
	}

	return FReply::Handled();
}

FReply FSemiUniformGridSlotExtension::HandleShiftColumn(int32 ShiftAmount)
{
	BeginTransaction(LOCTEXT("MoveWidget", "Move Widget"));

	for ( FWidgetReference& Selection : SelectionCache )
	{
		UWidget* Preview = Selection.GetPreview();
		UWidget* Template = Selection.GetTemplate();
		if (!Preview)
		{
			UE_LOG(LogTemp, Warning, TEXT("no preview"));
		}
		if (!Template)
		{
			UE_LOG(LogTemp, Warning, TEXT("no template"));
		}
		ShiftColumn(Selection.GetPreview(), ShiftAmount);
		ShiftColumn(Selection.GetTemplate(), ShiftAmount);
	}

	EndTransaction();

	if (UWidgetBlueprint* BlueprintPtr = Blueprint.Get())
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr);
	}

	return FReply::Handled();
}

void FSemiUniformGridSlotExtension::ShiftRow(UWidget* Widget, int32 ShiftAmount)
{
	USemiUniformGridSlot* Slot = Cast<USemiUniformGridSlot>(Widget->Slot);
	Slot->SetRow(FMath::Max(Slot->GetRow() + ShiftAmount, 0));
}

void FSemiUniformGridSlotExtension::ShiftColumn(UWidget* Widget, int32 ShiftAmount)
{
	USemiUniformGridSlot* Slot = Cast<USemiUniformGridSlot>(Widget->Slot);
	Slot->SetColumn(FMath::Max(Slot->GetColumn() + ShiftAmount, 0));
}

FReply FSemiUniformGridSlotExtension::HandleExpandRow(int32 ExpandAmount)
{
	BeginTransaction(LOCTEXT("ExpandWidget", "Expand Widget"));

	for ( FWidgetReference& Selection : SelectionCache )
	{
		ExpandRow(Selection.GetPreview(), ExpandAmount);
		if (USemiUniformGridSlot* TemplateSlot = Cast<USemiUniformGridSlot>(Selection.GetTemplate()->Slot))
		{
			TemplateSlot->SynchronizeFromPreview(Selection.GetPreview()->Slot);
		}
	}

	EndTransaction();

	if (UWidgetBlueprint* BlueprintPtr = Blueprint.Get())
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr);
	}

	return FReply::Handled();
}

FReply FSemiUniformGridSlotExtension::HandleExpandColumn(int32 ExpandAmount)
{
	BeginTransaction(LOCTEXT("ExpandWidget", "Expand Widget"));
	
	for ( FWidgetReference& Selection : SelectionCache )
	{
		ExpandColumn(Selection.GetPreview(), ExpandAmount);
		if (USemiUniformGridSlot* TemplateSlot = Cast<USemiUniformGridSlot>(Selection.GetTemplate()->Slot))
		{
			TemplateSlot->SynchronizeFromPreview(Selection.GetPreview()->Slot);
		}
	}
	
	EndTransaction();

	if (UWidgetBlueprint* BlueprintPtr = Blueprint.Get())
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintPtr);
	}

	return FReply::Handled();
}

void FSemiUniformGridSlotExtension::ExpandRow(UWidget* Widget, int32 ExpandAmount)
{
	USemiUniformGridSlot* Slot = Cast<USemiUniformGridSlot>(Widget->Slot);
	int32 AbsExpandAmount = FMath::Abs(ExpandAmount);
	
	if (USemiUniformGridPanel* Panel = Cast<USemiUniformGridPanel>(Slot->Parent))
	{
		int32 WantedRow = FMath::Max(Slot->GetRow() + Slot->GetRowSpan() -1 + ExpandAmount, 0);

		if (Panel->IsSlotAvailable(Slot->GetColumn(), WantedRow))
		{
			if (ExpandAmount > 0)
			{
				Slot->SetRowSpan(FMath::Max(Slot->GetRowSpan() + ExpandAmount, 1));
			}
			else // We should move slot when expanding upwards.
			{
				Slot->SetRow(WantedRow);
				Slot->SetRowSpan(Slot->GetRowSpan() + AbsExpandAmount);
			}
		}
		else
		{
			if (ExpandAmount < 0)
			{
				int32 CurRowSpan = Slot->GetRowSpan();
				if (CurRowSpan > 1)
				{
					Slot->SetRowSpan(CurRowSpan - AbsExpandAmount);
				}
			}
		}
	}
}

void FSemiUniformGridSlotExtension::ExpandColumn(UWidget* Widget, int32 ExpandAmount)
{
	USemiUniformGridSlot* Slot = Cast<USemiUniformGridSlot>(Widget->Slot);
	int32 AbsExpandAmount = FMath::Abs(ExpandAmount);
	
	if (USemiUniformGridPanel* Panel = Cast<USemiUniformGridPanel>(Slot->Parent))
	{
		int32 WantedColumn = FMath::Max(Slot->GetColumn() + Slot->GetColumnSpan() -1 + ExpandAmount, 0);

		if (Panel->IsSlotAvailable(WantedColumn, Slot->GetRow()))
		{
			if (ExpandAmount > 0)
			{
				Slot->SetColumnSpan(FMath::Max(Slot->GetColumnSpan() + ExpandAmount, 1));
			}
			else // We should move slot when expanding towards left.
			{
				Slot->SetColumn(WantedColumn);
				Slot->SetColumnSpan(Slot->GetColumnSpan() + AbsExpandAmount);
			}
		}
		else
		{
			if (ExpandAmount < 0)
			{
				int32 CurColumnSpan = Slot->GetColumnSpan();
				if (CurColumnSpan > 1)
				{
					Slot->SetColumnSpan(CurColumnSpan - AbsExpandAmount);
				}
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE