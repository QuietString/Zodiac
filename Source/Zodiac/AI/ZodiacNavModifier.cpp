// the.quiet.string@gmail.com


#include "ZodiacNavModifier.h"

#include "NavModifierComponent.h"
#include "Components/BoxComponent.h"
#include "NavAreas/NavArea_Null.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacNavModifier)

AZodiacNavModifier::AZodiacNavModifier(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(SceneRoot);
	BoxComponent->SetBoxExtent(FVector(200.f, 200.f, 200.f));

	NavModifierComp = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifierComp"));
}

void AZodiacNavModifier::SetNavArea(TSubclassOf<UNavArea> NewAreaClass)
{
	if (NewAreaClass != nullptr && NavModifierComp)
	{
		NavModifierComp->SetAreaClass(NewAreaClass);
	}
}
