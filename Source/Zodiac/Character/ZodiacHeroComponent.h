// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Components/PawnComponent.h"
#include "ZodiacHeroComponent.generated.h"


class UZodiacAttributeManagerComponent;
class UZodiacAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroChanged, UZodiacHeroComponent*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillChanged, UAbilitySystemComponent*, const TArray<FGameplayAbilitySpecHandle>&);

UCLASS()
class ZODIAC_API UZodiacHeroComponent : public UPawnComponent, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:

	UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static int32 AssignNewID(UZodiacHeroComponent* HeroComponent); 
	int32 GetUniqueID() { return UniqueID; }
	
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	TArray<FName> GetCurrentAbilitySockets(const FGameplayTag AbilityTag);
	
	TObjectPtr<UZodiacAttributeManagerComponent> GetHealthComponent() { return AttributeManagerComponent; }
	
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	const UZodiacHeroData* GetHeroData() { return HeroData; }
	
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	UZodiacAbilitySystemComponent* InitializeAbilitySystem();

	void ActivateHero();
	void DeactivateHero();

public:

	FOnHeroChanged OnHeroChanged;

	FOnSkillChanged OnSkillChanged;
	
protected:

	void AddAbilities();

protected:

	UPROPERTY()
	FName HeroName;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac")
	const UZodiacHeroData* HeroData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer HeroTags;

	// Handles to hero abilities.
	FZodiacAbilitySet_GrantedHandles AbilityHandles;

	FZodiacSkillSetWithHandle SkillData;
private:
	
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UZodiacAttributeManagerComponent> AttributeManagerComponent;

	UPROPERTY()
	int32 UniqueID;
};

