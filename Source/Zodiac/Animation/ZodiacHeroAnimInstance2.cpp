// the.quiet.string@gmail.com


#include "ZodiacHeroAnimInstance2.h"

#include "Character/ZodiacHeroCharacter2.h"

void UZodiacHeroAnimInstance2::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AZodiacHeroCharacter2* Hero = Cast<AZodiacHeroCharacter2>(GetOwningActor()))
	{
		RetargetSourceMesh = Hero->GetRetargetSourceMesh();
	}
}
