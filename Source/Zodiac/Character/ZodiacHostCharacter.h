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

UENUM(BlueprintType)
enum EZodiacGait
{
	Gait_Walk,
	Gait_Run,
	Gait_Sprint
};

class UZodiacHealthComponent;

UCLASS(Abstract)
class ZODIAC_API AZodiacHostCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();
	virtual UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent();
	void InitializeHostAbilitySystem(UAbilitySystemComponent* InASC);

	void CallOrRegister_OnAbilitySystemInitialized(FOnHostAbilitySystemComponentLoaded::FDelegate&& Delegate);
	UZodiacHealthComponent* GetCurrentHeroHealthComponent();

	void ChangeHero(const int32 Index);
	
	/** Overrides the camera from an active gameplay ability */
	void SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Clears the camera override if it is set */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

protected:
	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	//~End of AActor interface

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void InitializePlayerInput();
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);

	void OnAimingTagChanged(FGameplayTag Tag, int Count);
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);
	
	void InitializeHeroes();

	
	TSubclassOf<UZodiacCameraMode> DetermineCameraMode();

protected:
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UZodiacCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UZodiacCameraMode> DefaultAbilityCameraMode;

	UPROPERTY()
	TSubclassOf<UZodiacCameraMode> ActiveAbilityCameraMode;

	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

protected:
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
	
	UFUNCTION()
	void OnRep_ActiveHeroIndex(int32 OldIndex);
};