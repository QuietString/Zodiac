// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacGameplayEffectContext.h"

#include "AbilitySystem/ZodiacAbilitySourceInterface.h"
#include "Engine/HitResult.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayEffectContext)

class FArchive;

FZodiacGameplayEffectContext* FZodiacGameplayEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FZodiacGameplayEffectContext::StaticStruct()))
	{
		return (FZodiacGameplayEffectContext*)BaseEffectContext;
	}

	return nullptr;
}

bool FZodiacGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}

void FZodiacGameplayEffectContext::SetAbilitySource(const IZodiacAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
	//SourceLevel = InSourceLevel;
}

const IZodiacAbilitySourceInterface* FZodiacGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IZodiacAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FZodiacGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}

