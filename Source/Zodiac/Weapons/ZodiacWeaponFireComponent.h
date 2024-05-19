// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "ZodiacWeaponFireComponent.generated.h"


class UZodiacHeroComponent;

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class ZODIAC_API UZodiacWeaponFireComponent : public UPawnComponent
{
	GENERATED_BODY()

public:	
	UZodiacWeaponFireComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> HeroMeshComponent;

};
