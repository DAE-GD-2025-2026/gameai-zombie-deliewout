// Fill out your copyright notice in the Description page of Project Settings.


#include "IsInventoryFullDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/ItemManager.h"

bool UIsInventoryFullDelieWout::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return false;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return false;

	UInventoryComponent* Inventory = Pawn->GetComponentByClass<UInventoryComponent>();
	if (!Inventory) return false;

	auto Items = Inventory->GetInventory();
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i] == nullptr)
			return false;
	}
	return true;
}