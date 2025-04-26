// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacCharacterType.h"
#include "Engine/DataAsset.h"
#include "ZodiacGameData.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType, Const, Meta = (DisplayName = "Zodiac Game Data", ShortTooltip = "Data asset containing global game data."))
class ZODIAC_API UZodiacGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UZodiacGameData& Get();

public:
	// Gameplay effect used to apply damage. Uses SetByCaller for the damage magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// Gameplay effect used to apply healing. Uses SetByCaller for the healing magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// Gameplay effect used to charge ultimate. Uses SetByCaller for the charging magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Charge Ultimate Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> ChargeUltimateGameplayEffect_SetByCaller;
	
	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Zombie")
	TArray<FZodiacExtendedMovementConfig> MovementConfigTemplates;
};
