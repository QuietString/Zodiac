// the.quiet.string@gmail.com


#include "ZodiacGameData.h"

#include "ZodiacAssetManager.h"

const UZodiacGameData& UZodiacGameData::Get()
{
	return UZodiacAssetManager::Get().GetGameData();
}
