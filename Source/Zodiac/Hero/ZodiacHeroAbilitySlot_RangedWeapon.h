// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Curves/CurveFloat.h"

#include "ZodiacHeroAbilitySlot_Weapon.h"

#include "ZodiacHeroAbilitySlot_RangedWeapon.generated.h"

class UPhysicalMaterial;

UCLASS()
class UZodiacHeroAbilitySlot_RangedWeapon : public UZodiacHeroAbilitySlot_Weapon
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;
	
	int32 GetBulletsPerCartridge() const { return BulletsPerCartridge; }
	float GetMaxDamageRange() const { return MaxDamageRange; }
	float GetBulletTraceSweepRadius() const { return BulletTraceSweepRadius; }

protected:
	
	// Number of bullets to fire in a single cartridge (typically 1, but may be more for shotguns)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config")
	int32 BulletsPerCartridge = 1;

	// The maximum distance at which this weapon can deal damage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float MaxDamageRange = 25000.0f;

	// The radius for bullet traces sweep spheres (0.0 will result in a line trace)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float BulletTraceSweepRadius = 0.0f;

	// A curve that maps the distance (in cm) to a multiplier on the base damage from the associated gameplay effect
	// If there is no data in this curve, then the weapon is assumed to have no falloff with distance
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	FRuntimeFloatCurve DistanceDamageFalloff;

	// List of special tags that affect how damage is dealt
	// These tags will be compared to tags in the physical material of the thing being hit
	// If more than one tag is present, the multipliers will be combined multiplicatively
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Config")
	TMap<FGameplayTag, float> MaterialDamageMultiplier;

private:
	

public:
	
	//~IZodiacAbilitySourceInterface interface
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	//~End of IZodiacAbilitySourceInterface interface


};
