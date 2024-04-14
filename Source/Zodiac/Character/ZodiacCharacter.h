// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagAssetInterface.h"
#include "ZodiacCharacter.generated.h"

struct FInputActionValue;
class UZodiacPawnData;
class UInputMappingContext;
class UZodiacHeroComponent;
class UZodiacPawnExtensionComponent;
class UZodiacHealthComponent;
class UZodiacHealthSet;
class AZodiacPlayerState;
class UZodiacAbilitySystemComponent;
class UAbilitySystemComponent;

UCLASS()
class ZODIAC_API AZodiacCharacter : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface 
{
	GENERATED_BODY()

public:
	AZodiacCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	AZodiacPlayerState* GetZodiacPlayerState() const;
	
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//~IGameplayTagAssetInterface interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface interface

protected:

	void AddDefaultAbilities(UZodiacAbilitySystemComponent* ZodiacASC);
	void OnManaChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	virtual void OnAbilitySystemInitialized();
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;

	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	void InitializePlayerInput();

	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Zodiac|Input")
	UZodiacPawnData* PawnData;
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac|Character", meta = (AllowPrivateAccess = true))
	UZodiacPawnExtensionComponent* PawnExtComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac|Character", meta = (AllowPrivateAccess = true))
	UZodiacHeroComponent* HeroComponent;
	
	UPROPERTY()
	UZodiacAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zodiac|Character", meta = (AllowPrivateAccess = true))
	UZodiacHealthComponent* HealthComponent;
	
	UPROPERTY()
	const UZodiacHealthSet* HealthSet;
};
