// the.quiet.string@gmail.com


#include "Monster/ZodiacMonster.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"


AZodiacMonster::AZodiacMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

}

void AZodiacMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitializeAbilitySystemComponent();
}

void AZodiacMonster::BeginPlay()
{
	Super::BeginPlay();
}

UZodiacAbilitySystemComponent* AZodiacMonster::GetZodiacAbilitySystemComponent() const
{
	return Cast<UZodiacAbilitySystemComponent>(AbilitySystemComponent);
}

UAbilitySystemComponent* AZodiacMonster::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacMonster::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
}

bool AZodiacMonster::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return IGameplayTagAssetInterface::HasMatchingGameplayTag(TagToCheck);
}

bool AZodiacMonster::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return IGameplayTagAssetInterface::HasAllMatchingGameplayTags(TagContainer);
}

bool AZodiacMonster::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return IGameplayTagAssetInterface::HasAnyMatchingGameplayTags(TagContainer);
}

void AZodiacMonster::InitializeAbilitySystemComponent()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AddAbilities();

	HealthSet = AbilitySystemComponent->GetSet<UZodiacHealthSet>();
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZodiacHealthSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);

	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
}

void AZodiacMonster::AddAbilities()
{
	for (TObjectPtr<UZodiacAbilitySet> AbilitySet : Abilities)
	{
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
	}
}

void AZodiacMonster::HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;
	
	UE_LOG(LogTemp, Warning, TEXT("health changed from %.1f to %.1f"), OldValue, NewValue);
}

void AZodiacMonster::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser,
                                       const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = ZodiacGameplayTags::GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;
			
			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			int32 Num = AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
			UE_LOG(LogTemp, Warning, TEXT("out of health event sent: %d "), Num);
		}
	}
#endif
}
