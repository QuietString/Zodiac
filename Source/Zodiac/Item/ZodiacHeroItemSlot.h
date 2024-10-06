// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "System/GameplayTagStack.h"
#include "UObject/Object.h"
#include "ZodiacHeroItemSlot.generated.h"

class UZodiacSkillSlotWidgetBase;
class UZodiacAbilitySet;
class UZodiacHeroItemSlot;

USTRUCT(BlueprintType)
struct FZodiacHeroItemDefinition
{
	GENERATED_BODY()

public:
	template<typename T = UZodiacHeroItemSlot>
	T* CreateInstance(AActor* InOwner) const;

public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="HUD.Type.SkillSlot"))
	FGameplayTag SlotType;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UZodiacAbilitySet> SkillSetToGrant;
	
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Ability.Cost.Stack"))
	TMap<FGameplayTag, int32> InitialTagStack;
};

template <typename T>
T* FZodiacHeroItemDefinition::CreateInstance(AActor* InOwner) const
{
	T* Item = NewObject<T>(InOwner, T::StaticClass());
	Item->InitializeItem(*this);
	
	return Item;
}

/**
 * Contains data for associated abilities of a slot. 
 */
UCLASS(BlueprintType)
class ZODIAC_API UZodiacHeroItemSlot : public UObject
{
	GENERATED_BODY()

public:
	UZodiacHeroItemSlot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface

	// Set up data, authority only.
	virtual void InitializeItem(const FZodiacHeroItemDefinition& InData);

	FGameplayTag GetSlotType() const { return SlotType; }
	const FZodiacHeroItemDefinition& GetSlotDefinition() const { return Definition; }

	template <typename ResultClass>
	const ResultClass* FindFragment() const { return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass()); }
	
	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetStatTagStack(FGameplayTag Tag, int32 StackCount);
	
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	UPROPERTY(Replicated)
	FZodiacAbilitySet_GrantedHandles GrantedHandles;

protected:
	UPROPERTY()
	FZodiacHeroItemDefinition Definition;

	UPROPERTY(Replicated)
	FGameplayTag SlotType;

	UPROPERTY()	
	TObjectPtr<UZodiacSkillSlotWidgetBase> Widget;
	
private:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTag;
};