// Fill out your copyright notice in the Description page of Project Settings.


#include "ClearHouseDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Village/House/House.h"
#include "Navigation/PathFollowingComponent.h"

EBTNodeResult::Type UClearHouseDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	AHouse* House = Cast<AHouse>(BB->GetValueAsObject("NearestHouse"));
	if (!House) return EBTNodeResult::Failed;

	FHouseBounds Bounds = House->GetBounds();

	FVector Target = Bounds.Origin + FVector(
		FMath::RandRange(-Bounds.Extent.X * 0.5f, Bounds.Extent.X * 0.5f),
		FMath::RandRange(-Bounds.Extent.Y * 0.5f, Bounds.Extent.Y * 0.5f),
		0.f);

	EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(Target, 50.f);
	if (Result == EPathFollowingRequestResult::Failed) return EBTNodeResult::Failed;

	return EBTNodeResult::Succeeded;
}