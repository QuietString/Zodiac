// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "ZodiacMonster.generated.h"

class UZodiacAbilitySet;
class UZodiacAbilitySystemComponent;

UCLASS()
class ZODIAC_API AZodiacMonster : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AZodiacMonster();

	//~AActor interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	//~End of AActor interface

	UFUNCTION(BlueprintCallable)
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//~IGameplayTagAssetInterface interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

protected:
	void InitializeAbilitySystemComponent();

	void AddAbilities();

private:
	UPROPERTY()
    TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Ability")
	TArray<UZodiacAbilitySet*> Abilities;
};
