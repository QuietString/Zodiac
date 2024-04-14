// the.quiet.string@gmail.com


#include "ZodiacAbilitySet.h"

#include "ZodiacAbilitySystemComponent.h"
#include "ZodiacLogChannels.h"
#include "Attributes/ZodiacCombatSet.h"

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
