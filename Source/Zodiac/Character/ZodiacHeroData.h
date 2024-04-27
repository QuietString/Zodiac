// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZodiacHeroData.generated.h"

class UZodiacAbilitySet;

/**
 * 
 */
UCLASS(BlueprintType, Const, meta = (DisplayName = "Zodiac Hero Data"))
class ZODIAC_API UZodiacHeroData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Hero")
	TObjectPtr<USkeletalMesh> HeroMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Hero")
	TSubclassOf<UAnimInstance> AnimInstance;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UZodiacAbilitySet> AbilitySet;
};
