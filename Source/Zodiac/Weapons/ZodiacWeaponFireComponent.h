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

protected:

	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	void OnHeroChanged(UZodiacHeroComponent* NewHeroComponent);
	
	void UpdateMuzzleSockets(const TArray<FName>& InSocketNames);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FName> MuzzleSocketNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> HeroMeshComponent;
	
private:

	UPROPERTY()
	TArray<TObjectPtr<UZodiacHeroComponent>> HeroComponents;

};
