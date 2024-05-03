// the.quiet.string@gmail.com


#include "ZodiacHealthBarWidget.h"

#include "Character/ZodiacHealthComponent.h"
#include "Character/ZodiacHeroComponent.h"
#include "Character/ZodiacPlayerCharacter.h"

void UZodiacHealthBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	}
}

void UZodiacHealthBarWidget::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (AZodiacPlayerCharacter* ZodiacCharacter = Cast<AZodiacPlayerCharacter>(NewPawn))
	{
		PlayerCharacter = ZodiacCharacter;

		TArray<UZodiacHeroComponent*> HeroComponents;
		ZodiacCharacter->GetComponents(UZodiacHeroComponent::StaticClass(), HeroComponents);
		if (HeroComponents.Num() > 0)
		{
			for (UZodiacHeroComponent* HeroComponent : HeroComponents)
			{
				HeroComponent->OnHeroChanged.AddUObject(this, &ThisClass::HandleHeroChanged);
			}
		}
		
		TArray<UZodiacHealthComponent*> HealthComponents;
		ZodiacCharacter->GetComponents(UZodiacHealthComponent::StaticClass(), HealthComponents);
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

void UZodiacHealthBarWidget::OnHeroChanged_Implementation(float NewValue, float MaxHealth)
{
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

void UZodiacHealthBarWidget::OnHealthChanged_Implementation(float OldValue, float NewValue, float MaxHealth)
{
	
}
