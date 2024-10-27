// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "ZodiacHeroData.h"
#include "Components/ActorComponent.h"
#include "ZodiacHeroAbilityManagerComponent.generated.h"

class UZodiacAbilitySlotWidgetBase;
class UZodiacAbilitySystemComponent;
class UZodiacHeroAbilitySlot_RangedWeapon;
class UZodiacHeroAbilitySlot_Weapon;
class UAbilitySystemComponent;
class UZodiacHeroItemSlot;
class UZodiacWeaponSlot;
class AZodiacHeroCharacter;
class AZodiacHostCharacter;
class UZodiacHeroData;
class UZodiacReticleWidgetBase;
class UZodiacHealthComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthComponentInitialized, UZodiacHealthComponent*);

USTRUCT(BlueprintType, DisplayName = "HUD Message Attribute Value Changed")
struct FZodiacHUDMessage_AttributeValueChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AController> Controller = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Hero = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayAttribute Attribute;
	
	UPROPERTY(BlueprintReadWrite)
	float OldValue = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float NewValue = 0.0f;
};


USTRUCT(BlueprintType, DisplayName = "HUD Message Widget Changed")
struct FZodiacHUDMessage_WidgetChangedBatch
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AController> Controller = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TMap<TObjectPtr<UZodiacHeroAbilitySlot>, TSubclassOf<UZodiacAbilitySlotWidgetBase>> Widgets;
};

USTRUCT(BlueprintType, DisplayName = "HUD Message Widget Changed")
struct FZodiacHUDMessage_WidgetChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AController> Controller = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UZodiacHeroAbilitySlot> Slot;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<TSubclassOf<UZodiacAbilitySlotWidgetBase>> Widgets;
};

USTRUCT(BlueprintType, DisplayName = "HUD Message Reticle Changed")
struct FZodiacHUDMessage_ReticleChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AController> Controller = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UZodiacHeroAbilitySlot> Slot;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> Widgets;
};

USTRUCT(BlueprintType, DisplayName = "HUD Message Health Changed")
struct FZodiacHUDMessage_HealthChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AZodiacHeroCharacter> Hero = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsHeroActive = false;

	UPROPERTY(BlueprintReadWrite)
	float MaxValue = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	float OldValue = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	float NewValue = 0.0f;
};

// Responsible for communication between HUD widgets to display info like health, skill slot and etc.
UCLASS(BlueprintType)
class ZODIAC_API UZodiacHeroAbilityManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZodiacHeroAbilityManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ UActorComponent interface
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End of UActorComponent interface

	AController* GetHostController();
	
	void InitializeWithAbilitySystem(UZodiacAbilitySystemComponent* InAbilitySystemComponent, const UZodiacHeroData* InHeroData);
	void BindMessageDelegates();

	void OnHeroActivated();
	void OnHeroDeactivated();

protected:
	void SendChangeReticleMessage(const TArray<TSubclassOf<UZodiacReticleWidgetBase>>& Widgets, UZodiacHeroAbilitySlot* Slot);
	void SendChangeWidgetMessage(const TArray<TSubclassOf<UZodiacAbilitySlotWidgetBase>>& Widgets, UZodiacHeroAbilitySlot* Slot);
	void SendChangeSlotWidgetsMessage(TMap<TObjectPtr<UZodiacHeroAbilitySlot>, TSubclassOf<UZodiacAbilitySlotWidgetBase>> Widgets);
	
	void ClearAbilityReticle();

	void HandleUltimateChanged(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec* EffectSpec, float Magnitude, float OldValue, float NewValue);
	
	UFUNCTION()
	void SendChangeHealthMessage(UZodiacHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);
	void SendAttributeValueChangedMessage(const FOnAttributeChangeData& OnAttributeChangeData);

private:
	UPROPERTY()
	const UZodiacHeroData* HeroData;

	UPROPERTY()
	UZodiacAbilitySystemComponent* AbilitySystemComponent;
	
	bool bIsHeroActive = false;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<UZodiacHeroAbilitySlot>> Slots;
};