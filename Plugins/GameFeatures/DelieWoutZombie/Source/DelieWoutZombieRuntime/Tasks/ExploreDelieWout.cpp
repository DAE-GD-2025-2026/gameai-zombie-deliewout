// Fill out your copyright notice in the Description page of Project Settings.


#include "ExploreDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

UExploreDelieWout::UExploreDelieWout()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UExploreDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;

	FNavLocation NavLocation;
	if (!NavSys->GetRandomPointInNavigableRadius(Pawn->GetActorLocation(), SearchRadius, NavLocation))
		return EBTNodeResult::Failed;

	EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(NavLocation.Location, 50.f);
	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)    return EBTNodeResult::Succeeded;
	if (Result == EPathFollowingRequestResult::RequestSuccessful) return EBTNodeResult::InProgress;

	return EBTNodeResult::Failed;
}

void UExploreDelieWout::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
