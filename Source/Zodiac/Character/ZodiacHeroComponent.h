// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Components/PawnComponent.h"
#include "ZodiacHeroComponent.generated.h"


class AZodiacPlayerCharacter;
class UZodiacAbilitySystemComponent;
class UZodiacAbilitySet;

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

	UZodiacAbilitySystemComponent* InitializeAbilitySystemComponent();

	void ActivateHero();
	void DeactivateHero();
protected:
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName HeroName = TEXT("Default");
	
protected:

	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<AZodiacPlayerCharacter> PlayerCharacter;
	
	UPROPERTY()
	TObjectPtr<const class UZodiacHealthSet> HealthSet;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Ability")
	TArray<TObjectPtr<UZodiacAbilitySet>> AbilitySets;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TObjectPtr<USkeletalMesh> HeroMesh;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TObjectPtr<USkeletalMesh> InvisibleMesh;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TSubclassOf<UAnimInstance> HeroAnimInstance;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Mesh")
	TSubclassOf<UAnimInstance> CopyPoseAnimInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer HeroTags;
};

