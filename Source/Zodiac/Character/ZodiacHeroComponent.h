// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagAssetInterface.h"
#include "Components/PawnComponent.h"
#include "ZodiacHeroComponent.generated.h"


class UZodiacHealthComponent;
struct FAttributeDefaults;
class UZodiacHealthSet;
class AZodiacPlayerCharacter;
class UZodiacAbilitySystemComponent;
class UZodiacAbilitySet;

UCLASS(BlueprintType)
class UZodiacHeroData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Category = "Zodiac")
	FName HeroName = TEXT("Default");
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Ability")
	TArray<TObjectPtr<UZodiacAbilitySet>> AbilitySets;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Attribute")
	TArray<FAttributeDefaults> Attributes;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TObjectPtr<USkeletalMesh> HeroMesh;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TObjectPtr<USkeletalMesh> InvisibleMesh;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TSubclassOf<UAnimInstance> HeroAnimInstance;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TSubclassOf<UAnimInstance> CopyPoseAnimInstance;
};

UCLASS()
class ZODIAC_API UZodiacHeroComponent : public UPawnComponent, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	UZodiacAbilitySystemComponent* InitializeAbilitySystem();

	void ActivateHero();
	void DeactivateHero();

protected:

	void AddAbilities();
	
public:
	
	UPROPERTY()
	FName HeroName;
	
protected:

	UPROPERTY(EditAnywhere, Category = "Zodiac")
	UZodiacHeroData* HeroData;
	
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
	
	UPROPERTY()
	TObjectPtr<AZodiacPlayerCharacter> PlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer HeroTags;

};

