// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ZodiacHeroAbilitySlotDefinition.generated.h"

class UZodiacHeroAbilityFragment;
class UZodiacAbilitySet;
class UZodiacHeroAbilitySlot;
class AZodiacHeroAbilitySlotActor;

UCLASS()
class ZODIAC_API UZodiacHeroAbilitySlotDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="HUD.Type.AbilitySlot"))
	FGameplayTag SlotType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UZodiacHeroAbilitySlot> SlotClass;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AZodiacHeroAbilitySlotActor>> ActorsToSpawn;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UZodiacAbilitySet> AbilitySetToGrant;
	
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Ability.Cost.Stack"))
	TMap<FGameplayTag, int32> InitialTagStack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<UZodiacHeroAbilityFragment*> Fragments;
};
