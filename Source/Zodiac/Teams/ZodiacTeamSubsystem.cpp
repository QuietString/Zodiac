// the.quiet.string@gmail.com


#include "ZodiacTeamSubsystem.h"

#include "ZodiacLogChannels.h"
#include "ZodiacTeamAgentInterface.h"
#include "AbilitySystemGlobals.h"
#include "Player/ZodiacPlayerState.h"

int32 UZodiacTeamSubsystem::FindTeamFromObject(const UObject* TestObject) const
{
	// See if it's directly a team agent
	if (const IZodiacTeamAgentInterface* ObjectWithTeamInterface = Cast<IZodiacTeamAgentInterface>(TestObject))
	{
		return ObjectWithTeamInterface->GetGenericTeamId();
	}

	if (const AActor* TestActor = Cast<const AActor>(TestObject))
	{
		// See if the instigator is a team actor
		if (const IZodiacTeamAgentInterface* InstigatorWithTeamInterface = Cast<IZodiacTeamAgentInterface>(TestActor->GetInstigator()))
		{
			return InstigatorWithTeamInterface->GetGenericTeamId();
		}
	}

	return INDEX_NONE;
}

EZodiacTeamComparison UZodiacTeamSubsystem::CompareTeams(const UObject* A, const UObject* B, int32& TeamIdA,
	int32& TeamIdB) const
{
	TeamIdA = FindTeamFromObject(Cast<const AActor>(A));
	TeamIdB = FindTeamFromObject(Cast<const AActor>(B));

	if ((TeamIdA == INDEX_NONE) || (TeamIdB == INDEX_NONE))
	{
		return EZodiacTeamComparison::InvalidArgument;
	}
	else
	{
		return (TeamIdA == TeamIdB) ? EZodiacTeamComparison::OnSameTeam : EZodiacTeamComparison::DifferentTeams;
	}
}

EZodiacTeamComparison UZodiacTeamSubsystem::CompareTeams(const UObject* A, const UObject* B) const
{
	int32 TeamIdA;
	int32 TeamIdB;
	return CompareTeams(A, B, OUT TeamIdA, OUT TeamIdB);
}

bool UZodiacTeamSubsystem::CanCauseDamage(const UObject* Instigator, const UObject* Target,
                                          bool bAllowDamageToSelf) const
{
	if (bAllowDamageToSelf)
	{
		if (Instigator == Target)
		{
			return true;
		}
	}
	
	int32 InstigatorTeamId;
	int32 TargetTeamId;
	const EZodiacTeamComparison Relationship = CompareTeams(Instigator, Target, OUT InstigatorTeamId, OUT TargetTeamId);
	
	if (Relationship == EZodiacTeamComparison::DifferentTeams)
	{
		return true;
	}
	
	if ((Relationship == EZodiacTeamComparison::InvalidArgument) && (InstigatorTeamId != INDEX_NONE))
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Cast<const AActor>(Target));

		if (TargetASC)
		{
			UE_LOG(LogZodiacTeams, Warning, TEXT("Allowing damage to an object with no team: %s"), *Target->GetName());
			return true;
		}
		return false;
	}
	
	return false;
}
