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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacHealthComponent* GetHealthComponent() const override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner) override;

	UFUNCTION(BlueprintCallable)
	void SetAIState(EZodiacAIState NewState) { AIState = NewState; }

	UFUNCTION(BlueprintCallable)
	EZodiacAIState GetAIState() { return AIState; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	const UZodiacHeroData* HeroData;

	UPROPERTY(Replicated)
	TEnumAsByte<EZodiacAIState> AIState;
	
private:
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHealthComponent> HealthComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;
};
