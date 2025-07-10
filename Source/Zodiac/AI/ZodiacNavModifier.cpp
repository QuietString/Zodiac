// the.quiet.string@gmail.com

#include "ZodiacNavModifier.h"
#include "NavModifierComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "NavAreas/NavArea_Null.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacNavModifier)

AZodiacNavModifier::AZodiacNavModifier(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

#if WITH_EDITOR
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (!IsRunningCommandlet() && SpriteComponent)
	{
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture;
			FConstructorStatics()
				: SpriteTexture(TEXT("/Engine/EditorResources/S_TriggerBox"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.SpriteTexture.Get();
		SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->SetVisibleFlag(true);
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->SetAbsolute(false, false, true);
		SpriteComponent->bIsScreenSizeScaled = true;
	}
#endif
	
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
