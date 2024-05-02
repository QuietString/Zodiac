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
class UZodiacCombatSet;
class AZodiacPlayerCharacter;
class UZodiacAbilitySystemComponent;
class UZodiacAbilitySet;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroChanged, const TArray<FName>&);

UCLASS(BlueprintType)
class UZodiacHeroData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Category = "Zodiac")
	FName HeroName = TEXT("NoHeroName");
	
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

	UPROPERTY(EditAnywhere, Category = "Zodiac|Socket")
	TArray<FName> MuzzleSocketNames;
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

public:

	FOnHeroChanged OnHeroChanged;
	
protected:

	void AddAbilities();

protected:

	UPROPERTY()
	FName HeroName;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac")
	UZodiacHeroData* HeroData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer HeroTags;

	UPROPERTY()
	uint8 SlotIndex;
	
private:
	
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UZodiacHealthSet> HealthSet;

	UPROPERTY()
	TObjectPtr<const UZodiacCombatSet> CombatSet;
	
	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
	
	UPROPERTY()
	TObjectPtr<AZodiacPlayerCharacter> PlayerCharacter;
};

