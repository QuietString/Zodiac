// the.quiet.string@gmail.com

#include "ZodiacAbilityCost_TagStack.h"

#include "NativeGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Hero/Abilities/ZodiacHeroAbility.h"
#include "AbilitySystem/Hero/Slot/ZodiacHeroAbilitySlot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilityCost_TagStack)

UZodiacAbilityCost_TagStack::UZodiacAbilityCost_TagStack()
{
	Quantity.SetValue(1.0f);
	//FailureTag = TAG_ABILITY_FAIL_COST;
}

bool UZodiacAbilityCost_TagStack::CheckCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Ability)
	{
		if (const UZodiacHeroAbility* HeroAbility = Cast<UZodiacHeroAbility>(Ability))
		{
			FScalableFloat QuantityToUse = HeroAbility->ShouldUseInitialCost() ? Quantity_Initial : Quantity;
			const float QuantityReal = QuantityToUse.GetValueAtLevel(1);
			const int32 StackCost = FMath::TruncToInt(QuantityReal);

			int32 StacksFound = 0;
			
			if (UZodiacHeroAbilitySlot* AbilitySlot = HeroAbility->GetAssociatedSlot())
			{
				StacksFound = AbilitySlot->GetStatTagStackCount(Tag);
			}

			const bool bCanApplyCost = StacksFound >= StackCost;
		
			// Inform other abilities why this cost cannot be applied
			if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);				
			}
			return bCanApplyCost;
		}
	}
	
	return false;
}

void UZodiacAbilityCost_TagStack::ApplyCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority() && Ability)
	{
		if (const UZodiacHeroAbility* HeroAbility = Cast<UZodiacHeroAbility>(Ability))
		{
			FScalableFloat QuantityToUse = HeroAbility->ShouldUseInitialCost() ? Quantity_Initial : Quantity;
			const float QuantityReal = QuantityToUse.GetValueAtLevel(0);
			const int32 StackCost = FMath::TruncToInt(QuantityReal);
	
			if (UZodiacHeroAbilitySlot* AbilitySlot = HeroAbility->GetAssociatedSlot())
			{
				AbilitySlot->RemoveStatTagStack(Tag, StackCost);		
			}
		}
	}
}