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

UCLASS(DefaultToInstanced, EditInlineNew, Abstract, CollapseCategories)
class ZODIAC_API UZodiacHeroAbilityFragment : public UObject
{
	GENERATED_BODY()
	
public:

	UZodiacHeroAbilityFragment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void OnSlotCreated(UZodiacHeroAbilitySlot* InSlot) const {}
};

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

public:
	const UZodiacHeroAbilityFragment* FindFragmentByClass(const TSubclassOf<UZodiacHeroAbilityFragment>& FragmentClass) const;
};

inline const UZodiacHeroAbilityFragment* UZodiacHeroAbilitySlotDefinition::FindFragmentByClass(const TSubclassOf<UZodiacHeroAbilityFragment>& FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UZodiacHeroAbilityFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}
