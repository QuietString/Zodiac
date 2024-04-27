// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagAssetInterface.h"
#include "InputActionValue.h"
#include "ZodiacCharacter.generated.h"

class UZodiacInputData;
class UZodiacAbilitySet;
class UZodiacCharacterChangeComponent;
class AZodiacTaggedActor;
class UZodiacHeroData;
struct FInputActionValue;
class UInputMappingContext;
class UZodiacHeroComponent;
class UZodiacPawnExtensionComponent;
class UZodiacHealthComponent;
class AZodiacPlayerState;
class UZodiacAbilitySystemComponent;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FZodiacCharacterChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	int32 SlotIndex;
};

UCLASS()
class ZODIAC_API AZodiacCharacter : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface 
{
	GENERATED_BODY()

public:
	AZodiacCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AActor interface
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	//~End of AActor interface

	UFUNCTION(BlueprintCallable)
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetRetargetedMeshComponent();

	UFUNCTION(BlueprintCallable)
	UZodiacCharacterChangeComponent* GetCharacterChangeComponent();

	//~IGameplayTagAssetInterface interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

	TArray<UZodiacHeroData*> GetHeroes();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void InitializeAbilitySystemComponent();
	void InitializePlayerInput();

	void AddStartingAbilities();
	void OnManaChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual bool CanJumpInternal_Implementation() const override;

	UFUNCTION(BlueprintCallable)
	void Input_ChangeCharacter(const int32 NewSlotIndex, const FGameplayTag SlotActionTag);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Player Input")
	UZodiacInputData* InputData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Ability")
	TArray<TObjectPtr<UZodiacAbilitySet>> StartingAbilities;
	
private:

	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac|Character", meta = (AllowPrivateAccess = true))
	UZodiacHealthComponent* HealthComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac|Heroes", meta=(AllowPrivateAccess=true))
	TObjectPtr<USkeletalMeshComponent> RetargetedMeshComponent;

	UPROPERTY()
	UZodiacCharacterChangeComponent* CharacterChangeComponent;

	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UZodiacHealthSet> HealthSet;
	
	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UZodiacCombatSet> CombatSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zodiac|Heroes", meta = (AllowPrivateAccess = true))
	TArray<UZodiacHeroData*> Heroes;
};
