// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Components/PawnComponent.h"
#include "ZodiacHeroComponent.generated.h"

class UHeroDisplayManagerComponent;

UCLASS()
class UHeroSlotDisplayData : public UObject
{
	GENERATED_BODY()

public:
	FSlateBrush Brush;
	
};

class UZodiacHealthComponent;
class UZodiacAbilitySystemComponent;
class UZodiacHeroComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroChanged, UZodiacHeroComponent*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillChanged, UAbilitySystemComponent*, const TArray<FGameplayAbilitySpecHandle>&);

UCLASS()
class ZODIAC_API UZodiacHeroComponent : public UPawnComponent, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	TObjectPtr<UZodiacHealthComponent> GetHealthComponent() { return HealthComponent; }
	
	int32 GetSlotIndex() { return SlotIndex; }
	void SetSlotIndex(const int32 NewIndex) { SlotIndex = NewIndex; }
	
	TArray<FName> GetCurrentAbilitySockets(const FGameplayTag AbilityTag);
	
	//~IGameplayTagAssetInterface interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface
	
	const UZodiacHeroData* GetHeroData() { return HeroData; }
	
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	UZodiacAbilitySystemComponent* InitializeAbilitySystem();

	void ActivateHero();
	void DeactivateHero();

public:

	FOnHeroChanged OnHeroChanged;
	FOnSkillChanged OnSkillChanged;
	FSimpleMulticastDelegate OnHeroChanged_Simple;
	
protected:

	void AddAbilities();
	
protected:

	UPROPERTY()
	FName HeroName;

	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac")
	const UZodiacHeroData* HeroData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer HeroTags;

	// Handles to hero abilities.
	FZodiacAbilitySet_GrantedHandles AbilityHandles;

	FZodiacSkillSetWithHandle SkillData;

private:
	
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY()
	TObjectPtr<UHeroDisplayManagerComponent> DisplayManager;

};

