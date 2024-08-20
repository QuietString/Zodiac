// the.quiet.string@gmail.com


#include "ZodiacMonster.h"

#include "ZodiacAIController.h"
#include "AbilitySystem/ZodiacAbilitySet.h"

AZodiacMonster::AZodiacMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilitySystemComponent* AZodiacMonster::GetAbilitySystemComponent() const
{
	if (AZodiacAIController* ZodiacAC = GetController<AZodiacAIController>())
	{
		return ZodiacAC->GetAbilitySystemComponent();
	}

	return nullptr;
}

void AZodiacMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AZodiacAIController* ZodiacAC = Cast<AZodiacAIController>(NewController))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
		{
			InitializeAbilitySystem(ZodiacASC, ZodiacAC);
		}
	}
}

void AZodiacMonster::InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner)
{
	Super::InitializeAbilitySystem(InASC, InOwner);

	if (HeroData && HasAuthority())
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);	
			}
		}
	}
}
