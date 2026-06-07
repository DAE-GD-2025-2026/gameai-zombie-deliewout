// Fill out your copyright notice in the Description page of Project Settings.


#include "GoToHouseDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Village/House/House.h"

UGoToHouseDelieWout::UGoToHouseDelieWout()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UGoToHouseDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	AHouse* House = Cast<AHouse>(BB->GetValueAsObject("NearestHouse"));
	if (!House) return EBTNodeResult::Failed;

	EPathFollowingRequestResult::Type Result = Controller->MoveToActor(House);
	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)    return EBTNodeResult::Succeeded;
	if (Result == EPathFollowingRequestResult::RequestSuccessful) return EBTNodeResult::InProgress;

	return EBTNodeResult::Failed;
}

void UGoToHouseDelieWout::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UGoToHouseDelieWout::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller) Controller->StopMovement();
	return Super::AbortTask(OwnerComp, NodeMemory);
}
