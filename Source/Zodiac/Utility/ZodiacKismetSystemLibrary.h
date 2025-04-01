// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ZodiacKismetSystemLibrary.generated.h"

UCLASS()
class ZODIAC_API UZodiacKismetSystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
   * Casts a rectangular beam from Start to End by doing four parallel line traces at the corners.
   * @param WorldContextObject   Usually "self" in Blueprint
   * @param Start                Start position of the rectangular beam
   * @param End                  End position of the rectangular beam
   * @param BeamWidth            Width (and height) of the rectangle cross-section
   * @param TraceChannel         The 'TraceTypeQuery' to use for collision
   * @param bTraceComplex        Whether to trace against complex collision
   * @param ActorsToIgnore       Array of actors to ignore
   * @param DrawDebugType        Whether and how to draw the debug lines
   * @param OutHits              The array of all hits from the four line traces
   * @param bIgnoreSelf          Whether to ignore the calling actor
   * @param TraceColor           Debug color for lines
   * @param TraceHitColor        Debug color for lines on hit
   * @param DrawTime             Duration of debug lines
   * @return                     True if at least one line found a blocking hit, otherwise false
   */
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(WorldContext="WorldContextObject", bIgnoreSelf="true", DisplayName="Rectangular Beam Trace By Channel"))
	static bool BeamTraceMultiByChannel(
		const UObject* WorldContextObject,
		FVector Start,
		FVector End,
		float BeamWidth,
		ETraceTypeQuery TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		EDrawDebugTrace::Type DrawDebugType,
		TArray<FHitResult>& OutHits,
		bool bIgnoreSelf = true,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor TraceHitColor = FLinearColor::Green,
		float DrawTime = 5.0f
	);

	// Capsule trace with ComponentsToIgnore
	UFUNCTION(BlueprintCallable, Category="Collision", meta=(bIgnoreSelf="true", WorldContext="WorldContextObject", AutoCreateRefTerm="ActorsToIgnore", DisplayName = "Capsule Trace By Channel", AdvancedDisplay="TraceColor,TraceHitColor,DrawTime", Keywords="sweep"))
	static bool CapsuleTraceSingle(const UObject* WorldContextObject, const FVector Start, const FVector End, float Radius, float HalfHeight, ETraceTypeQuery TraceChannel, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, const TArray<UPrimitiveComponent*>& ComponentsToIgnore, EDrawDebugTrace::Type DrawDebugType, FHitResult& OutHit, bool bIgnoreSelf, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 5.0f);
};
