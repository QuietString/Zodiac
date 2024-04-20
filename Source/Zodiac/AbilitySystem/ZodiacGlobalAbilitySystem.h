// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpecHandle.h"
#include "Templates/SubclassOf.h"

#include "ZodiacGlobalAbilitySystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UZodiacAbilitySystemComponent;
class UObject;
struct FActiveGameplayEffectHandle;
struct FFrame;
struct FGameplayAbilitySpecHandle;

USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UZodiacAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayAbility> Ability, UZodiacAbilitySystemComponent* ASC);
	void RemoveFromASC(UZodiacAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UZodiacAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayEffect> Effect, UZodiacAbilitySystemComponent* ASC);
	void RemoveFromASC(UZodiacAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

UCLASS()
class UZodiacGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UZodiacGlobalAbilitySystem();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Zodiac")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Zodiac")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Zodiac")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Zodiac")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** Register an ASC with global system and apply any active global effects/abilities. */
	void RegisterASC(UZodiacAbilitySystemComponent* ASC);

	/** Removes an ASC from the global system, along with any active global effects/abilities. */
	void UnregisterASC(UZodiacAbilitySystemComponent* ASC);

private:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UZodiacAbilitySystemComponent>> RegisteredASCs;
};
