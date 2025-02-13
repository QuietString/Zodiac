﻿// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagAssetInterface.h"
#include "ZodiacHeroComponent.h"
#include "Camera/ZodiacCameraMode.h"
#include "GameFramework/Character.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacPlayerCharacter.generated.h"

class UZodiacHealthComponent;
class AZodiacPlayerState;
struct FGenericTeamId;
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
class ZODIAC_API AZodiacPlayerCharacter : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface, public IZodiacTeamAgentInterface
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

	UFUNCTION(BlueprintCallable)
	TArray<UZodiacHeroComponent*> GetHeroComponents() { return HeroComponents ; }

	UFUNCTION(BlueprintCallable)
	UZodiacHeroComponent* GetCurrentHeroComponent() { return HeroComponents.IsValidIndex(ActiveHeroIndex) ? HeroComponents[ActiveHeroIndex] : nullptr; }

	UFUNCTION(BlueprintCallable)
	UZodiacHealthComponent* GetCurrentHealthComponent() { return HeroComponents.IsValidIndex(ActiveHeroIndex) ? HeroComponents[ActiveHeroIndex]->GetHealthComponent() : nullptr; }

	UFUNCTION(BlueprintCallable)
	void SetModularMesh(TSubclassOf<USkeletalMeshComponent> NewMeshCompClass, FName Socket);

	UFUNCTION(BlueprintCallable)
	void ClearModularMesh();
	
	//~AActor interface
	virtual void PostRegisterAllComponents() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor Interface
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//~IZodiacTeamAgentInterface interface
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	//~End of IZodiacTeamAgentInterface interface
	
	/** Overrides the camera from an active gameplay ability */
	void SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Clears the camera override if it is set */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);
	
	void ChangeHero(const int32 NewIndex, const bool bForced = false);
	void ChangeCharacterMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimInstance);
	void ChangeHeroMesh(USkeletalMesh* NewMesh);

	void OnHeroChanged(UZodiacHeroComponent* NewHeroComponent);

protected:
	
	void InitializeHeroComponents();

	TSubclassOf<UZodiacCameraMode> DetermineCameraMode();
	
	void InitializePlayerInput();

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);

	void OnSprintTagChanged(FGameplayTag Tag, int Count);
	virtual bool CanJumpInternal_Implementation() const override;

protected:
	
	UFUNCTION()
	void OnRep_ActiveHeroIndex(int32 OldIndex);

public:
	UPROPERTY()
	bool bIsSprinting = false;
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac|Hero")
	TObjectPtr<USkeletalMeshComponent> ModularMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Player Input")
	UZodiacInputData* InputData;

	// Reduce left/right movement input amount when sprinting
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Player Input")
	float SprintWeight = 0.3f;
	
	UPROPERTY(ReplicatedUsing=OnRep_ActiveHeroIndex, BlueprintReadOnly)
	int32 ActiveHeroIndex;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UZodiacCameraMode> DefaultAbilityCameraMode;

	UPROPERTY()
	TSubclassOf<UZodiacCameraMode> ActiveAbilityCameraMode;

	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	UPROPERTY()
	EZodiacTeam MyTeam;

	UPROPERTY()
	UZodiacHealthComponent* CurrentHealthComponent;
	
private:
	bool bHeroesInitialized = false;
};
