// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "ZodiacHeroData.generated.h"


class UZodiacAbilitySet;
struct FAttributeDefaults;
class UZodiacReticleWidgetBase;

UCLASS(DefaultToInstanced, EditInlineNew)
class UZodiacSkillSocketSet : public UObject
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Ability.Skill.Type"))
	FGameplayTag SkillTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> Sockets;
};

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

	UPROPERTY(EditAnywhere, Category = "Socket")
	TArray<FName> MuzzleSocketNames;

	UPROPERTY(EditAnywhere, Instanced, Category = "Sockets")
	TArray<UZodiacSkillSocketSet*> SkillSockets;
	
	UPROPERTY(EditAnywhere, Category = "Display")
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> ReticleWidgets;
};