// Fill out your copyright notice in the Description page of Project Settings.


#include "UseMedkitDelieWout.h"
#include "AIController.h"
#include "Items/Medkit.h"
#include "Common/InventoryComponent.h"

UUseMedkitDelieWout::UUseMedkitDelieWout()
{
}

EBTNodeResult::Type UUseMedkitDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)return EBTNodeResult::Failed;

	UInventoryComponent* InventoryComponent = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComponent)return EBTNodeResult::Failed;

	auto Inventory = InventoryComponent->GetInventory();

	for (int i{ 0 }; i < Inventory.Num(); ++i)
	{
		if (Inventory[i] == nullptr) continue;
		if (Inventory[i]->GetItemType() == EItemType::Medkit)
		{
			InventoryComponent->UseItem(i);
			InventoryComponent->RemoveItem(i);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
