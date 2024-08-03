// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ZodiacHero.generated.h"

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
	TObjectPtr<UZodiacAbilitySystemComponent> GetZodiacAbilitySystemComponent() const;

	AZodiacHostCharacter* GetHostCharacter() const;
	UZodiacHealthComponent* GetHealthComponent() const;
	USkeletalMeshComponent* GetMesh() { return Mesh; }
	
	void Activate();
	void Deactivate();
	
protected:
	void Initialize();
	
	void InitializeAbilitySystem();

	void AttachToOwner();

	void OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* HostASC);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere)
	UZodiacAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Shape, meta=(ClampMin="0", UIMin="0"))
	float CapsuleHalfHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Shape, meta=(ClampMin="0", UIMin="0"))
	float CapsuleRadius;
	
private:
	// Initial use only for initialization on a client.
	UPROPERTY(ReplicatedUsing=OnRep_bIsActive)
	bool bIsActive = false;

	UFUNCTION()
	void OnRep_bIsActive(bool OldValue);
};