// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "ZodiacHeroSkeletalMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacHeroSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	virtual bool ShouldTickPose() const override
	{
		return (Super::ShouldTickPose() && !bIsHeroHidden);
	}
	
	bool bIsHeroHidden = true;
};
