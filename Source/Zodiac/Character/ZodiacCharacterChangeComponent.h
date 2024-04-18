// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "ZodiacCharacterChangeComponent.generated.h"


class UZodiacHeroData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ZODIAC_API UZodiacCharacterChangeComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacCharacterChangeComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnRegister() override;
	
protected:
	TArray<UZodiacHeroData*> GetHeroes() const;
	
	void InitializeData();

private:

	UPROPERTY()
	TArray<UZodiacHeroData*> Heroes;

	UPROPERTY()
	TArray<USkeletalMesh*> Meshes;

	UPROPERTY()
	TArray<UAnimInstance*> AnimInstances;
};
