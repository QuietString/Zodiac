// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacter.h"
#include "ZodiacMonster.generated.h"

class UPhysicalAnimationComponent;
class UZodiacHeroData;

UENUM(BlueprintType)
enum EZodiacAIState : uint8
{
	Idle,
	Wandering,
	Chasing
};

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacMonster : public AZodiacCharacter
{
	GENERATED_BODY()

public:
	AZodiacMonster(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacHealthComponent* GetHealthComponent() const override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

private:
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;
};
