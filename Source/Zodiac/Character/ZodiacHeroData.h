// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"

#include "ZodiacHeroData.generated.h"

class UZodiacSkillSlotDefinition;
class UZodiacAbilitySet;
struct FAttributeDefaults;
class UZodiacReticleWidgetBase;

UCLASS(BlueprintType, Const)
class UZodiacHeroData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Category = "Display")
	FName HeroName = TEXT("NoHeroName");
	
	UPROPERTY(EditAnywhere, Category = "Ability")
	TArray<TObjectPtr<UZodiacAbilitySet>> AbilitySets;

	UPROPERTY(EditAnywhere, Category = "Attribute")
	TArray<FAttributeDefaults> Attributes;
	
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<USkeletalMesh> HeroMesh;
	
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<USkeletalMesh> InvisibleMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	TSubclassOf<UAnimInstance> HeroAnimInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Skills", meta=(DisplayName = "Skill Slots", Categories = "SkillSlot"))
	TMap<FGameplayTag, TObjectPtr<UZodiacSkillSlotDefinition>> SkillSlotDefinitions;

	UPROPERTY(EditAnywhere, Category = "Display")
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> ReticleWidgets;
};