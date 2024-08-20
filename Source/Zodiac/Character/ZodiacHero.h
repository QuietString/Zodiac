// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ZodiacHero.generated.h"

class UZodiacHUDManagerComponent;
struct FZodiacHeroList;
class UInputMappingContext;
class UZodiacHeroAbilitySystemComponent;
class UZodiacCharacterMovementComponent;
class AZodiacHostCharacter;
class UZodiacHealthComponent;
class UZodiacHeroData;
class UZodiacAbilitySystemComponent;

UCLASS(Abstract)
class ZODIAC_API AZodiacHero : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZodiacHero(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void OnRep_Owner() override;
	//~End of AActor interface
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent() const;

	const UZodiacHeroData* GetHeroData() const { return HeroData; }
	AZodiacHostCharacter* GetHostCharacter() const;
	UZodiacHealthComponent* GetHealthComponent() const;
	USkeletalMeshComponent* GetMesh() { return Mesh; }

	int32 GetIndex() const { return Index; }
	
	void Activate();
	void Deactivate();
	FSimpleMulticastDelegate OnHeroActivated;
	
protected:
	void Initialize();
	
	void InitializeAbilitySystem();

	void OnStatusTagChanged(FGameplayTag Tag, int Count);

	void AttachToOwner();

	void OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* HostASC);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere)
	UZodiacHeroAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHUDManagerComponent> HUDManagerComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Shape, meta=(ClampMin="0", UIMin="0"))
	float CapsuleHalfHeight = 88.0f;;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Shape, meta=(ClampMin="0", UIMin="0"))
	float CapsuleRadius = 40.0f;
	
private:
	friend FZodiacHeroList;
	
	UPROPERTY()
	TObjectPtr<AZodiacHostCharacter> HostCharacter;

	UPROPERTY()
	int32 Index = INDEX_NONE;
	
	// Initial use only for initialization on a client.
	UPROPERTY(ReplicatedUsing=OnRep_bIsActive)
	bool bIsActive = false;

	UFUNCTION()
	void OnRep_bIsActive(bool OldValue);
};