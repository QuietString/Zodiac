// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagAssetInterface.h"
#include "Camera/ZodiacCameraMode.h"
#include "GameFramework/Character.h"
#include "ZodiacPlayerCharacter.generated.h"

class UZodiacCameraComponent;
class UZodiacHeroComponent;
class UHeroCopyPoseMeshData;
struct FInputActionValue;
class UZodiacInputData;
class AZodiacHero;
class UZodiacAbilitySystemComponent;

/**
 * Invisible player controlled character that drives Hero Character
 */
UCLASS()
class ZODIAC_API AZodiacPlayerCharacter : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AZodiacPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface

	UFUNCTION(BlueprintCallable)
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	
	virtual void PostInitializeComponents() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/** Overrides the camera from an active gameplay ability */
	void SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Clears the camera override if it is set */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);
	
	void ChangeHero(int32 NewIndex);
	void ChangeCharacterMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimInstance);
	void ChangeHeroMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimInstance);

	void CheckReady();

public:

	
	
protected:
	
	void InitializeHeroComponents();
	void SelectFirstHero();

	TSubclassOf<UZodiacCameraMode> DetermineCameraMode();
	
	void InitializePlayerInput();

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanJumpInternal_Implementation() const override;

	UFUNCTION()
	void OnRep_ActiveHeroIndex(int32 OldIndex);

protected:
	
	UPROPERTY(VisibleAnywhere, Category = "Zodiac|Heroes")
	UZodiacHeroComponent* HeroComponent1;

	UPROPERTY(VisibleAnywhere, Category = "Zodiac|Heroes")
	UZodiacHeroComponent* HeroComponent2;

	UPROPERTY()
	TArray<TObjectPtr<UZodiacHeroComponent>> HeroComponents;

	UPROPERTY()
	TArray<TObjectPtr<UZodiacAbilitySystemComponent>> AbilitySystemComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac")
	TObjectPtr<UZodiacCameraComponent> CameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac|Heroes")
	TObjectPtr<USkeletalMeshComponent> HeroMeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Player Input")
	UZodiacInputData* InputData;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveHeroIndex, BlueprintReadOnly)
	int32 ActiveHeroIndex;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UZodiacCameraMode> DefaultAbilityCameraMode;

	UPROPERTY()
	TSubclassOf<UZodiacCameraMode> ActiveAbilityCameraMode;

	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;
	
	bool bHeroesInitialized = false;

	bool bReady = false;
};
