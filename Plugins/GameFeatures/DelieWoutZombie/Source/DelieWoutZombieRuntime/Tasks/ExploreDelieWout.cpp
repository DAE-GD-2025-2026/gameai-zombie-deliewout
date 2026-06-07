// Fill out your copyright notice in the Description page of Project Settings.


#include "ExploreDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Zombies/BaseZombie.h"

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

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	FExploreMemory* Memory = reinterpret_cast<FExploreMemory*>(NodeMemory);

	//Wander
	Memory->WanderAngle += FMath::RandRange(-WanderJitter, WanderJitter);

	float AngleRad = FMath::DegreesToRadians(Memory->WanderAngle);
	FVector WanderDir(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f);

	// Flee from nearest zombie
	FVector FleeDir = FVector::ZeroVector;
	if (BB)
	{
		if (ABaseZombie* Zombie = Cast<ABaseZombie>(BB->GetValueAsObject("NearestZombie")))
		{
			FVector ToZombie = Zombie->GetActorLocation() - Pawn->GetActorLocation();
			if (!ToZombie.IsNearlyZero())
				FleeDir = -ToZombie.GetSafeNormal();
		}
	}

	// Blend wander and Flee
	FVector BlendedDir = WanderDir * WanderWeight + FleeDir * FleeWeight;
	if (BlendedDir.IsNearlyZero()) BlendedDir = WanderDir;
	BlendedDir.Normalize();

	FVector Target = Pawn->GetActorLocation() + BlendedDir * SearchRadius;

	FNavLocation NavLocation;
	if (!NavSys->GetRandomPointInNavigableRadius(Target, SearchRadius * 0.3f, NavLocation))
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

EBTNodeResult::Type UExploreDelieWout::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller) Controller->StopMovement();
	return Super::AbortTask(OwnerComp, NodeMemory);
}
