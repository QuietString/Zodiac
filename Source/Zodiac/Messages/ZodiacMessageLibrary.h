// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZodiacMessageLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacMessageLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Message")
	static FGameplayTag GetCooldownChannelByTag(const FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Message")
	static FGameplayTag GetCooldownChannelByTags(const FGameplayTagContainer& TagContainer);

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Message")
	static FGameplayTag GetUltimateChargeChannel();
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Message")
	static FGameplayTag GetSkillChangeChannelByTag(const FGameplayTag Tag);
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Message")
	static FGameplayTag GetSkillChangeChannelByTags(const FGameplayTagContainer& TagContainer);
};
