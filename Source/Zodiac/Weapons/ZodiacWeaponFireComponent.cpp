// the.quiet.string@gmail.com


#include "Weapons/ZodiacWeaponFireComponent.h"

#include "Character/ZodiacHeroComponent.h"
#include "Character/ZodiacPlayerCharacter.h"


void UZodiacWeaponFireComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	UE_LOG(LogTemp, Warning, TEXT("weapon component init"));
}

void UZodiacWeaponFireComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AZodiacPlayerCharacter* Character = GetPawn<AZodiacPlayerCharacter>())
	{
		TInlineComponentArray<UZodiacHeroComponent*> HeroComponents_Inline;
		Character->GetComponents<UZodiacHeroComponent>(HeroComponents_Inline);
		HeroComponents = HeroComponents_Inline;
		
		if (UActorComponent* Component = Character->FindComponentByTag(USkeletalMeshComponent::StaticClass(), TEXT("Hero")))
		{
			HeroMeshComponent = CastChecked<USkeletalMeshComponent>(Component);
		}
	}
	
	if (HeroComponents.Num() > 0)
	{
		for (auto& HeroComponent : HeroComponents)
		{
			HeroComponent->OnHeroChanged.AddUObject(this, &ThisClass::OnHeroChanged);
		}
	}
}

void UZodiacWeaponFireComponent::OnHeroChanged(UZodiacHeroComponent* NewHeroComponent)
{
	if (NewHeroComponent)
	{
		UpdateMuzzleSockets(NewHeroComponent->GetHeroData()->MuzzleSocketNames);	
	}
}

void UZodiacWeaponFireComponent::UpdateMuzzleSockets(const TArray<FName>& InSocketNames)
{
	if (!InSocketNames.IsEmpty())
	{
		MuzzleSocketNames = InSocketNames;	
	}
}
