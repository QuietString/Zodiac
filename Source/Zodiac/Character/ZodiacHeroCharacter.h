// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
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
class AZodiacHeroCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroChanged, AZodiacHeroCharacter*, Hero);

UCLASS(Abstract)
class ZODIAC_API AZodiacHeroCharacter : public ACharacter, public IAbilitySystemInterface, public IZodiacTeamAgentInterface, public IGameplayTagAssetInterface
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

	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface
	
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	UZodiacHeroAbilitySystemComponent* GetHeroAbilitySystemComponent() const;
	UZodiacAbilitySystemComponent* GetHostAbilitySystemComponent() const;
	
	const UZodiacHeroData* GetHeroData() const { return HeroData; }

	UFUNCTION(BlueprintCallable)
	AZodiacHostCharacter* GetHostCharacter() const;

	UZodiacHealthComponent* GetHealthComponent() const;
	
	UZodiacHeroAnimInstance* GetHeroAnimInstance() const;

	UFUNCTION(BlueprintCallable)
	void SetModularMesh(TSubclassOf<USkeletalMeshComponent> SkeletalMeshCompClass, FName Socket);
	
	UFUNCTION(BlueprintCallable)
	void ClearModularMesh();
	
	void Activate();
	void Deactivate();
	
	FSimpleMulticastDelegate OnHeroActivated;
	FSimpleMulticastDelegate OnHeroDeactivated;

	UPROPERTY(BlueprintAssignable)
	FOnHeroChanged OnHeroActivated_BP;
	
	UPROPERTY(BlueprintAssignable)
	FOnHeroChanged OnHeroDeactivated_BP;
	
	bool GetIsInitialized() const { return bIsInitialized; }

	UFUNCTION(BlueprintPure)
	bool GetIsActive() const { return bIsActive; }

	UFUNCTION(BlueprintPure)
	float GetAimYaw() const;
	
protected:
	void InitializeWithHostCharacter();

	void InitializeAbilitySystem();

	void OnStatusTagChanged(FGameplayTag Tag, int Count);

	void AttachToOwner();

	void OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* InHostASC);

protected:
	UPROPERTY(VisibleAnywhere)
	UZodiacHeroAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHeroAbilityManagerComponent> AbilityManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> ModularMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimLayerClass;
	
private:
	friend FZodiacHeroList;
	
	UPROPERTY()
	TObjectPtr<AZodiacHostCharacter> HostCharacter;

	// Initial use only for initialization on a client.
	UPROPERTY(ReplicatedUsing=OnRep_IsActive)
	bool bIsActive = false;

	bool bIsInitialized = false;
	
	UFUNCTION()
	void OnRep_IsActive(bool OldValue);
};