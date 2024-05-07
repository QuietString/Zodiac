// the.quiet.string@gmail.com

#include "ZodiacAbilitySet.h"

#include "ZodiacAbilitySystemComponent.h"
#include "ZodiacLogChannels.h"
#include "Abilities/ZodiacGameplayAbility.h"
#include "Attributes/ZodiacCombatSet.h"

void FZodiacAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FZodiacAbilitySet_GrantedHandles::AddSkillHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		SkillHandles.Add(Handle);
	}
}

void FZodiacAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

UZodiacAbilitySet::UZodiacAbilitySet(const FObjectInitializer& ObjectInitializer)
{
}

void UZodiacAbilitySet::GiveToAbilitySystem(UZodiacAbilitySystemComponent* ZodiacASC,
                                            FZodiacAbilitySet_GrantedHandles* OutGrantedHandles, FZodiacSkillSetWithHandle* OutSkillData, UObject* SourceObject) const
{
	check(ZodiacASC);

	if (!ZodiacASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FZodiacAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogZodiacAbilitySystem, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		UZodiacGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UZodiacGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = ZodiacASC->GiveAbility(AbilitySpec);
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
	
	// Grant the skill abilities.
	for (int32 SkillIndex = 0; SkillIndex < GrantedSkillAbilities.Num(); ++SkillIndex)
	{
		const FZodiacSkillSet& SkillToGrant = GrantedSkillAbilities[SkillIndex];

		if (!IsValid(SkillToGrant.Ability))
		{
			UE_LOG(LogZodiacAbilitySystem, Error, TEXT("GrantedSkillAbilities[%d] on ability set [%s] is not valid."), SkillIndex, *GetNameSafe(this));
			continue;
		}

		if (!SkillToGrant.SkillType.IsValid())
		{
			UE_LOG(LogZodiacAbilitySystem, Error, TEXT("GrantedSkillAbilities[%d] on ability set [%s] has no skill type."), SkillIndex, *GetNameSafe(this));
			continue;;
		}
		
		UZodiacGameplayAbility* AbilityCDO = SkillToGrant.Ability->GetDefaultObject<UZodiacGameplayAbility>();
		AbilityCDO->AbilityTags.AddTag(SkillToGrant.SkillType);
		
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, SkillToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.DynamicAbilityTags.AddTag(SkillToGrant.InputTag);
		
		const FGameplayAbilitySpecHandle AbilitySpecHandle = ZodiacASC->GiveAbility(AbilitySpec);

		if (OutSkillData)
		{
			OutSkillData->Map.Add(AbilitySpecHandle, &GrantedSkillAbilities[SkillIndex]);	
		}

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddSkillHandle(AbilitySpecHandle);
		}
	}
	
	// Grant the attribute sets.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FZodiacAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogZodiacAbilitySystem, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(ZodiacASC->GetOwner(), SetToGrant.AttributeSet);
		ZodiacASC->AddAttributeSetSubobject(NewSet);
		
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
}
