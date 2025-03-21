// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ZodiacHeroAnimInstance2.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacHeroAnimInstance2 : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> RetargetSourceMesh;
};
