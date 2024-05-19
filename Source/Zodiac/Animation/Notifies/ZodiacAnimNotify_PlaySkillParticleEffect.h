// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlayParticleEffect.h"
#include "ZodiacAnimNotify_PlaySkillParticleEffect.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacAnimNotify_PlaySkillParticleEffect : public UAnimNotify_PlayParticleEffect
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
