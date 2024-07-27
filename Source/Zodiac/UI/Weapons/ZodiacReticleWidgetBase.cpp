// the.quiet.string@gmail.com


#include "ZodiacReticleWidgetBase.h"

UZodiacReticleWidgetBase::UZodiacReticleWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

// void UZodiacReticleWidgetBase::InitializeFromHero(UZodiacHeroComponent* InHeroComponent)
// {
// 	//HeroComponent = InHeroComponent;
//
// 	OnHeroInitialized();
// }

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
