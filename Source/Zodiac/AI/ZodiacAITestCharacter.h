// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacCharacter.h"
#include "ZodiacAITestCharacter.generated.h"

UCLASS()
class ZODIAC_API AZodiacAITestCharacter : public AZodiacCharacter
{
	GENERATED_BODY()

public:
	AZodiacAITestCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface
	
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }

	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Hero;
};
