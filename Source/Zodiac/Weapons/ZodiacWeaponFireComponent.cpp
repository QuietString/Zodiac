// the.quiet.string@gmail.com


#include "Weapons/ZodiacWeaponFireComponent.h"

#include "Character/ZodiacHeroComponent.h"
#include "Character/ZodiacPlayerCharacter.h"


UZodiacWeaponFireComponent::UZodiacWeaponFireComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
}

void UZodiacWeaponFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	if (AZodiacPlayerCharacter* Character = GetPawn<AZodiacPlayerCharacter>())
	{
		if (UActorComponent* Component = Character->FindComponentByTag(USkeletalMeshComponent::StaticClass(), TEXT("Hero")))
		{
			HeroMeshComponent = CastChecked<USkeletalMeshComponent>(Component);
		}
	}
}
