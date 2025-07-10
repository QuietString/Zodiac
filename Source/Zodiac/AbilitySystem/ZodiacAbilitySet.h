// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "GameplayAbilitySpecHandle.h"
#include "Engine/DataAsset.h"
#include "ZodiacAbilitySet.generated.h"

struct FActiveGameplayEffectHandle;
class UGameplayEffect;
class UZodiacHeroAbilityManagerComponent;
class UZodiacGameplayAbility;
class UZodiacAbilitySystemComponent;
class UAttributeSet;

/**
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FZodiacAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:

	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UZodiacGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 *	Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FZodiacAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:

	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/**
 *	Data used by the ability set to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct FZodiacAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	// Gameplay attribute to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> Table;
};

/**
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct FZodiacAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

private:
	friend UZodiacHeroAbilityManagerComponent;
	
protected:
	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
	
	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/**
 *	Non-mutable data asset used to grant gameplay abilities and gameplay effects.
 */
UCLASS(BlueprintType, Const)
class ZODIAC_API UZodiacAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UZodiacAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GiveToAbilitySystem(UZodiacAbilitySystemComponent* ZodiacASC, FZodiacAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;
protected:
	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly)
	TArray<FZodiacAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FZodiacAbilitySet_GameplayEffect> GrantedGameplayEffects;
	
	// Attribute sets to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, meta=(TitleProperty=AttributeSet))
	TArray<FZodiacAbilitySet_AttributeSet> GrantedAttributes;
};