// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZodiacHeroAbilityDefinition.h"
#include "ZodiacHeroAbilitySlotDefinition.h"
#include "AbilitySystem/ZodiacAbilitySourceInterface.h"
#include "System/GameplayTagStack.h"
#include "UI/Weapons/ZodiacReticleWidgetBase.h"
#include "UObject/Object.h"
#include "ZodiacHeroAbilitySlot.generated.h"

class AZodiacHostCharacter;
class AZodiacHeroAbilitySlotActor;
class UZodiacHeroAbilitySlotDefinition;
DECLARE_DELEGATE_TwoParams(FOnReticleChanged, const TArray<TSubclassOf<UZodiacReticleWidgetBase>>&, UZodiacHeroAbilitySlot*);

/**
 * Contains data for associated abilities of a slot. 
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class ZODIAC_API UZodiacHeroAbilitySlot : public UObject, public IZodiacAbilitySourceInterface
{
	GENERATED_BODY()

public:
	UZodiacHeroAbilitySlot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface
	
	//~IZodiacAbilitySourceInterface interface
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const override { return 1.0f; }
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const override { return 1.0f; }
	//~End of IZodiacAbilitySourceInterface interface

	virtual void Tick(float DeltaTime) {}
	virtual void InitializeSlot(const UZodiacHeroAbilitySlotDefinition* InDef);

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetSlotType() const;
	
	UFUNCTION(BlueprintCallable)
	virtual void UpdateActivationTime();

	UFUNCTION(BlueprintPure)
	TArray<AZodiacHeroAbilitySlotActor*> GetSpawnedActors() const { return SpawnedActors; }
	
	void ChangeReticle();
	void ClearReticle();
	
	UFUNCTION(BlueprintPure)
	APawn* GetPawn() const
	{
		return Cast<APawn>(GetOuter());
	}
	
	template<typename T>
	T* GetPawn() const
	{
		return Cast<T>(GetOuter());
	}

	UFUNCTION(BlueprintPure)
	AZodiacHostCharacter* GetHostCharacter() const;
	
	const UZodiacHeroAbilitySlotDefinition* GetSlotDefinition() const { return Definition; }

	const UZodiacHeroAbilityFragment* FindFragmentByClass(const TSubclassOf<UZodiacHeroAbilityFragment>& FragmentClass) const;

	template<typename T>
	T* FindFragmentByClass() const
	{
		static_assert(TIsDerivedFrom<T, UZodiacHeroAbilityFragment>::IsDerived, "T must be derived from UZodiacHeroAbilityFragment");

		for (UZodiacHeroAbilityFragment* Fragment : Definition->Fragments)
		{
			if (Fragment)
			{
				T* TypedFragment = Cast<T>(Fragment);
				if (TypedFragment)
				{
					return TypedFragment;
				}
			}
		}
		return nullptr;
	}
	
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

	FOnReticleChanged OnReticleApplied;
	FSimpleDelegate OnReticleCleared;
	
protected:
	UPROPERTY(Replicated)
	const UZodiacHeroAbilitySlotDefinition* Definition;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<AZodiacHeroAbilitySlotActor>> SpawnedActors;

private:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTag;

	double TimeLastFired = 0.0;
};