// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ZodiacReticleWidgetBase.generated.h"

class UZodiacHeroAbilitySlot_Weapon;

/**
 * 
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacReticleWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UZodiacReticleWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	void InitializeFromWeaponSlot(UZodiacHeroAbilitySlot_Weapon* InSlot);
	
	/** Returns the current weapon's diametrical spread angle, in degrees */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float ComputeSpreadAngle() const;

	/** Returns the current weapon's maximum spread radius in screenspace units (pixels) */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float ComputeMaxScreenspaceSpreadRadius() const;

	/**
	 * Returns true if the current weapon is at 'first shot accuracy'
	 * (the weapon allows it and it is at min spread)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasFirstShotAccuracy() const;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UZodiacHeroAbilitySlot_Weapon> WeaponSlot;
};
