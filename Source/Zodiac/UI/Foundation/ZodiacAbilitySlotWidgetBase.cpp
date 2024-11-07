// the.quiet.string@gmail.com


#include "ZodiacAbilitySlotWidgetBase.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilitySlotWidgetBase)

UZodiacAbilitySlotWidgetBase::UZodiacAbilitySlotWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacAbilitySlotWidgetBase::InitializeFromAbilitySlot(UZodiacHeroAbilitySlot* InSlot)
{
	AbilitySlot = InSlot;
}

void UZodiacAbilitySlotWidgetBase::OnHeroChanged_Implementation()
{
}
