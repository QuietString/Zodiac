// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "System/GameplayTagStack.h"
#include "UObject/Object.h"
#include "ZodiacSkillSlot.generated.h"

class UZodiacSkillSlotDefinition;
class UZodiacSkillSlotFragment;
class UZodiacAbilitySystemComponent;
class UZodiacSkillSlot;
class UZodiacSkillSet;

DECLARE_DELEGATE_OneParam(FOnTagStackChanged, UZodiacSkillSlot* Slot);
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

	const UZodiacSkillSlotFragment* FindFragmentByClass(TSubclassOf<UZodiacSkillSlotFragment> FragmentClass) const;
	FGameplayTag GetSlotType() const { return SlotType; }
	const UZodiacSkillSlotDefinition* GetSlotDefinition() const { return SlotDefinition; }
	void SetSlotDefinition(const UZodiacSkillSlotDefinition* InSlotDefinition) { SlotDefinition = InSlotDefinition; }
	void SetSlotType(const FGameplayTag InSlotType) { SlotType = InSlotType; }
	
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

protected:
	UFUNCTION()
	void OnRep_StatTag();
	
public:
	FOnTagStackChanged OnTagStackChanged;

	UPROPERTY(Replicated)
	FZodiacAbilitySet_GrantedHandles GrantedHandles;
	
protected:
	UPROPERTY(Replicated)
	const UZodiacSkillSlotDefinition* SlotDefinition = nullptr;

	UPROPERTY(Replicated)
	FGameplayTag SlotType;
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_StatTag)
	FGameplayTagStackContainer StatTag;
};