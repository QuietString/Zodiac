// the.quiet.string@gmail.com


#include "ZodiacHeroItemSlot.h"

#include "Net/UnrealNetwork.h"
#include "UI/HUD/ZodiacSkillSlotWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroItemSlot)


UZodiacHeroItemSlot::UZodiacHeroItemSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacHeroItemSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTag);
	DOREPLIFETIME_CONDITION(ThisClass, GrantedHandles, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, SlotType, COND_InitialOnly);
}

void UZodiacHeroItemSlot::InitializeItem(const FZodiacHeroItemDefinition& InData)
{
	SlotType = InData.SlotType;
	Definition = InData;
}

int32 UZodiacHeroItemSlot::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTag.GetStackCount(Tag);
}

void UZodiacHeroItemSlot::SetStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.SetStack(Tag, StackCount);
}

void UZodiacHeroItemSlot::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.AddStack(Tag, StackCount);
}

void UZodiacHeroItemSlot::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.RemoveStack(Tag, StackCount);
}
