// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootGunDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
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

	// Find the gun slot.
	const auto& Items = Inventory->GetInventory();
	int GunSlot = -1;
	for (int i = 0; i < Items.Num(); ++i)
	{
		if (Items[i] && (Items[i]->GetItemType() == EItemType::Pistol || Items[i]->GetItemType() == EItemType::Shotgun))
		{
			GunSlot = i;
			break;
		}
	}
	if (GunSlot < 0) return EBTNodeResult::Failed;

	bool bRestoreUpright = false;
	if (UBlackboardComponent* BB = Controller->GetBlackboardComponent())
	{
		FVector AimAt;
		bool bHaveTarget = false;
		if (AActor* Zombie = Cast<AActor>(BB->GetValueAsObject("NearestZombie")))
		{
			AimAt = Zombie->GetActorLocation();
			bHaveTarget = true;
		}
		else if (BB->GetValueAsBool("WasBitten"))
		{
			AimAt = BB->GetValueAsVector("LastBiteLocation");
			bHaveTarget = true;
		}

		if (bHaveTarget)
		{
			const FVector ToTarget = AimAt - Pawn->GetActorLocation();
			if (!ToTarget.IsNearlyZero())
			{
				Pawn->SetActorRotation(ToTarget.Rotation());
				bRestoreUpright = true;
			}
		}
	}

	const FRotator Upright(0.f, Pawn->GetActorRotation().Yaw, 0.f);
	Inventory->UseItem(GunSlot);
	if (bRestoreUpright)
		Pawn->SetActorRotation(Upright);

	if (Items[GunSlot] && Items[GunSlot]->GetValue() <= 0)
		Inventory->RemoveItem(GunSlot);

	return EBTNodeResult::Succeeded;
}
