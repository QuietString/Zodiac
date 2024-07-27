// the.quiet.string@gmail.com

#pragma once

#include "UI/HUD/ZodiacHeroWidgetBase.h"

#include "ZodiacWeaponUserInterface.generated.h"


UCLASS()
class UZodiacWeaponUserInterface : public UZodiacHeroWidgetBase
{
	GENERATED_BODY()

public:
	

protected:

	//virtual void HandleHeroChanged(UZodiacHeroComponent* HeroComponent) override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnHeroChanged(TSubclassOf<UZodiacReticleWidgetBase> ReticleWidget);
	
};
