// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "ZodiacCharacter.h"
#include "Camera/ZodiacCameraMode.h"
#include "ZodiacHeroList.h"
#include "ZodiacHostCharacter.generated.h"

class UZodiacHeroData;
class UZodiacHealthComponent;

UCLASS(Abstract)
class ZODIAC_API AZodiacHostCharacter : public AZodiacCharacter
{
	GENERATED_BODY()

public:
	AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetHostAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable)
	virtual UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent() const;
	
	virtual UZodiacHealthComponent* GetHealthComponent() const override;

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

	virtual void OnJustLanded() override;

	void InitializeHeroes();
	
	TSubclassOf<UZodiacCameraMode> DetermineCameraMode();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UZodiacCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UZodiacCameraMode> DefaultAbilityCameraMode;

	UPROPERTY()
	TSubclassOf<UZodiacCameraMode> ActiveAbilityCameraMode;

	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Host")
	const UZodiacHeroData* HostData;

	UPROPERTY(EditDefaultsOnly, Category="Zodiac|Hero")
	TArray<TSubclassOf<AZodiacHero>> HeroClasses;
	
	UPROPERTY(Replicated)
	FZodiacHeroList HeroList;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveHeroIndex, BlueprintReadOnly)
	int32 ActiveHeroIndex = INDEX_NONE;

private:
	UFUNCTION()
	void OnRep_ActiveHeroIndex(int32 OldIndex);
};