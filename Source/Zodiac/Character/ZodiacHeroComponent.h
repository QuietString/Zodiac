// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Components/PawnComponent.h"
#include "UI/Weapons/ZodiacReticleWidgetBase.h"
#include "ZodiacHeroComponent.generated.h"


class UZodiacHealthComponent;
struct FAttributeDefaults;
class UZodiacHealthSet;
class UZodiacCombatSet;
class AZodiacPlayerCharacter;
class UZodiacAbilitySystemComponent;
class UZodiacAbilitySet;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroChanged, UZodiacHeroComponent*);

UCLASS(BlueprintType)
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

	UPROPERTY(EditAnywhere, Category = "Display")
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> ReticleWidgets;
};

UCLASS()
class ZODIAC_API UZodiacHeroComponent : public UPawnComponent, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	TObjectPtr<UZodiacHealthComponent> GetHealthComponent() { return HealthComponent; }
	
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	const UZodiacHeroData* GetHeroData() { return HeroData; }
	
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
	const UZodiacHeroData* HeroData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer HeroTags;
	
private:
	
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
};

