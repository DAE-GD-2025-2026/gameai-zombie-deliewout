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
	if (!Pawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject("NearestItem"));
	if (!Item) return EBTNodeResult::Failed;

	GEngine->AddOnScreenDebugMessage(6, 1.f, FColor::Blue, TEXT("Go to the item"));

	// Cache item and slot so PickupItem still works if perception clears the BB mid-move
	CachedItem = Item;
	CachedSlot = BB->GetValueAsInt("FreeItemSlot");

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
		// Re-stamp BB so PickupItem still has a valid reference even if perception
		// fired and cleared NearestItem while we were walking
		if (UBlackboardComponent* BB = Controller->GetBlackboardComponent())
		{
			if (CachedItem.IsValid())
			{
				BB->SetValueAsObject("NearestItem", CachedItem.Get());
				BB->SetValueAsInt("FreeItemSlot", CachedSlot);
			}
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UGoToItemDelieWout::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (Controller) Controller->StopMovement();
	CachedItem = nullptr;
	CachedSlot = -1;
	return Super::AbortTask(OwnerComp, NodeMemory);
}
