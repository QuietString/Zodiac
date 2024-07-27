// the.quiet.string@gmail.com


#include "ZodiacAnimInstance.h"

#include "Character/ZodiacHostCharacter.h"

void UZodiacAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (AZodiacHostCharacter* Pawn = Cast<AZodiacHostCharacter>(TryGetPawnOwner()))
	{
		HostCharacter = Pawn;
		CharacterMovement = HostCharacter->GetCharacterMovement();
	}
}

void UZodiacAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (HostCharacter)
	{
		
	}
}

void UZodiacAnimInstance::UpdateData()
{
}

void UZodiacAnimInstance::SetGait()
{
}
