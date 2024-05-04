// the.quiet.string@gmail.com


#include "ZodiacHealthBarWidget.h"

#include "Character/ZodiacHealthComponent.h"
#include "Character/ZodiacHeroComponent.h"
#include "Character/ZodiacPlayerCharacter.h"



void UZodiacHealthBarWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	Super::OnPossessedPawnChanged(OldPawn, NewPawn);
	
	if (AZodiacPlayerCharacter* ZodiacCharacter = Cast<AZodiacPlayerCharacter>(NewPawn))
	{
		PlayerCharacter = ZodiacCharacter;

		TArray<UZodiacHealthComponent*> HealthComponents;
		PlayerCharacter->GetComponents(UZodiacHealthComponent::StaticClass(), HealthComponents);
		if (HealthComponents.Num() > 0)
		{
			for (auto& HealthComponent : HealthComponents)
			{
				HealthComponent->OnHealthChanged.AddDynamic(this, &ThisClass::HandleHealthChanged);
			}
		}

		UZodiacHeroComponent* CurrentHeroComponent = PlayerCharacter->GetCurrentHeroComponent();
		if (CurrentHeroComponent)
		{
			// Initialize health visualization
			HandleHeroChanged(PlayerCharacter->GetCurrentHeroComponent());
		}
	}
}

void UZodiacHealthBarWidget::HandleHeroChanged(UZodiacHeroComponent* HeroComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("handle hero change is called on %s"), PlayerCharacter->HasAuthority() ? TEXT("server") : TEXT("client"));
	if (UZodiacHealthComponent* HealthComponent = HeroComponent->GetHealthComponent())
	{
		float InCurrentHealth;
		float InCurrentMaxHealth;
		HealthComponent->GetCurrentHealth(InCurrentHealth, InCurrentMaxHealth);
		OnHeroChanged(InCurrentHealth, InCurrentMaxHealth);
		CurrentMaxHealth = InCurrentMaxHealth;
	}
}

void UZodiacHealthBarWidget::HandleHealthChanged(UZodiacHealthComponent* HealthComponent, float OldValue, float NewValue,
                                                 AActor* Instigator)
{
	// Apply health change to this UI only when the change is happened on the current Hero.
	if (PlayerCharacter->GetCurrentHealthComponent() == HealthComponent)
	{
		OnHealthChanged(OldValue, NewValue, CurrentMaxHealth);
	}
}