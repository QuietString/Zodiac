// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacter.h"
#include "ZodiacCharacterType.h"
#include "Traversal/ZodiacTraversalActorInterface.h"
#include "ZodiacMonster.generated.h"

class UPhysicalAnimationComponent;
class UZodiacHeroData;

UENUM(BlueprintType)
enum EZodiacAIState : uint8
{
	Idle,
	Wandering,
	Chasing
};

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
	
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPhysicsTagChanged(FGameplayTag Tag, int Count);

	UFUNCTION(BlueprintPure)
	uint8 GetSpawnSeed() const { return SpawnSeed; }
	void SetSpawnSeed(const int32 Seed);

	FZodiacZombieSpawnConfig GetZombieSpawnConfig() const { return SpawnConfig; }
	void SetSpawnConfig(const FZodiacZombieSpawnConfig& InSpawnConfig);
	
protected:
	void OnSpawnSeedSet_Internal();
	void OnSpawnConfigSet();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> RetargetedMeshComponent;
	
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;
	
	// A randomizer seed for movement speed, walk/run animations when it's spawned by ZodiacZombieSpawner.
	UPROPERTY(ReplicatedUsing = OnRep_SpawnSeed)
	uint8 SpawnSeed;

	UFUNCTION()
	void OnRep_SpawnSeed();
	
	UPROPERTY(ReplicatedUsing = OnRep_SpawnConfig)
	FZodiacZombieSpawnConfig SpawnConfig;

	UFUNCTION()
	void OnRep_SpawnConfig();
	
	bool bHasMovementInitialized = false;
};
