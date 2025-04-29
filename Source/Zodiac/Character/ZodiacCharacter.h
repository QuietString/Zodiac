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

class UZodiacPreMovementComponentTickComponent;
class UZodiacHeroData;
struct FZodiacExtendedMovementConfig;
enum class EZodiacExtendedMovementMode : uint8;
enum class EZodiacWalkMode : uint8;
class UZodiacHealthComponent;
class UZodiacAbilitySystemComponent;
struct FInputActionValue;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilitySystemComponentInitialized, UAbilitySystemComponent*);

DECLARE_DELEGATE_FourParams(FOnPlayHitReact, FVector, FName, float, const FGameplayTagContainer&);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExtendedMovementModeChangedSignature, EZodiacExtendedMovementMode, ExtendedMovementMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FZodiacAIPawnOnWakeUpSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZodiacCharacterSleepAfterDeathDelegateSignature, AActor*, DeadActor);

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

USTRUCT()
struct FZodiacReplicatedIndependentYaw
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsAllowed = false;
	
	UPROPERTY()
	uint8 Yaw = 0;

	double GetUnpackedYaw() { return double(Yaw) * 360.0f / 255.0f;}
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

	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface
	
	virtual void BeginPlay() override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	virtual UZodiacHealthComponent* GetHealthComponent() const;
	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintCallable)
	virtual USkeletalMeshComponent* GetRetargetedMesh() const { return  nullptr; }

	UZodiacPreMovementComponentTickComponent* GetPreMovementComponentTick() const { return PreMovementComponentTick; }
	
	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface
	
	/** Delegate fired when the ability system component of this actor initialized */
	FOnAbilitySystemComponentInitialized OnAbilitySystemComponentInitialized;
	void CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate&& Delegate);
	
	void OnCharacterAttached(ACharacter* AttachedCharacter);
	
	virtual bool UpdateSharedReplication();

	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SimulateOrPlayHitReact(FVector HitDirection, FName HitBone, float Magnitude, FGameplayTagContainer InstigatorTags);

	FOnPlayHitReact OnSimulateOrPlayHitReact;

	UFUNCTION(BlueprintCallable)
	void SetExtendedMovementMode(const EZodiacExtendedMovementMode& InMode);
	void SetExtendedMovementConfig(const FZodiacExtendedMovementConfig& InConfig);

	virtual void OnExtendedMovementModeChanged(EZodiacExtendedMovementMode PreviousMode);
	virtual void SetExtendedMovementModeTag(EZodiacExtendedMovementMode ExtendedMovementMode, bool bTagEnabled);

	FZodiacReplicatedIndependentYaw GetReplicatedIndependentYaw() const { return ReplicatedIndependentYaw; };

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_WakeUp(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	virtual void WakeUp(const FVector& SpawnLocation, const FRotator& SpawnRotation);
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_Sleep();
	virtual void Sleep();
	
	UPROPERTY(BlueprintAssignable)
	FZodiacAIPawnOnWakeUpSignature OnWakeUp;

	UPROPERTY(BlueprintAssignable)
	FZodiacAIPawnOnWakeUpSignature OnSleep;

	UPROPERTY(BlueprintAssignable)
	FOnExtendedMovementModeChangedSignature BP_OnExtendedMovementModeChanged;
	
protected:	
	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner);
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void InitializePlayerInput();
	virtual void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	virtual void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Fly(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);

	void OnStatusTagChanged(FGameplayTag Tag, int Count);
	void OnMovementTagChanged(FGameplayTag Tag, int Count);
	virtual void OnPhysicsTagChanged(FGameplayTag Tag, int Count);
	
	virtual void OnJustLanded();
	virtual void OnJustLifted();
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Config")
	UZodiacHeroData* CharacterData;
	
	// @TODO: It's temporary testing usage for AHeroCharacter2
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Config")
	TArray<UZodiacHeroData*> HeroData;
	
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Config")
	FZodiacInputConfig InputConfig;

	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Replicated, Transient)
	FZodiacReplicatedIndependentYaw ReplicatedIndependentYaw;

private:
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZodiacPreMovementComponentTickComponent> PreMovementComponentTick;
	
private:
	UPROPERTY()
	bool bMovementDisabled = false;
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FZodiacReplicatedAcceleration ReplicatedAcceleration;

	UFUNCTION()
	void OnRep_ReplicatedAcceleration();
};
