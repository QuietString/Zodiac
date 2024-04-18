// the.quiet.string@gmail.com


#include "ZodiacCharacterChangeComponent.h"

#include "ZodiacCharacter.h"
#include "ZodiacHeroData.h"


UZodiacCharacterChangeComponent::UZodiacCharacterChangeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacCharacterChangeComponent::OnRegister()
{
	Super::OnRegister();

	InitializeData();
}

TArray<UZodiacHeroData*> UZodiacCharacterChangeComponent::GetHeroes() const
{
	TArray<UZodiacHeroData*> Data;
	
	if (GetOwner())
	{
		if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetOwner()))
		{
			Data.Append(ZodiacCharacter->GetHeroes());
		}
	}

	return Data;
}


void UZodiacCharacterChangeComponent::InitializeData()
{
	if (GetOwner())
	{
		if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetOwner()))
		{
			Heroes = ZodiacCharacter->GetHeroes();
			
		}
	}
}
