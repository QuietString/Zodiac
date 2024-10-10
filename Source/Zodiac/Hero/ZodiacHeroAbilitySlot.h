// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZodiacHeroAbilityDefinition.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "System/GameplayTagStack.h"
#include "UObject/Object.h"
#include "ZodiacHeroAbilitySlot.generated.h"

/**
 * Contains data for associated abilities of a slot. 
 */
UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API UZodiacHeroAbilitySlot : public UObject
{
	GENERATED_BODY()

public:
	UZodiacHeroAbilitySlot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface

	virtual void Tick(float DeltaTime) {};
	virtual void InitializeSlot(const FZodiacHeroAbilityDefinition& InDef);

	UFUNCTION(BlueprintCallable)
	virtual void UpdateActivationTime();
	
	UFUNCTION(BlueprintPure)
	APawn* GetPawn() const;

	FGameplayTag GetSlotType() const { return SlotType; }
	const FZodiacHeroAbilityDefinition& GetSlotDefinition() const { return Definition; }

	const UZodiacHeroAbilityFragment* FindFragmentByClass(const TSubclassOf<UZodiacHeroAbilityFragment>& FragmentClass) const;
	
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
	FZodiacHeroAbilityDefinition Definition;
	
	UPROPERTY(Replicated)
	FGameplayTag SlotType;

private:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTag;

	double TimeLastFired = 0.0;
};
