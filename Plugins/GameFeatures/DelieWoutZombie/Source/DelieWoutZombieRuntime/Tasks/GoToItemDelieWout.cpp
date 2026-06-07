// Fill out your copyright notice in the Description page of Project Settings.


#include "GoToItemDelieWout.h"
#include "AIController.h"
#include "Items/BaseItem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UGoToItemDelieWout::UGoToItemDelieWout()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UGoToItemDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject("NearestItem"));
	if (!Item)return EBTNodeResult::Failed;

	EPathFollowingRequestResult::Type Result = Controller->MoveToActor(Item);
	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)    return EBTNodeResult::Succeeded;
	if (Result == EPathFollowingRequestResult::RequestSuccessful) return EBTNodeResult::InProgress;

	return EBTNodeResult::Failed;
}

void UGoToItemDelieWout::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

EBTNodeResult::Type UGoToItemDelieWout::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller) Controller->StopMovement();
	return Super::AbortTask(OwnerComp, NodeMemory);
}
