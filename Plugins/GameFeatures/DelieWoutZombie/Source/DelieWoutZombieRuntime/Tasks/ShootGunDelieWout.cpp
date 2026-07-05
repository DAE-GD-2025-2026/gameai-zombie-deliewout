// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGunDelieWout.h"
#include "AIController.h"
#include "Common/InventoryComponent.h"

UShootGunDelieWout::UShootGunDelieWout()
{
}

EBTNodeResult::Type UShootGunDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return EBTNodeResult::Failed;

	const auto& Items = Inventory->GetInventory();

	for (int i{ 0 }; i < Items.Num(); ++i)
	{
		if (!Items[i])continue;
		if (Items[i]->GetItemType() == EItemType::Pistol || Items[i]->GetItemType() == EItemType::Shotgun)
		{
			Inventory->UseItem(i);
			if (Items[i]->GetValue() <= 0)
				Inventory->RemoveItem(i);
			return EBTNodeResult::Succeeded;
		}
	}

    return EBTNodeResult::Failed;
}
