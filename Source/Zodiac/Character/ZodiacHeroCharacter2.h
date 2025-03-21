// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "ZodiacCharacter.h"
#include "ZodiacHeroCharacter2.generated.h"

class UZodiacCameraComponent;
class UZodiacCameraMode;
class UZodiacHealthComponent;

UCLASS()
class ZODIAC_API AZodiacHeroCharacter2 : public AZodiacCharacter
{
	GENERATED_BODY()

public:
	AZodiacHeroCharacter2(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AZodiacCharacter interface
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacHealthComponent* GetHealthComponent() const override { return HealthComponent; }
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	virtual USkeletalMeshComponent* GetRetargetedMesh() const override { return RetargetSourceMesh; }
	
	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner) override;
	//~End of AZodiacCharacter interface

	virtual USkeletalMeshComponent* GetRetargetSourceMesh() const { return RetargetSourceMesh; }
	
	virtual void BeginPlay() override;
	virtual void PossessedBy(class AController* NewController) override;
	virtual void PostInitializeComponents() override;
	
	void ToggleSprint(bool bShouldSprint);

	TSubclassOf<UZodiacCameraMode> DetermineCameraMode();

	void UpdateHeroEyeLocationOffset();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnCloseContactStarted();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnCloseContactFinished();
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UZodiacCameraMode> DefaultAbilityCameraMode;
	
	UPROPERTY()
	TSubclassOf<UZodiacCameraMode> ActiveAbilityCameraMode;

	UPROPERTY(EditAnywhere)
	bool bEnableCameraHeroOffset = true;

	
	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> RetargetSourceMesh;
	
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacCameraComponent> CameraComponent;

	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Hero;
};
