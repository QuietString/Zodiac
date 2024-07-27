// the.quiet.string@gmail.com


#include "ZodiacHealthBarWidget.h"

#include "Character/ZodiacHealthComponent.h"
//#include "Character/ZodiacHeroComponent.h"
#include "Character/ZodiacHostCharacter.h"



void UZodiacHealthBarWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	Super::OnPossessedPawnChanged(OldPawn, NewPawn);
	
	if (AZodiacHostCharacter* ZodiacCharacter = Cast<AZodiacHostCharacter>(NewPawn))
	{
		HostCharacter = ZodiacCharacter;

		TArray<UZodiacHealthComponent*> HealthComponents;
		HostCharacter->GetComponents(UZodiacHealthComponent::StaticClass(), HealthComponents);
		if (HealthComponents.Num() > 0)
		{
			for (auto& HealthComponent : HealthComponents)
			{
				//HealthComponent->OnHealthChanged.AddDynamic(this, &ThisClass::HandleHealthChanged);
			}
		}

		// UZodiacHeroComponent* CurrentHeroComponent = HostCharacter->GetCurrentHeroComponent();
		// if (CurrentHeroComponent)
		// {
		// 	// Initialize health visualization
		// 	HandleHeroChanged(HostCharacter->GetCurrentHeroComponent());
		// }
	}
}

// void UZodiacHealthBarWidget::HandleHeroChanged(UZodiacHeroComponent* HeroComponent)
// {
// 	if (UZodiacHealthComponent* HealthComponent = HeroComponent->GetHealthComponent())
// 	{
// 		// float InCurrentHealth;
// 		// float InCurrentMaxHealth;
// 		// HealthComponent->GetCurrentHealth(InCurrentHealth, InCurrentMaxHealth);
// 		// OnHeroChanged(InCurrentHealth, InCurrentMaxHealth);
// 		// CurrentMaxHealth = InCurrentMaxHealth;
// 	}
// }

// void UZodiacHealthBarWidget::HandleHealthChanged(UZodiacHealthComponent* HealthComponent, float OldValue, float NewValue,
//                                                  AActor* Instigator)
// {
// 	// Apply health change to this UI only when the change is happened on the current Hero.
// 	if (HostCharacter->GetCurrentHealthComponent() == HealthComponent)
// 	{
// 		OnHealthChanged(OldValue, NewValue, CurrentMaxHealth);
// 	}
// }