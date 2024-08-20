// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacAIController.generated.h"

class UZodiacAbilitySystemComponent;

UCLASS()
class ZODIAC_API AZodiacAIController : public AAIController, public IAbilitySystemInterface, public IZodiacTeamAgentInterface
{
	GENERATED_BODY()

public:
	AZodiacAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	
	//~IZodiacTeamAgentInterface interface
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	//~End of IZodiacTeamAgentInterface interface

private:
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Monster;
};
