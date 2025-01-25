// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacter.h"
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
class ZODIAC_API AZodiacMonster : public AZodiacCharacter
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
	
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPhysicsTagChanged(FGameplayTag Tag, int Count);

	UFUNCTION(BlueprintPure)
	uint8 GetSpawnSeed() const { return SpawnSeed; }
	void SetSpawnSeed(const uint8 Seed);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

	UFUNCTION(BlueprintImplementableEvent)
	void OnSpawnSeedSet();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> RetargetedMeshComponent;
	
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;

	UPROPERTY(ReplicatedUsing = OnRep_SpawnSeed)
	uint8 SpawnSeed;

	UFUNCTION()
	void OnRep_SpawnSeed();
};
