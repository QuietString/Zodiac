// the.quiet.string@gmail.com


#include "ZodiacHeroAbility_Reload.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/Hero/Slot/ZodiacHeroAbilitySlot_Weapon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbility_Reload)

bool UZodiacHeroAbility_Reload::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if(!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (UZodiacHeroAbilitySlot_Weapon* Slot = GetAssociatedSlot<UZodiacHeroAbilitySlot_Weapon>())
	{
		int32 CurrentAmmo = Slot->GetStatTagStackCount(ZodiacGameplayTags::Ability_Cost_Stack_MagazineAmmo);
		int32 MaxAmmo = Slot->GetStatTagStackCount(ZodiacGameplayTags::Ability_Cost_Stack_MagazineSize);
		return (CurrentAmmo < MaxAmmo);
	}
	
	return true;
}
