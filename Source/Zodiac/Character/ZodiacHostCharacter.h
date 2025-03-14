// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "ZodiacCharacter.h"
#include "Camera/ZodiacCameraMode.h"
#include "ZodiacHeroList.h"
#include "Traversal/ZodiacTraversalActorInterface.h"
#include "ZodiacHostCharacter.generated.h"

class UZodiacHeroData;
class UZodiacHealthComponent;

UCLASS(Abstract, BlueprintType)
class ZODIAC_API AZodiacHostCharacter : public AZodiacCharacter, public IZodiacTraversalActorInterface
{
	GENERATED_BODY()

public:
	AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AZodiacCharacter interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetHostAbilitySystemComponent() const;
	virtual void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled) override;
	virtual void SetExtendedMovementModeTag(EZodiacExtendedMovementMode ExtendedMovementMode, bool bTagEnabled) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	virtual USkeletalMeshComponent* GetRetargetedMesh() const override;
	virtual UZodiacHealthComponent* GetHealthComponent() const override;
	//~End of AZodiacCharacter interface
	
	//~IZodiacTraversalActorInterface
	virtual UAbilitySystemComponent* GetTraversalAbilitySystemComponent() const override;
	//~End of IZodiacTraversalActorInterface
	
	UFUNCTION(BlueprintCallable)
	virtual UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent() const;

	/** Delegate fired when the ability system component of this actor initialized */
	FOnAbilitySystemComponentInitialized OnAbilitySystemComponentInitialized;

	// Return currently active hero
	UFUNCTION(BlueprintCallable)
	AZodiacHeroCharacter* GetHero() const { return HeroList.GetHero(ActiveHeroIndex); }

	UFUNCTION(BlueprintPure)
	TArray<AZodiacHeroCharacter*> GetHeroes() const { return HeroList.GetHeroes(); }

	UFUNCTION(BlueprintCallable)
	void ChangeHero(const int32 Index);

	void CheckAllHeroesInitialized();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAllHeroesInitialized();
	
	/** Overrides the camera from an active gameplay ability */
	void SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Clears the camera override if it is set */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

	FVector GetHeroEyeLocationOffset() const { return HeroEyeLocationOffset; }

	void ToggleSprint(bool bShouldSprint);
	
protected:
	//~AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	//~End of AActor interface

	virtual void Input_AbilityInputTagPressed(FGameplayTag InputTag) override;
	virtual void Input_AbilityInputTagReleased(FGameplayTag InputTag) override;

	void InitializeHeroes();
	
	TSubclassOf<UZodiacCameraMode> DetermineCameraMode();

	void UpdateHeroEyeLocationOffset();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UZodiacCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UZodiacCameraMode> DefaultAbilityCameraMode;

	UPROPERTY(EditAnywhere)
	bool bEnableCameraHeroOffset = true;
	
	UPROPERTY()
	TSubclassOf<UZodiacCameraMode> ActiveAbilityCameraMode;

	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Host")
	const UZodiacHeroData* HostData;

	UPROPERTY(EditDefaultsOnly, Category="Zodiac|Hero")
	TArray<TSubclassOf<AZodiacHeroCharacter>> HeroClasses;
	
	UPROPERTY(ReplicatedUsing = OnRep_HeroList)
	FZodiacHeroList HeroList;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveHeroIndex, BlueprintReadOnly)
	int32 ActiveHeroIndex = INDEX_NONE;

private:
	UFUNCTION()
	void OnRep_ActiveHeroIndex(int32 OldIndex);

	UFUNCTION()
	void OnRep_HeroList();
	
	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Hero;

	UPROPERTY(Transient)
	FVector HeroEyeLocationOffset;

	bool bHasHeroInitialized = false;
};