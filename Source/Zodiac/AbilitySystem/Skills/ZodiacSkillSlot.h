// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacSkillSlotDefinition.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "System/GameplayTagStack.h"
#include "UObject/Object.h"
#include "ZodiacSkillSlot.generated.h"

class UZodiacSkillSlotDefinition;
class UZodiacSkillSlotFragment;
class UZodiacAbilitySystemComponent;
class UZodiacSkillSlot;
class UZodiacSkillSet;

/**
 * 
 */
UCLASS(BlueprintType)
class ZODIAC_API UZodiacSkillSlot : public UObject
{
	GENERATED_BODY()

public:
	UZodiacSkillSlot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface

	// Set up slot data, authority only.
	void InitializeSlot(const UZodiacSkillSlotDefinition* InDef, FGameplayTag InType);
	
	void CreateSlotWidget();
	UUserWidget* GetSlotWidget();
	
	FGameplayTag GetSlotType() const { return SlotType; }
	const UZodiacSkillSlotDefinition* GetSlotDefinition() const { return Definition; }

	template <typename ResultClass>
	const ResultClass* FindFragment() const { return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass()); }
	
	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	UPROPERTY(Replicated)
	FZodiacAbilitySet_GrantedHandles GrantedHandles;

	UFUNCTION()
	void OnTagStackChanged_Internal(FGameplayTag Tag, const int32 OldValue, const int32 NewValue);

public:
	// Create slot widget instance.
	UFUNCTION()
	void OnRep_Definition();
	
protected:
	UFUNCTION()
	void OnRep_StatTag();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Definition)
	const UZodiacSkillSlotDefinition* Definition = nullptr;

	UPROPERTY(Replicated)
	FGameplayTag SlotType;

	UPROPERTY()
	TObjectPtr<UZodiacSkillSlotWidgetBase> Widget;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_StatTag)
	FGameplayTagStackContainer StatTag;
};