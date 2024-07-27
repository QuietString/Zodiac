// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacMessageTypes.generated.h"

class UZodiacSkillSlotWidgetBase;

USTRUCT(BlueprintType)
struct FHeroValueChangedMessage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;
	
	UPROPERTY(BlueprintReadOnly)
	float OldValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float NewValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float OptionalValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FHeroChangedMessage_SkillSlot
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotType = FGameplayTag();
	
	UPROPERTY(BlueprintReadWrite)
	FSlateBrush Brush = FSlateBrush();

	UPROPERTY(BlueprintReadOnly)
	bool bIsReady = false;

	UPROPERTY(BlueprintReadOnly)
	float CurrentValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float OptionalValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	uint8 CurrentStack = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 MaxStack = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 ActivationStack = 0;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer DisplayCostType;
	
	UPROPERTY(BlueprintReadOnly)
	float Cooldown_Remaining = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float Cooldown_Duration = 0.0f;
};

USTRUCT(BlueprintType)
struct FSkillDurationMessage
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotType = FGameplayTag();
	
	UPROPERTY(BlueprintReadWrite)
	float Cooldown_Duration = 0;
};

USTRUCT(BlueprintType)
struct FZodiacSkillCommitMessage_TagStack
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotType = FGameplayTag();
	
	UPROPERTY(BlueprintReadOnly)
	int32 OldValue = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ConsumeAmount = 0;
};

USTRUCT(BlueprintType)
struct FUltimateChargeMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float ChargeAmount = 0;
};

USTRUCT(BlueprintType)
struct FZodiacHUDMessage_SkillSlotChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta=(Categories="HUD.Slot"))
	FGameplayTag Slot;

	UPROPERTY(BlueprintReadOnly)
	int32 HeroIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FZodiacHUDMessage_SlotCreated
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 HeroIndex = INDEX_NONE;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UUserWidget> Widget = nullptr;

	UPROPERTY(BlueprintReadOnly, meta=(Categories="SkillSlot"))
	FGameplayTag Slot;
};

USTRUCT(BlueprintType)
struct FZodiacHUDMessage_HeroChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 HeroIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FZodiacHUDMessage_SlotTagStackChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 HeroIndex = INDEX_NONE;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SlotType = FGameplayTag::EmptyTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ChangedTag = FGameplayTag::EmptyTag;
	
	UPROPERTY(BlueprintReadOnly)
	int32 OldValue = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NewValue = 0;
};

UENUM()
enum class EZodiacAttributeType
{
	None,
	Health,
	Ultimate,
};

USTRUCT(BlueprintType)
struct FZodiacHUDMessage_AttributeChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 HeroIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	int32 OldValue = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NewValue = 0;
};