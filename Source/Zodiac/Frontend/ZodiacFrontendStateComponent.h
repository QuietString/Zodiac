// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ControlFlowNode.h"
#include "LoadingProcessInterface.h"
#include "Components/GameStateComponent.h"
#include "ZodiacFrontendStateComponent.generated.h"


class UCommonActivatableWidget;
DECLARE_DELEGATE_OneParam(FControlFlowWaitDelegate, FControlFlowNodeRef)
DECLARE_DELEGATE_OneParam(FControlFlowPopulator, TSharedRef<FControlFlow>)

UCLASS(Abstract, meta=(BlueprintSpawnableComponent))
class ZODIAC_API UZodiacFrontendStateComponent : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	UZodiacFrontendStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	
	//~ILoadingProcessInterface interface
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface interface

private:
	void FlowStep_WaitForUserInitialization(FControlFlowNodeRef SubFlow);
	void FlowStep_TryJoinRequestedSession(FControlFlowNodeRef SubFlow);
	void FlowStep_TryShowMainScreen(FControlFlowNodeRef SubFlow);

protected:
	UPROPERTY(EditAnywhere, Category = UI)
	TSoftClassPtr<UCommonActivatableWidget> MainScreenClass;
	
private:
	bool bShouldShowLoadingScreen = true;

	TSharedPtr<FControlFlow> FrontendFlow;

	FDelegateHandle OnJoinSessionCompleteEventHandle;
};
