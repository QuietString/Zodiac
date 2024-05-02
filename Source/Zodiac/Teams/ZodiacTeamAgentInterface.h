// the.quiet.string@gmail.com

#pragma once

#include "GenericTeamAgentInterface.h"
#include "ZodiacTeamAgentInterface.generated.h"

UENUM(BlueprintType)
enum class EZodiacTeam : uint8
{
	Hero     UMETA(DisplayName = "Hero"),
	Monster  UMETA(DisplayName = "Monster"),
	Neutral  UMETA(DisplayName = "Neutral"),
	NoTeam = 255 UMETA(DisplayName = "No Team")
};

inline EZodiacTeam GenericTeamToZodiacTeam(const FGenericTeamId GenericTeamId)
{
	return static_cast<EZodiacTeam>(GenericTeamId.GetId());
}

UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class UZodiacTeamAgentInterface : public UGenericTeamAgentInterface
{
	GENERATED_BODY()
};

class ZODIAC_API IZodiacTeamAgentInterface : public IGenericTeamAgentInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	
};
