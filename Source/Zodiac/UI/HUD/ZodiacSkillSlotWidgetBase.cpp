// the.quiet.string@gmail.com


#include "ZodiacSkillSlotWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillSlotWidgetBase)

bool UZodiacSkillSlotWidgetBase::IsMessageForThisWidget(const APawn* Instigator, const int32 InHeroIndex) const
{
	APawn* Pawn = GetOwningPlayerPawn();
	return (Pawn == Instigator) && (InHeroIndex == HeroIndex);
}

bool UZodiacSkillSlotWidgetBase::IsMessageForThisWidgetWithSlotType(const APawn* Instigator, const int32 InHeroIndex,
                                                        const FGameplayTag InSlotType) const
{
	APawn* Pawn = GetOwningPlayerPawn();
	return (Pawn == Instigator) && (InHeroIndex == HeroIndex) && (SlotType == InSlotType);
}
