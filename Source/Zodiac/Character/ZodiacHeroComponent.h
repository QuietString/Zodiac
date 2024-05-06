// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ZodiacHeroData.h"
#include "Components/PawnComponent.h"
#include "UI/Weapons/ZodiacReticleWidgetBase.h"
#include "ZodiacHeroComponent.generated.h"


class UZodiacHealthComponent;
class AZodiacPlayerCharacter;
class UZodiacAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroChanged, UZodiacHeroComponent*);

UCLASS()
class ZODIAC_API UZodiacHeroComponent : public UPawnComponent, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	TArray<FName> GetCurrentAbilitySockets(const FGameplayTag AbilityTag);
	
	TObjectPtr<UZodiacHealthComponent> GetHealthComponent() { return HealthComponent; }
	
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	const UZodiacHeroData* GetHeroData() { return HeroData; }
	
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	UZodiacAbilitySystemComponent* InitializeAbilitySystem();

	void ActivateHero();
	void DeactivateHero();

public:

	FOnHeroChanged OnHeroChanged;
	
protected:

	void AddAbilities();

protected:

	UPROPERTY()
	FName HeroName;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac")
	const UZodiacHeroData* HeroData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer HeroTags;
	
private:
	
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
};

