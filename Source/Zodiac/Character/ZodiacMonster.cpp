// the.quiet.string@gmail.com


#include "ZodiacMonster.h"

#include "ZodiacAIController.h"
#include "ZodiacHealthComponent.h"
#include "AbilitySystem/ZodiacAbilitySet.h"

AZodiacMonster::AZodiacMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
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
		if (UZodiacAbilitySystemComponent* ZodiacASC = ZodiacAC->GetZodiacAbilitySystemComponent())
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

	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}
