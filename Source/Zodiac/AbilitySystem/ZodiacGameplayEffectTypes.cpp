// the.quiet.string@gmail.com


#include "ZodiacGameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayEffectTypes)

void FGameplayTagBlueprintPropertyMapReverse::Initialize(UObject* Owner, UAbilitySystemComponent* ASC)
{
    Unregister();

    if (!Owner || !ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("FGameplayTagBlueprintPropertyMapReverse::Initialize called with invalid Owner or ASC."));
        return;
    }

    CachedOwner = Owner;
    CachedASC   = ASC;

    UClass* OwnerClass = Owner->GetClass();
    for (FGameplayTagBlueprintPropertyReverseMapping& Mapping : PropertyMappings)
    {
        // Find the property in the owner's class
        FProperty* FoundProp = OwnerClass->FindPropertyByName(Mapping.PropertyName);
        if (FoundProp)
        {
            // We only support boolean in this example
            if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(FoundProp))
            {
                Mapping.FoundProperty = FoundProp;

                // Initialize bLastValue from the property’s current value
                bool bCurrentValue = BoolProp->GetPropertyValue_InContainer(Owner);
                Mapping.bLastValue = bCurrentValue;

                // If it's currently true, ensure the tag is present
                if (bCurrentValue && Mapping.Tag.IsValid())
                {
                    ASC->AddLooseGameplayTag(Mapping.Tag);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("FGameplayTagBlueprintPropertyMapReverse: Property '%s' is not a bool! (Owner: %s)"),
                    *Mapping.PropertyName.ToString(), *GetNameSafe(Owner));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("FGameplayTagBlueprintPropertyMapReverse: Could not find property '%s' in Owner '%s'"),
                *Mapping.PropertyName.ToString(), *GetNameSafe(Owner));
        }
    }
}

void FGameplayTagBlueprintPropertyMapReverse::TickUpdateProperties()
{
    UObject* Owner   = CachedOwner.Get();
    UAbilitySystemComponent* ASC = CachedASC.Get();
    if (!Owner || !ASC)
    {
        return;
    }

    for (FGameplayTagBlueprintPropertyReverseMapping& Mapping : PropertyMappings)
    {
        // If we successfully found a bool property
        if (FProperty* Prop = Mapping.FoundProperty.Get())
        {
            if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
            {
                bool bCurrentValue = BoolProp->GetPropertyValue_InContainer(Owner);

                // Compare to the last known value
                if (bCurrentValue != Mapping.bLastValue)
                {
                    // The value changed => update ASC
                    if (Mapping.Tag.IsValid())
                    {
                        if (bCurrentValue)
                        {
                            // Was false, now true => add tag
                            ASC->AddLooseGameplayTag(Mapping.Tag);
                        }
                        else
                        {
                            // Was true, now false => remove tag
                            ASC->RemoveLooseGameplayTag(Mapping.Tag);
                        }
                    }

                    Mapping.bLastValue = bCurrentValue;
                }
            }
        }
    }
}

void FGameplayTagBlueprintPropertyMapReverse::Unregister()
{
    // If we wanted, we could remove any leftover tags that are "stuck" from previous sessions, etc.
    // For now, do nothing but clear references
    CachedOwner.Reset();
    CachedASC.Reset();

    for (FGameplayTagBlueprintPropertyReverseMapping& Mapping : PropertyMappings)
    {
        Mapping.FoundProperty = nullptr;
        Mapping.bLastValue    = false;
    }
}