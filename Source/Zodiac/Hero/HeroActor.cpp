// the.quiet.string@gmail.com


#include "HeroActor.h"

#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HeroActor)

AHeroActor::AHeroActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UZodiacAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	Mesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	if (Mesh)
	{
		RootComponent = Mesh;
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}
}

void AHeroActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AHeroActor::InitializeAbilitySystem(AActor* InOwnerActor)
{
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, this);

	for (auto& AbilitySet : AbilitySets)
	{
		FZodiacAbilitySet_GrantedHandles Handles;
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &Handles);
	}
}
