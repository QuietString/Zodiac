// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "Input/ZodiacInputComponent.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacCharacter.generated.h"

struct FZodiacMovementInputDirections;
class UZodiacHealthComponent;
class UZodiacAbilitySystemComponent;
struct FInputActionValue;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilitySystemComponentInitialized, UAbilitySystemComponent*);

/**
 * FZodiacReplicatedAcceleration: Compressed representation of acceleration
 */
USTRUCT()
struct FZodiacReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};

/** The type we use to send FastShared movement updates. */
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	FSharedRepMovement();

	bool FillForCharacter(ACharacter* Character);
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY(Transient)
	FRepMovement RepMovement;

	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;
};

template<>
struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

UCLASS(BlueprintType)
class ZODIAC_API AZodiacCharacter : public ACharacter, public IAbilitySystemInterface, public IZodiacTeamAgentInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AZodiacCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Tick(float DeltaSeconds) override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	virtual UZodiacHealthComponent* GetHealthComponent() const;
	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintCallable)
	virtual USkeletalMeshComponent* GetRetargetedMesh() const { return  nullptr; }

	UFUNCTION(BlueprintCallable)
	FZodiacMovementInputDirections GetMovementInputDirection() const;
	
	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface
	
	/** Delegate fired when the ability system component of this actor initialized */
	FOnAbilitySystemComponentInitialized OnAbilitySystemComponentInitialized;
	void CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate&& Delegate);

	virtual bool UpdateSharedReplication();

	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SimulateOrPlayHitReact(FVector HitDirection, FName HitBone, float Magnitude, FGameplayTagContainer InstigatorTags);

	void SetDefaultCustomMovementMode(uint8 CustomMode);
	void SetMovementMode(EMovementMode MovementMode, uint8 CustomMovementMode);
	void SetMovementSpeeds(const FVector& InWalkSpeeds, const FVector& InRunSpeeds);

protected:	
	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner);
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void InitializePlayerInput();
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);

	void OnStatusTagChanged(FGameplayTag Tag, int Count);
	void OnMovementTagChanged(FGameplayTag Tag, int Count);
	virtual void OnPhysicsTagChanged(FGameplayTag Tag, int Count);
	
	virtual void OnJustLanded();
	virtual void OnJustLifted();
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Player Input")
	FZodiacInputConfig InputConfig;

	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	FName CollisionProfileNameCached;
private:
	UPROPERTY()
	bool bMovementDisabled = false;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FZodiacReplicatedAcceleration ReplicatedAcceleration;

	UFUNCTION()
	void OnRep_ReplicatedAcceleration();

	
};
