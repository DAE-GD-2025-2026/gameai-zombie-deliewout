// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupItemDelieWout.h"
#include "AIController.h"
#include "Common/InventoryComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Items//BaseItem.h"
#include "DelieWoutZombieRuntime/StudentPerceptorDelieWout.h"

UPickupItemDelieWout::UPickupItemDelieWout()
{
}

EBTNodeResult::Type UPickupItemDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB)return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject("NearestItem"));
	int SlotIndex = BB->GetValueAsInt("FreeItemSlot");
	if (!IsValid(Item) || SlotIndex < 0) return EBTNodeResult::Failed;

	UInventoryComponent* Inventory = Controller->GetPawn()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return EBTNodeResult::Failed;

	auto ClearTarget = [BB]()
	{
		BB->SetValueAsObject("NearestItem", nullptr);
		BB->SetValueAsInt("FreeItemSlot", -1);
	};

	if (Inventory->GetInventory().Contains(Item))
	{
		ClearTarget();
		return EBTNodeResult::Succeeded;
	}

	if (Inventory->GetInventory()[SlotIndex] != nullptr)
		Inventory->RemoveItem(SlotIndex);

	const bool bGrabbed = Inventory->GrabItem(SlotIndex, Item);
	if (bGrabbed)
	{
		if (auto* Perceptor = Controller->GetPawn()->FindComponentByClass<UStudentPerceptorDelieWout>())
			Perceptor->ForgetItem(Item);

		if (Item->GetItemType() == EItemType::Garbage)
			Inventory->RemoveItem(SlotIndex);
	}

	ClearTarget();
	return bGrabbed ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
