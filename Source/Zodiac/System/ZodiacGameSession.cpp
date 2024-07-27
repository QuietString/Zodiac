// the.quiet.string@gmail.com


#include "ZodiacGameSession.h"


// Sets default values
AZodiacGameSession::AZodiacGameSession()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZodiacGameSession::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZodiacGameSession::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

