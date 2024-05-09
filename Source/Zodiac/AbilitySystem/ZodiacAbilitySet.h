// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "Engine/DataAsset.h"
#include "ZodiacAbilitySet.generated.h"

class UZodiacGameplayAbility;
struct FActiveGameplayEffectHandle;
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

// Represents a fragment of an skill definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class ZODIAC_API UZodiacSkillFragment : public UObject
{
	GENERATED_BODY()

public:
};

UCLASS()
class USkillFragment_Display : public UZodiacSkillFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayName;
};

USTRUCT()
struct FZodiacSkillSet : public FZodiacAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	
	// Tag for identifying skill type.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "Ability.Type.Skill"))
	FGameplayTag SkillType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<UZodiacSkillFragment>> Fragments;

	template <typename FragmentType>
	FragmentType* GetFragmentByClass() const
	{
		for (const TObjectPtr<UZodiacSkillFragment>& Fragment : Fragments)
		{
			if (FragmentType* CastedFragment = Cast<FragmentType>(Fragment.Get()))
			{
				return CastedFragment;
			}
		}
		return nullptr;
	}
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
	void AddSkillHandle(const FGameplayAbilitySpecHandle& Handle);
	//void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	TArray<FGameplayAbilitySpecHandle> GetSkillHandles() { return SkillHandles; }
protected:

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted skill abilities.
	TArray<FGameplayAbilitySpecHandle> SkillHandles;
	
	// Handles to the granted gameplay effects.
	//UPROPERTY()
	//TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
	
	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


USTRUCT()
struct FZodiacSkillSetWithHandle
{
	GENERATED_BODY()

public:
	TMap<const FGameplayAbilitySpecHandle, const FZodiacSkillSet*> Map;
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

	void GiveToAbilitySystem(UZodiacAbilitySystemComponent* ZodiacASC, FZodiacAbilitySet_GrantedHandles* OutGrantedHandles, FZodiacSkillSetWithHandle* OutSkillData, UObject* SourceObject = nullptr) const;

protected:

	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FZodiacAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay skill abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TArray<FZodiacSkillSet> GrantedSkillAbilities;
	
	// Attribute sets to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FZodiacAbilitySet_AttributeSet> GrantedAttributes;
};