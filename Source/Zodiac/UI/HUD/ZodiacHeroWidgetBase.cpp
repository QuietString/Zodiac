// the.quiet.string@gmail.com


#include "ZodiacHeroWidgetBase.h"

#include "Character/ZodiacPlayerCharacter.h"

void UZodiacHeroWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	}
}

void UZodiacHeroWidgetBase::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
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
	}
}

void UZodiacHeroWidgetBase::HandleHeroChanged(UZodiacHeroComponent* HeroComponent)
{
}
