// Fill out your copyright notice in the Description page of Project Settings.


#include "HasGunDelieWout.h"
#include "AIController.h"
#include "Common/InventoryComponent.h"
#include "Items/Weapon.h"

bool UHasGunDelieWout::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return false;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)return false;

	UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)return false;

	for (const auto& Item : Inventory->GetInventory())
	{
		if (Item == nullptr) continue;
		if (Item->GetItemType() == EItemType::Pistol || Item->GetItemType() == EItemType::Shotgun)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Has gun"));
			return true;
		}
	}

	return false;
}