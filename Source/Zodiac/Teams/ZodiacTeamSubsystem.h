// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ZodiacTeamSubsystem.generated.h"


class AZodiacPlayerState;

// Result of comparing the team affiliation for two actors
UENUM(BlueprintType)
enum class EZodiacTeamComparison : uint8
{
	// Both actors are members of the same team
	OnSameTeam,

	// The actors are members of opposing teams
	DifferentTeams,

	// One (or both) of the actors was invalid or not part of any team
	InvalidArgument
};

/**
 * A subsystem for easy access to team information for team-based actors (e.g., pawns or player states)
 */
UCLASS()
class ZODIAC_API UZodiacTeamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	// Returns the team this object belongs to, or INDEX_NONE if it is not part of a team
	int32 FindTeamFromObject(const UObject* TestObject) const;

	// Compare the teams of two actors and returns a value indicating if they are on same teams, different teams, or one/both are invalid
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category=Teams, meta=(ExpandEnumAsExecs=ReturnValue))
	EZodiacTeamComparison CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA, int32& TeamIdB) const;

	// Compare the teams of two actors and returns a value indicating if they are on same teams, different teams, or one/both are invalid
	EZodiacTeamComparison CompareTeams(const UObject* A, const UObject* B) const;

	bool CanCauseDamage(const UObject* Instigator, const UObject* Target, bool bAllowDamageToSelf = true) const;
};
