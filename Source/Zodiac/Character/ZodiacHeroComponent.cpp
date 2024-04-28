// the.quiet.string@gmail.com

#include "ZodiacHeroComponent.h"

#include "ZodiacPlayerCharacter.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"


UZodiacHeroComponent::UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::GetZodiacAbilitySystemComponent()
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* UZodiacHeroComponent::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void UZodiacHeroComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);	
	}
}

void UZodiacHeroComponent::OnRegister()
{
	if (HeroData)
	{
		AbilitySystemComponent->DefaultStartingData = HeroData->Attributes;
	}

	PlayerCharacter = GetPawnChecked<AZodiacPlayerCharacter>();

	Super::OnRegister();
}

void UZodiacHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	ensureMsgf(HeroData, TEXT("Must have HeroData"));

	HealthSet = AbilitySystemComponent->GetSet<UZodiacHealthSet>();
	
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::InitializeAbilitySystemComponent()
{
	if (HeroData)
	{
		if (HeroData->AbilitySets.Num() > 0)
		{
			for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
			{
				AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
			}
		}
		
		AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());

		if (AbilitySystemComponent->GetAttributeSet(UZodiacHealthSet::StaticClass()))
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZodiacHealthSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
		}
		
		return AbilitySystemComponent;
	}

	return nullptr;
}

void UZodiacHeroComponent::ActivateHero()
{
	//UE_LOG(LogTemp, Warning, TEXT("Activate %s"), *HeroData->HeroName.ToString());

	PlayerCharacter->ChangeHeroMesh(HeroData->HeroMesh, HeroData->HeroAnimInstance);
	PlayerCharacter->ChangeCharacterMesh(HeroData->InvisibleMesh, HeroData->CopyPoseAnimInstance);
}

void UZodiacHeroComponent::DeactivateHero()
{
	//UE_LOG(LogTemp, Warning, TEXT("Deactivate %s"), *HeroData->HeroName.ToString());
}

void UZodiacHeroComponent::OnHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;

	UE_LOG(LogTemp, Warning, TEXT("Health changed from %.1f to %.1f"), OldValue, NewValue);
}
