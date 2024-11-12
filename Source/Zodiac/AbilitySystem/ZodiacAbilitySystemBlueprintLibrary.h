// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/ZodiacGameplayAbility_Death.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZodiacAbilitySystemBlueprintLibrary.generated.h"


UCLASS(meta=(ScriptName="ZodiacAbilitySystemLibrary"))
class ZODIAC_API UZodiacAbilitySystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Ability, Meta = (Tooltip = "This function can be used to trigger an ability on the actor in question with useful payload data."))
	static void SendGameplayEventToActorNotPredicted(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload);
};
