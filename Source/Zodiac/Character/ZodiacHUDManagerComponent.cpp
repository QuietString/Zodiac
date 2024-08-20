// the.quiet.string@gmail.com


#include "ZodiacHUDManagerComponent.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacHero.h"
#include "ZodiacHeroData.h"
#include "ZodiacHostCharacter.h"
#include "UI/Weapons/ZodiacReticleWidgetBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHUDManagerComponent)

UZodiacHUDManagerComponent::UZodiacHUDManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
}

void UZodiacHUDManagerComponent::OnRegister()
{
	Super::OnRegister();

	AZodiacHero* Hero = GetOwner<AZodiacHero>();
	ensureMsgf(Hero, TEXT("HeroUIManagerComponent should be attached to AZodiacHero"));
}

void UZodiacHUDManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (AZodiacHero* Hero = GetOwner<AZodiacHero>())
	{
		Hero->OnHeroActivated.AddUObject(this, &ThisClass::OnHeroActivated);
	}
}

void UZodiacHUDManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZodiacHUDManagerComponent::OnHeroActivated()
{
	if (AZodiacHero* Hero = Cast<AZodiacHero>(GetOwner()))
	{
		if (const UZodiacHeroData* HeroData = Hero->GetHeroData())
		{
			if (!HeroData->ReticleWidgets.IsEmpty())
			{
				SendChangeReticleMessage();
			}
		}
	}
}

void UZodiacHUDManagerComponent::SendChangeReticleMessage()
{
	if (AZodiacHero* Hero = GetOwner<AZodiacHero>())
	{
		if (const UZodiacHeroData* HeroData = Hero->GetHeroData())
		{
			FZodiacHUDMessage_ReticleChanged Message;
			Message.Instigator = Hero->GetHostCharacter();
			Message.Reticle = HeroData->ReticleWidgets[0];
	
			const FGameplayTag Channel = ZodiacGameplayTags::HUD_Message_ReticleChanged;
			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(Channel, Message);
		}
	}
}
