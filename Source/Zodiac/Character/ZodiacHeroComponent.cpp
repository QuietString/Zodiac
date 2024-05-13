// the.quiet.string@gmail.com

#include "ZodiacHeroComponent.h"

#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacAttributeManagerComponent.h"
#include "ZodiacHeroData.h"
#include "Skills/ZodiacSkillManagerComponent.h"

UZodiacHeroComponent::UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UniqueID = INDEX_NONE;
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeManagerComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAttributeManagerComponent>(this, TEXT("AttributeManagerComponent"));
}

int32 UZodiacHeroComponent::AssignNewID(UZodiacHeroComponent* HeroComponent)
{
	// Must be in C++ to avoid duplicate statics across execution units
	static int32 GHandle = 1;
	HeroComponent->UniqueID = GHandle++;

	return HeroComponent->UniqueID;
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::GetZodiacAbilitySystemComponent()
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* UZodiacHeroComponent::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TArray<FName> UZodiacHeroComponent::GetCurrentAbilitySockets(const FGameplayTag AbilityTag)
{
	for (auto& SocketSet : HeroData->SkillSockets)
	{
		if (SocketSet->SkillTag == AbilityTag)
		{
			return SocketSet->Sockets;
		}
	}
	return TArray<FName>();
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
		// DefaultStartingData is added to ASC on OnRegister()
		AbilitySystemComponent->DefaultStartingData = HeroData->Attributes;
		HeroName = HeroData->HeroName;
	}
	
	Super::OnRegister();
}

void UZodiacHeroComponent::BeginPlay()
{
	check(HeroData && "Must have HeroData");

	Super::BeginPlay();
	
	OnHeroChanged.Broadcast(this);
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::InitializeAbilitySystem()
{
	AActor* Owner = GetOwner();
	check(Owner && "No Owner Actor");

	AddAbilities();
	
	AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());
	AttributeManagerComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
	
	
	return AbilitySystemComponent;
}

void UZodiacHeroComponent::ActivateHero()
{
	OnHeroChanged.Broadcast(this);
	OnSkillChanged.Broadcast(AbilitySystemComponent, AbilityHandles.GetSkillHandles());
}

void UZodiacHeroComponent::DeactivateHero()
{
	
}

void UZodiacHeroComponent::AddAbilities()
{
	if (HeroData && HeroData->AbilitySets.Num() > 0)
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, OUT &AbilityHandles, OUT &SkillData);
		}
	}
	
	if (APawn* Pawn = GetPawn<APawn>())
	{
		UZodiacSkillManagerComponent* SkillManager = Pawn->FindComponentByClass<UZodiacSkillManagerComponent>();
		SkillManager->RegisterSkillDisplayData(SkillData);
	}
}
