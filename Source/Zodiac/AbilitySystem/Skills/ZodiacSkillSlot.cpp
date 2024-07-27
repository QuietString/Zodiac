// the.quiet.string@gmail.com


#include "ZodiacSkillSlot.h"

#include "ZodiacSkillSlotDefinition.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/ZodiacSkillSlotWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillSlot)

UZodiacSkillSlot::UZodiacSkillSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacSkillSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTag);
	DOREPLIFETIME_CONDITION(ThisClass, Definition, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, GrantedHandles, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, SlotType, COND_InitialOnly);
}

void UZodiacSkillSlot::InitializeSlot(const UZodiacSkillSlotDefinition* InDef, FGameplayTag InType)
{
	check(InDef);
	SlotType = InType;
	Definition = InDef;

	StatTag.OnStackChanged.BindUObject(this, &ThisClass::OnTagStackChanged_Internal);
}

void UZodiacSkillSlot::CreateSlotWidget()
{
	if (Definition->SlotWidgetClass)
	{
		if (const ACharacter* Char = Cast<ACharacter>(GetOuter()))
		{
			if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
			{
				Widget = CreateWidget<UZodiacSkillSlotWidgetBase>(PC, Definition->SlotWidgetClass);
			}
		}	
	}
}

UUserWidget* UZodiacSkillSlot::GetSlotWidget()
{
	return Widget;
}

int32 UZodiacSkillSlot::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTag.GetStackCount(Tag);
}

void UZodiacSkillSlot::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.AddStack(Tag, StackCount);
}

void UZodiacSkillSlot::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.RemoveStack(Tag, StackCount);
}

void UZodiacSkillSlot::OnRep_StatTag()
{
}

void UZodiacSkillSlot::OnTagStackChanged_Internal(const FGameplayTag Tag, const int32 OldValue, const int32 NewValue)
{
	if (Widget)
	{
		Widget->OnTagStackChanged.Broadcast(Tag, OldValue, NewValue);
	}
}

void UZodiacSkillSlot::OnRep_Definition()
{
	CreateSlotWidget();
	StatTag.OnStackChanged.BindUObject(this, &ThisClass::OnTagStackChanged_Internal);
}