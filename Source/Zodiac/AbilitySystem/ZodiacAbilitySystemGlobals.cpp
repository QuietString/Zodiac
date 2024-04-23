// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacAbilitySystemGlobals.h"

#include "ZodiacGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilitySystemGlobals)

struct FGameplayEffectContext;

UZodiacAbilitySystemGlobals::UZodiacAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplayEffectContext* UZodiacAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FZodiacGameplayEffectContext();
}

