// the.quiet.string@gmail.com


#include "ZodiacKismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacKismetSystemLibrary)

bool UZodiacKismetSystemLibrary::BeamTraceMultiByChannel(const UObject* WorldContextObject, FVector Start, FVector End, float BeamWidth,
	ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, EDrawDebugTrace::Type DrawDebugType,
	TArray<FHitResult>& OutHits, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	// Convert ETraceTypeQuery -> ECollisionChannel
    ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

    // Configure the collision query params similarly to how the engine does in KismetSystemLibrary
    static const FName RectLineTraceName(TEXT("BeamTraceMulti"));
    FCollisionQueryParams Params(RectLineTraceName, bTraceComplex);
    // Add ActorsToIgnore, ignoring self if requested
    Params.AddIgnoredActors(ActorsToIgnore);
    if (bIgnoreSelf && WorldContextObject)
    {
        if (AActor* IgnoreActor = Cast<AActor>(WorldContextObject->GetTypedOuter<AActor>()))
        {
            Params.AddIgnoredActor(IgnoreActor);
        }
    }

    // Get the world from context
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }

    // Prepare the array to store results
    OutHits.Reset();

    // Direction vector from Start to End
    FVector TraceDir = (End - Start);
    FVector TraceDirNorm = TraceDir.GetSafeNormal();
    // We'll define a "Right" vector by crossing with an "Up" direction
    // If the line is near vertical, pick a fallback up
    FVector UpVector = (FMath::Abs(TraceDirNorm.Z) > 0.99f) ? FVector::RightVector : FVector::UpVector;
    FVector Right = (TraceDirNorm ^ UpVector).GetSafeNormal() * (BeamWidth * 0.5f);
    // Then we define an "Up" that is truly perpendicular to the line and Right
    FVector MyUp = (TraceDirNorm ^ Right).GetSafeNormal() * (BeamWidth * 0.5f);

    // We'll do 4 corners if we want a "rectangle" cross section
    // top-left, top-right, bottom-left, bottom-right
    TArray<FVector> CornerStarts;
    TArray<FVector> CornerEnds;

    CornerStarts.Add(Start + Right + MyUp);    // top-right
    CornerEnds.Add(End + Right + MyUp);

    CornerStarts.Add(Start - Right + MyUp);    // top-left
    CornerEnds.Add(End - Right + MyUp);

    CornerStarts.Add(Start + Right - MyUp);    // bottom-right
    CornerEnds.Add(End + Right - MyUp);

    CornerStarts.Add(Start - Right - MyUp);    // bottom-left
    CornerEnds.Add(End - Right - MyUp);

    bool bAnyHit = false;

    // Perform 4 line traces
    for (int32 i = 0; i < CornerStarts.Num(); i++)
    {
        TArray<FHitResult> CornerHits;
        // line trace
        bool bHitCorner = World->LineTraceMultiByChannel(
            CornerHits,
            CornerStarts[i],
            CornerEnds[i],
            CollisionChannel,
            Params
        );

        if (bHitCorner)
        {
            bAnyHit = true;
            OutHits.Append(CornerHits);
        }

#if ENABLE_DRAW_DEBUG
        // Draw debug lines for each corner
        if (DrawDebugType != EDrawDebugTrace::None)
        {
            bool bHitSomething = (CornerHits.Num() > 0);
            const FHitResult* Hit = bHitSomething ? &CornerHits.Last() : nullptr;

            // If requested, draw the trace lines
            if (DrawDebugType == EDrawDebugTrace::ForOneFrame || DrawDebugType == EDrawDebugTrace::ForDuration || DrawDebugType == EDrawDebugTrace::Persistent)
            {
                // No hit
                if (!bHitCorner)
                {
                    DrawDebugLine(World, CornerStarts[i], CornerEnds[i], TraceColor.ToFColor(true), (DrawDebugType == EDrawDebugTrace::Persistent), DrawTime);
                }
                else
                {
                    // Draw up to hit, then from hit to end
                    DrawDebugLine(World, CornerStarts[i], Hit->Location, TraceColor.ToFColor(true), (DrawDebugType == EDrawDebugTrace::Persistent), DrawTime);
                    DrawDebugLine(World, Hit->Location, CornerEnds[i], TraceHitColor.ToFColor(true), (DrawDebugType == EDrawDebugTrace::Persistent), DrawTime);
                    DrawDebugPoint(World, Hit->Location, 8.0f, TraceHitColor.ToFColor(true), (DrawDebugType == EDrawDebugTrace::Persistent), DrawTime);
                }
            }
        }
#endif
    }

    return bAnyHit;
}
