// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacHeroCharacter.generated.h"

class UZodiacHeroAnimInstance;
class UZodiacHeroAbilityManagerComponent;
struct FZodiacHeroList;
class UInputMappingContext;
class UZodiacHeroAbilitySystemComponent;
class UZodiacCharacterMovementComponent;
class AZodiacHostCharacter;
class UZodiacHealthComponent;
class UZodiacHeroData;
class UZodiacAbilitySystemComponent;

UCLASS(Abstract)
class ZODIAC_API AZodiacHeroCharacter : public ACharacter, public IAbilitySystemInterface, public IZodiacTeamAgentInterface
{
	GENERATED_BODY()

public:
	AZodiacHeroCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void OnRep_Owner() override;
	//~End of AActor interface

	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent() const;

	const UZodiacHeroData* GetHeroData() const { return HeroData; }

	UFUNCTION(BlueprintCallable)
	AZodiacHostCharacter* GetHostCharacter() const;
	
	UZodiacHealthComponent* GetHealthComponent() const;
	
	UZodiacHeroAnimInstance* GetHeroAnimInstance() const;
	
	void Activate();
	void Deactivate();
	
	FSimpleMulticastDelegate OnHeroActivated;
	FSimpleMulticastDelegate OnHeroDeactivated;
	
protected:
	void InitializeWithHostCharacter();
	
	void InitializeAbilitySystem();

	void OnStatusTagChanged(FGameplayTag Tag, int Count);

	void AttachToOwner();

	void OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* HostASC);

protected:
	UPROPERTY(VisibleAnywhere)
	UZodiacHeroAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHeroAbilityManagerComponent> AbilityManagerComponent;

	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

private:
	friend FZodiacHeroList;
	
	UPROPERTY()
	TObjectPtr<AZodiacHostCharacter> HostCharacter;

	// Initial use only for initialization on a client.
	UPROPERTY(ReplicatedUsing=OnRep_bIsActive)
	bool bIsActive = false;

	UFUNCTION()
	void OnRep_bIsActive(bool OldValue);
};