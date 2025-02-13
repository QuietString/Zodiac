// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacMonster.generated.h"

class UZodiacHealthComponent;
class UZodiacAbilitySet;
class UZodiacAbilitySystemComponent;

UCLASS()
class ZODIAC_API AZodiacMonster : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface, public IZodiacTeamAgentInterface
{
	GENERATED_BODY()

public:
	AZodiacMonster(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~AActor interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	//~End of AActor interface

	UFUNCTION(BlueprintCallable)
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//~IZodiacTeamAgentInterface interface
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	//~End of IZodiacTeamAgentInterface interface
	
	//~IGameplayTagAssetInterface interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

protected:
	void InitializeAbilitySystemComponent();

	void AddAbilities();

protected:
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UZodiacHealthComponent> AttributeManagerComponent;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Ability")
	TArray<UZodiacAbilitySet*> Abilities;

private:
	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Monster;
};
