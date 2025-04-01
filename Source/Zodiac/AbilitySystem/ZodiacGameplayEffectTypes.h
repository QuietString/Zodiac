// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/WeakObjectPtr.h"
#include "ZodiacGameplayEffectTypes.generated.h"

class UAbilitySystemComponent;
class FProperty;

/**
 * A single mapping from "Bool Property in a Blueprint" --> "Gameplay Tag in ASC".
 * If the bool is true, we ensure the tag is present; if false, remove it.
 */
USTRUCT()
struct FGameplayTagBlueprintPropertyReverseMapping
{
    GENERATED_BODY()

public:
    /** Gameplay tag to add/remove based on the bool property. */
    UPROPERTY(EditAnywhere, Category="ReverseTagMap")
    FGameplayTag Tag;

    /** The name of the bool property we read from the Blueprint owner. */
    UPROPERTY(EditAnywhere, Category="ReverseTagMap")
    FName PropertyName;

    // --------------------------------------------------
    // Runtime / Internal
    // --------------------------------------------------

    /** We store a pointer to the FBoolProperty once we find it (so we don't do the find each tick). */
    TFieldPath<FProperty> FoundProperty;

    /** The last known value we read from that property. */
    bool bLastValue = false;
};

/**
 * A container that manages multiple "property->tag" mappings. 
 * You call Initialize() once, then TickUpdateProperties() each frame (or as needed).
 */
USTRUCT()
struct FGameplayTagBlueprintPropertyMapReverse
{
    GENERATED_BODY()

public:
    /** The array of property->tag bindings. Fill this in your Blueprint or C++ defaults. */
    UPROPERTY(EditAnywhere, Category="ReverseTagMap")
    TArray<FGameplayTagBlueprintPropertyReverseMapping> PropertyMappings;

    /**
     * Call this to initialize the struct with a particular Owner (the AnimInstance or Actor) and its ASC.
     * Afterward, call TickUpdateProperties() each frame to apply changes to the ASC.
     */
    void Initialize(UObject* Owner, UAbilitySystemComponent* ASC);

    /**
     * Each tick (or as needed), read each bool property. If it changed, add/remove the tag.
     * This is a “pull” approach – we have no automatic property‐changed delegate in UE for arbitrary BPs,
     * so we have to check each frame.
     */
    void TickUpdateProperties();

    /** Call when you’re done, or to reset everything. Removes any leftover references. */
    void Unregister();

protected:
    TWeakObjectPtr<UObject> CachedOwner;
    TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};