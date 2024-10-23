// the.quiet.string@gmail.com


#include "ZodiacHeroAbilityManagerComponent.h"

#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroCharacter.h"
#include "ZodiacHeroData.h"
#include "ZodiacHostCharacter.h"
#include "ZodiacHealthComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Hero/ZodiacHeroAbilityFragment_Reticle.h"
#include "Hero/ZodiacHeroAbilityFragment_SlotWidget.h"
#include "Hero/ZodiacHeroAbilitySlot.h"
#include "Hero/ZodiacHeroAbilitySlot_RangedWeapon.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbilityManagerComponent)

UZodiacHeroAbilityManagerComponent::UZodiacHeroAbilityManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bWantsInitializeComponent = true;
	bReplicateUsingRegisteredSubObjectList = true;
	SetIsReplicatedByDefault(true);
}

void UZodiacHeroAbilityManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing ZodiacHeroItemSlot
	if (IsUsingRegisteredSubObjectList())
	{
		for (const auto& Slot : Slots)
		{
			if (Slot)
			{
				AddReplicatedSubObject(Slot);
			}
		}
	}
}

void UZodiacHeroAbilityManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
}

void UZodiacHeroAbilityManagerComponent::OnRegister()
{
	Super::OnRegister();

	AZodiacHeroCharacter* Hero = GetOwner<AZodiacHeroCharacter>();
	ensureMsgf(Hero, TEXT("HeroUIManagerComponent should be attached to AZodiacHero"));
}

void UZodiacHeroAbilityManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (AZodiacHeroCharacter* Hero = GetOwner<AZodiacHeroCharacter>())
	{
		Hero->OnHeroActivated.AddUObject(this, &ThisClass::OnHeroActivated);
		Hero->OnHeroDeactivated.AddUObject(this, &ThisClass::OnHeroDeactivated);
	}
}

void UZodiacHeroAbilityManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AZodiacHeroCharacter* Hero = GetOwner<AZodiacHeroCharacter>())
	{
		for (auto& Slot : Slots)
		{
			if (Hero->HasAuthority())
			{
				for (auto& [Tag, Count] : Slot->GetSlotDefinition().InitialTagStack)
				{
					Slot->AddStatTagStack(Tag, Count);
				}
			}
				
			Slot->OnReticleApplied.BindUObject(this, &ThisClass::SendChangeReticleMessage);
			Slot->OnReticleCleared.BindUObject(this, &ThisClass::ClearAbilityReticle);
		}
	
		if (UZodiacHealthComponent* HealthComponent = Hero->GetComponentByClass<UZodiacHealthComponent>())
		{
			HealthComponent->OnHealthChanged.AddDynamic(this, &ThisClass::SendChangeHealthMessage);
		}
	}
}

void UZodiacHeroAbilityManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto& Slot : Slots)
	{
		Slot->OnReticleApplied.Unbind();
		Slot->OnReticleCleared.Unbind();
	}
	
	Super::EndPlay(EndPlayReason);
}

void UZodiacHeroAbilityManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	for (auto& Slot : Slots)
	{
		Slot->Tick(DeltaTime);
	}
}

void UZodiacHeroAbilityManagerComponent::InitializeWithAbilitySystem(UZodiacAbilitySystemComponent* InAbilitySystemComponent, const UZodiacHeroData* InHeroData)
{
	if (!InAbilitySystemComponent || !InHeroData)
	{
		return;
	}

	AbilitySystemComponent = InAbilitySystemComponent;
	HeroData = InHeroData;

	if (GetOwner()->HasAuthority())
	{
		if (!HeroData->AbilitySlots.IsEmpty())
		{
			for (auto& SlotDef : HeroData->AbilitySlots)
			{
				if (TSubclassOf<UZodiacHeroAbilitySlot> SlotClass = SlotDef.SlotClass)
				{
					if (UZodiacHeroAbilitySlot* Slot = Slots.Add_GetRef(NewObject<UZodiacHeroAbilitySlot>(GetOwner(), SlotClass)))
					{
						Slot->InitializeSlot(SlotDef);

						if (UZodiacAbilitySet* AbilitySet = SlotDef.SkillSetToGrant)
						{
							AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr, Slot);	
						}
			
						if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
						{
							AddReplicatedSubObject(Slot);
						}
					}
				}
			}
		}
	}
}

void UZodiacHeroAbilityManagerComponent::OnHeroActivated()
{
	bIsHeroActive = true;

	bool bHasReticleSet = false;
	
	for (auto& Slot : Slots)
	{
		if (!bHasReticleSet)
		{
			if (UZodiacHeroAbilityFragment_Reticle* ReticleFragment = Slot->FindFragmentByClass<UZodiacHeroAbilityFragment_Reticle>())
			{
				if (ReticleFragment->bIsMainReticle)
				{
					SendChangeReticleMessage(ReticleFragment->ReticleWidgets, Slot);
					bHasReticleSet = true;
				}
			}
		}

		// UZodiacHeroAbilityFragment_SlotWidget* SlotWidgetFragment = Slot->FindFragmentByClass<UZodiacHeroAbilityFragment_SlotWidget>();
		// TArray<TSubclassOf<UZodiacAbilitySlotWidgetBase>> Widgets;
		// if (SlotWidgetFragment)
		// {
		// 	Widgets.Add(SlotWidgetFragment->Widget);
		// }
		//
		// // Send message even when Widget is empty to update HUD.
		// SendChangeWidgetMessage(Widgets, Slot);
		
		if (UZodiacHeroAbilityFragment_SlotWidget* SlotWidgetFragment = Slot->FindFragmentByClass<UZodiacHeroAbilityFragment_SlotWidget>())
		{
			TArray<TSubclassOf<UZodiacAbilitySlotWidgetBase>> Widgets;
			if (Widgets.Add_GetRef(SlotWidgetFragment->Widget))
			{
				SendChangeWidgetMessage(Widgets, Slot);
			}
		}
	}
	
	if (AZodiacHeroCharacter* Hero = Cast<AZodiacHeroCharacter>(GetOwner()))
	{
		if (UZodiacHealthComponent* HealthComponent = GetOwner()->FindComponentByClass<UZodiacHealthComponent>())
		{
			SendChangeHealthMessage(HealthComponent, HealthComponent->GetHealth(), HealthComponent->GetHealth(), nullptr);
		}
	}
}

void UZodiacHeroAbilityManagerComponent::OnHeroDeactivated()
{
	bIsHeroActive = false;
}

AController* UZodiacHeroAbilityManagerComponent::GetHostController()
{
	if (AZodiacHeroCharacter* Hero = GetOwner<AZodiacHeroCharacter>())
	{
		if (AZodiacHostCharacter* Host = Hero->GetHostCharacter())
		{
			return Host->GetController();
		}
	}

	return nullptr;
}

void UZodiacHeroAbilityManagerComponent::SendChangeReticleMessage(const TArray<TSubclassOf<UZodiacReticleWidgetBase>>& Widgets,
                                                                  UZodiacHeroAbilitySlot* Slot)
{
	FZodiacHUDMessage_ReticleChanged Message;
	Message.Controller = GetHostController();
	Message.Slot = Slot;
	Message.Widgets = Widgets;
	const FGameplayTag Channel = ZodiacGameplayTags::HUD_Message_ReticleChanged;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Channel, Message);
}

void UZodiacHeroAbilityManagerComponent::SendChangeWidgetMessage(const TArray<TSubclassOf<UZodiacAbilitySlotWidgetBase>>& Widgets,
                                                                 UZodiacHeroAbilitySlot* Slot)
{
	FZodiacHUDMessage_WidgetChanged Message;
	Message.Controller = GetHostController();
	Message.Slot = Slot;
	Message.Widgets = Widgets;
	const FGameplayTag Channel = ZodiacGameplayTags::HUD_Message_WidgetChanged;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Channel, Message);
}

void UZodiacHeroAbilityManagerComponent::ClearAbilityReticle()
{
	for (auto& Slot : Slots)
	{
		if (UZodiacHeroAbilityFragment_Reticle* ReticleFragment = Slot->FindFragmentByClass<UZodiacHeroAbilityFragment_Reticle>())
		{
			if (ReticleFragment->bIsMainReticle)
			{
				SendChangeReticleMessage(ReticleFragment->ReticleWidgets, Slot);
				break;
			}
		}
	}
}

void UZodiacHeroAbilityManagerComponent::SendChangeHealthMessage(UZodiacHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	if (AZodiacHeroCharacter* Hero = GetOwner<AZodiacHeroCharacter>())
	{
		FZodiacHUDMessage_HealthChanged Message;
		Message.Owner = Hero->GetHostCharacter();
		Message.Hero = Hero;
		Message.bIsHeroActive = bIsHeroActive;
		Message.MaxValue = HealthComponent->GetMaxHealth();
		Message.OldValue = OldValue;
		Message.NewValue = NewValue;
		
		const FGameplayTag Channel = ZodiacGameplayTags::HUD_Message_HealthChanged;
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(Channel, Message);
	}
}
