// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacIndicatorManagerComponent.h"

#include "IndicatorDescriptor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacIndicatorManagerComponent)

UZodiacIndicatorManagerComponent::UZodiacIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRegister = true;
	bAutoActivate = true;
}

UZodiacIndicatorManagerComponent* UZodiacIndicatorManagerComponent::GetComponent(AController* Controller)
{
	if (Controller)
	{
		return Controller->FindComponentByClass<UZodiacIndicatorManagerComponent>();
	}

	return nullptr;
}

void UZodiacIndicatorManagerComponent::AddIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	IndicatorDescriptor->SetIndicatorManagerComponent(this);
	OnIndicatorAdded.Broadcast(IndicatorDescriptor);
	Indicators.Add(IndicatorDescriptor);
}

void UZodiacIndicatorManagerComponent::RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	if (IndicatorDescriptor)
	{
		ensure(IndicatorDescriptor->GetIndicatorManagerComponent() == this);
	
		OnIndicatorRemoved.Broadcast(IndicatorDescriptor);
		Indicators.Remove(IndicatorDescriptor);
	}
}
