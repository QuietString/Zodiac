// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameFramework/Character.h"
#include "Camera/ZodiacCameraMode.h"
#include "Input/ZodiacInputComponent.h"
#include "ZodiacHeroList.h"
#include "ZodiacHostCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHostAbilitySystemComponentLoaded, UAbilitySystemComponent*);

class UZodiacHealthComponent;

UENUM(BlueprintType)
enum EZodiacGait
{
	Gait_Walk,
	Gait_Run,
	Gait_Sprint
};

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


UCLASS(Abstract)
class ZODIAC_API AZodiacHostCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();
	virtual UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent();
	void InitializeHostAbilitySystem(UZodiacAbilitySystemComponent* InASC);

	void CallOrRegister_OnAbilitySystemInitialized(FOnHostAbilitySystemComponentLoaded::FDelegate&& Delegate);
	UZodiacHealthComponent* GetCurrentHeroHealthComponent();

	void ChangeHero(const int32 Index);
	
	/** Overrides the camera from an active gameplay ability */
	void SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Clears the camera override if it is set */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);
	
	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	virtual bool UpdateSharedReplication();
	
protected:
	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void InitializePlayerInput();
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);

	void OnStatusTagChanged(FGameplayTag Tag, int Count);
	void OnMovementTagChanged(FGameplayTag Tag, int Count);

	void OnJustLanded();
	void OnJustLifted();
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);
	
	void InitializeHeroes();
	
	TSubclassOf<UZodiacCameraMode> DetermineCameraMode();

protected:
	UPROPERTY()
	UZodiacAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UZodiacCameraComponent> CameraComponent;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FZodiacReplicatedAcceleration ReplicatedAcceleration;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UZodiacCameraMode> DefaultAbilityCameraMode;

	UPROPERTY()
	TSubclassOf<UZodiacCameraMode> ActiveAbilityCameraMode;

	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	/** Delegate fired when the ability system component of this actor initialized */
	FOnHostAbilitySystemComponentLoaded OnHostAbilitySystemComponentLoaded;

	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Player Input")
	FZodiacInputConfig InputConfig;

	UPROPERTY(EditDefaultsOnly, Category="Zodiac|Hero")
	TArray<TSubclassOf<AZodiacHero>> HeroClasses;
	
	UPROPERTY(Replicated)
	FZodiacHeroList HeroList;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveHeroIndex, BlueprintReadOnly)
	int32 ActiveHeroIndex = INDEX_NONE;

private:
	UFUNCTION()
	void OnRep_ReplicatedAcceleration();
	
	UFUNCTION()
	void OnRep_ActiveHeroIndex(int32 OldIndex);
};