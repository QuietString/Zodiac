// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"
#include "ZodiacPlayerCharacter.generated.h"

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

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//void InitializeHeroes();
	
	void ChangeMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimInstance);
	
	UFUNCTION(BlueprintCallable)
	void ChangeHero(int32 NewHeroIndex);

	UFUNCTION(BlueprintCallable)
	void SetActiveHeroIndex(int32 NewIndex);
	
protected:

	void InitializeHeroComponents();
	void SelectFirstHero();
	
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
	
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Player Input")
	UZodiacInputData* InputData;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveHeroIndex, BlueprintReadOnly)
	int32 ActiveHeroIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac|Heroes")
	TObjectPtr<USkeletalMeshComponent> HeroMeshComponent;

	TArray<TObjectPtr<UZodiacAbilitySystemComponent>> AbilitySystemComponents;
	
	UPROPERTY()
	bool bInitialized = false;
};
