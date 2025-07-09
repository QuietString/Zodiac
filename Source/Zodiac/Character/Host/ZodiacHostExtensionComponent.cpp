// the.quiet.string@gmail.com


#include "ZodiacHostExtensionComponent.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/Host/ZodiacHostCharacter.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Character/Hero/ZodiacHeroActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostExtensionComponent)

using namespace ZodiacGameplayTags;

void UZodiacHostExtensionComponent::HandleControllerChanged()
{
	Super::HandleControllerChanged();

	if (AZodiacHostCharacter* HostCharacter = GetPawn<AZodiacHostCharacter>())
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			Hero->CheckDefaultInitialization();
		}	
	}
}
