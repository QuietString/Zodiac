// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacCharacter.h"
#include "Character/ZodiacCharacterType.h"
#include "Traversal/ZodiacTraversalActorInterface.h"
#include "ZodiacMonster.generated.h"

class UZodiacHitReactSimulationComponent;
class UPhysicalAnimationComponent;

class UBehaviorTree;

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacMonster : public AZodiacCharacter, public IZodiacTraversalActorInterface
{
	GENERATED_BODY()

public:
	AZodiacMonster(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AZodiacCharacter interface
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacHealthComponent* GetHealthComponent() const override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual USkeletalMeshComponent* GetRetargetedMesh() const override { return RetargetedMeshComponent; }
	virtual void OnPhysicsTagChanged(FGameplayTag Tag, int Count) override;
	//~End of AZodiacCharacter interface

	//~IZodiacTraversalActorInterface
	virtual UAbilitySystemComponent* GetTraversalAbilitySystemComponent() const override;
	//~End of IZodiacTraversalActorInterface

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPhysicsTagChanged(FGameplayTag Tag, int Count);

	UFUNCTION(BlueprintPure)
	uint8 GetSpawnSeed() const { return SpawnConfig.Seed; }

	FZodiacZombieSpawnConfig GetZombieSpawnConfig() const { return SpawnConfig; }
	void SetSpawnConfig(const FZodiacZombieSpawnConfig& InSpawnConfig);

	virtual void Sleep() override;
	virtual void WakeUp(const FVector& SpawnLocation, const FRotator& SpawnRotation) override;
	
protected:
	void OnSpawnConfigSet();

public:
	friend class AZodiacAIPawnSpawner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "AI")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "AI")
	TWeakObjectPtr<AZodiacAIPawnSpawner> Spawner;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> RetargetedMeshComponent;
	
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHitReactSimulationComponent> HitReactSimulationComponent;

	UPROPERTY(ReplicatedUsing = OnRep_SpawnConfig)
	FZodiacZombieSpawnConfig SpawnConfig;

	UFUNCTION()
	void OnRep_SpawnConfig();
	
	bool bHasMovementInitialized = false;

	// Flag to decide to use class default behavior or not.
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bIsSpawnedBySpawner = false;
};
