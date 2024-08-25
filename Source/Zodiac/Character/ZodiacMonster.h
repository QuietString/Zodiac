// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacter.h"
#include "ZodiacMonster.generated.h"

class UZodiacHeroData;
class AZodiacHero;

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacMonster : public AZodiacCharacter
{
	GENERATED_BODY()

public:
	AZodiacMonster(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

private:
	UPROPERTY()
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
};
