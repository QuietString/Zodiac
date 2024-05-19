// the.quiet.string@gmail.com


#include "ZodiacAnimNotify_PlaySkillParticleEffect.h"

#include "NativeGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/ZodiacPlayerCharacter.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_EVENT_SKILL_SOCKET_UPDATED, "Event.Skill.Socket.Updated");

void UZodiacAnimNotify_PlaySkillParticleEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                       const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(MeshComp->GetOwner()))
    		{
    			if (UZodiacAbilitySystemComponent* ZodiacASC = PlayerCharacter->GetZodiacAbilitySystemComponent())
    			{
    				ZodiacASC->SetMuzzleSocketData(SocketName);
    				ZodiacASC->CheckAndExecuteGameplayCue();
    			}
    		}	
	}
	
	Super::Notify(MeshComp, Animation, EventReference);
}
