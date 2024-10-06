// the.quiet.string@gmail.com


#include "ZodiacReticleWidgetBase.h"

#include "Item/ZodiacWeaponSlot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacReticleWidgetBase)

UZodiacReticleWidgetBase::UZodiacReticleWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

float UZodiacReticleWidgetBase::ComputeSpreadAngle() const
{
	return 0.0f;
}

float UZodiacReticleWidgetBase::ComputeMaxScreenspaceSpreadRadius() const
{
	return 0.0f;
}

bool UZodiacReticleWidgetBase::HasFirstShotAccuracy() const
{
	return true;
}