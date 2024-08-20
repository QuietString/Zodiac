// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacAbilitySet.h"

#include "ZodiacAbilitySystemComponent.h"
#include "ZodiacLogChannels.h"
#include "Abilities/ZodiacGameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilitySet)

void FZodiacAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
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
                                            FZodiacAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
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
	
	// Grant the attribute sets.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FZodiacAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet) || !IsValid(SetToGrant.Table))
		{
			UE_LOG(LogZodiacAbilitySystem, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(ZodiacASC->GetOwner(), SetToGrant.AttributeSet);
		ZodiacASC->AddAttributeSetSubobject(NewSet);
		NewSet->InitFromMetaDataTable(SetToGrant.Table);
		
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
}
