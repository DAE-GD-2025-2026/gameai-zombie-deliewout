// Fill out your copyright notice in the Description page of Project Settings.


#include "RunFromPurgeDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "PurgeZones/PurgeZone.h"

URunFromPurgeDelieWout::URunFromPurgeDelieWout()
{
	bNotifyTick = false;
}

EBTNodeResult::Type URunFromPurgeDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	// Find the nearest purge zone and flee away from it
	TArray<AActor*> OverlappingActors;
	Pawn->GetOverlappingActors(OverlappingActors, APurgeZone::StaticClass());

	FVector FleeFrom = Pawn->GetActorLocation();
	if (OverlappingActors.Num() > 0)
	{
		FleeFrom = OverlappingActors[0]->GetActorLocation();
	}

	FVector PawnLocation = Pawn->GetActorLocation();
	FVector AwayDirection = (PawnLocation - FleeFrom).GetSafeNormal();
	FVector FleeTarget = PawnLocation + AwayDirection * FleeDistance;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->GetRandomPointInNavigableRadius(FleeTarget, 200.f, NavLocation))
		{
			FleeTarget = NavLocation.Location;
		}
	}

	Controller->MoveToLocation(FleeTarget, 50.f);

	return EBTNodeResult::Succeeded;
}