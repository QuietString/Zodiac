// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacAttributeSet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"


UZodiacAttributeSet::UZodiacAttributeSet()
{
}

UWorld* UZodiacAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UZodiacAbilitySystemComponent* UZodiacAttributeSet::GetZodiacAbilitySystemComponent() const
{
	return Cast<UZodiacAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
