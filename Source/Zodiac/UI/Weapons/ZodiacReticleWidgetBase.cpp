// the.quiet.string@gmail.com


#include "ZodiacReticleWidgetBase.h"

#include "Hero/ZodiacHeroAbilitySlot_RangedWeapon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacReticleWidgetBase)

UZodiacReticleWidgetBase::UZodiacReticleWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UZodiacReticleWidgetBase::InitializeFromAbilitySlot(UZodiacHeroAbilitySlot* InSlot)
{
	AbilitySlot = InSlot;
}

float UZodiacReticleWidgetBase::ComputeSpreadAngle() const
{
	if (UZodiacHeroAbilitySlot_Weapon* WeaponSlot = Cast<UZodiacHeroAbilitySlot_Weapon>(AbilitySlot))
	{
		const float BaseSpreadAngle = WeaponSlot->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = WeaponSlot->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		return ActualSpreadAngle;
	}
	else
	{
		return 0.0f;
	}
}

float UZodiacReticleWidgetBase::ComputeMaxScreenspaceSpreadRadius() const
{
	const float LongShotDistance = 10000.f;

	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->PlayerCameraManager)
	{
		// A weapon's spread can be thought of as a cone shape. To find the screenspace spread for reticle visualization,
		// we create a line on the edge of the cone at a long distance. The end of that point is on the edge of the cone's circle.
		// We then project it back onto the screen. Its distance from screen center is the spread radius.

		// This isn't perfect, due to there being some distance between the camera location and the gun muzzle.

		const float SpreadRadiusRads = FMath::DegreesToRadians(ComputeSpreadAngle() * 0.5f);
		const float SpreadRadiusAtDistance = FMath::Tan(SpreadRadiusRads) * LongShotDistance;

		FVector CamPos;
		FRotator CamOrient;
		PC->PlayerCameraManager->GetCameraViewPoint(CamPos, CamOrient);

		FVector CamFwdDir = CamOrient.RotateVector(FVector::ForwardVector);
		FVector CamUpDir   = CamOrient.RotateVector(FVector::UpVector);

		FVector OffsetTargetAtDistance = CamPos + (CamFwdDir * LongShotDistance) + (CamUpDir * SpreadRadiusAtDistance);

		FVector2D OffsetTargetInScreenspace;

		if (PC->ProjectWorldLocationToScreen(OffsetTargetAtDistance, OffsetTargetInScreenspace, true))
		{
			int32 ViewportSizeX(0), ViewportSizeY(0);
			PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

			const FVector2D ScreenSpaceCenter(FVector::FReal(ViewportSizeX) * 0.5f, FVector::FReal(ViewportSizeY) * 0.5f);

			return (OffsetTargetInScreenspace - ScreenSpaceCenter).Length();
		}
	}
	
	return 0.0f;
}

bool UZodiacReticleWidgetBase::HasFirstShotAccuracy() const
{
	if (UZodiacHeroAbilitySlot_Weapon* WeaponSlot = Cast<UZodiacHeroAbilitySlot_Weapon>(AbilitySlot))
	{
		return WeaponSlot->HasFirstShotAccuracy();
	}
	
	return false;
}