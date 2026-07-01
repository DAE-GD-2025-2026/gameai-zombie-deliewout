// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptorDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Zombies/BaseZombie.h"
#include "Village/House/House.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"
#include "Common/InventoryComponent.h"

UStudentPerceptorDelieWout::UStudentPerceptorDelieWout()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptorDelieWout::BeginPlay()
{
	Super::BeginPlay();

	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptorDelieWout::OnPerceptionUpdated);
	}
}

void UStudentPerceptorDelieWout::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	AAIController* Controller = Cast<AAIController>(Pawn->GetController());
	if (!Controller) return;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return;

	UAIPerceptionComponent* PerceptionComp = Pawn->GetComponentByClass<UAIPerceptionComponent>();
	if (!PerceptionComp) return;

	TArray<AActor*> SeenActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);

	UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>();

	// Slot layout: Pistol=0, Shotgun=1, Medkit=2, Food=3, slot 4 kept free
	auto GetPreferredSlot = [](EItemType Type) -> int32
	{
		switch (Type)
		{
			case EItemType::Pistol:  return 0;
			case EItemType::Shotgun: return 1;
			case EItemType::Medkit:  return 2;
			case EItemType::Food:    return 3;
			default:                 return -1;
		}
	};

	// When bitten and unarmed, only seek weapons
	bool bHasWeapon = false;
	if (Inventory)
	{
		for (const ABaseItem* Item : Inventory->GetInventory())
		{
			if (Item && (Item->GetItemType() == EItemType::Pistol || Item->GetItemType() == EItemType::Shotgun))
			{
				bHasWeapon = true;
				break;
			}
		}
	}
	const bool bNeedsWeapon = BB->GetValueAsBool("WasBitten") && !bHasWeapon;

	ABaseZombie* NearestZombie = nullptr;
	AHouse* NearestHouse = nullptr;
	ABaseItem* NearestItem = nullptr;
	int32 TargetItemSlot = -1;

	FVector PawnLocation = Pawn->GetActorLocation();
	float EnemyDistance = FLT_MAX;
	float HouseDistance = FLT_MAX;
	float ItemDistance = FLT_MAX;

	for (AActor* SeenActor : SeenActors)
	{
		float Distance = FVector::Dist(PawnLocation, SeenActor->GetActorLocation());

		if (ABaseZombie* Zombie = Cast<ABaseZombie>(SeenActor))
		{
			if (Distance < EnemyDistance)
			{
				EnemyDistance = Distance;
				NearestZombie = Zombie;
			}
		}
		else if (AHouse* House = Cast<AHouse>(SeenActor))
		{
			if (Distance < HouseDistance)
			{
				HouseDistance = Distance;
				NearestHouse = House;
			}
		}
		else if (ABaseItem* Item = Cast<ABaseItem>(SeenActor))
		{
			int32 Slot = GetPreferredSlot(Item->GetItemType());
			if (Slot < 0) continue;

			// When bitten and unarmed, ignore food and medkits — weapons only
			if (bNeedsWeapon && Slot != 0 && Slot != 1) continue;

			if (Inventory && Inventory->GetInventory()[Slot] != nullptr) continue;

			if (Distance < ItemDistance)
			{
				ItemDistance = Distance;
				NearestItem = Item;
				TargetItemSlot = Slot;
			}
		}
	}

	BB->SetValueAsObject("NearestZombie", NearestZombie);
	BB->SetValueAsObject("NearestHouse", NearestHouse);

	if (NearestItem)
	{
		BB->SetValueAsObject("NearestItem", NearestItem);
		BB->SetValueAsInt("FreeItemSlot", TargetItemSlot);
	}
	else
	{
		ABaseItem* CachedBBItem = Cast<ABaseItem>(BB->GetValueAsObject("NearestItem"));
		if (!IsValid(CachedBBItem))
		{
			BB->SetValueAsObject("NearestItem", nullptr);
			BB->SetValueAsInt("FreeItemSlot", -1);
		}
	}

	TArray<AActor*> ZombieBiters;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Damage::StaticClass(), ZombieBiters);
	BB->SetValueAsBool("WasBitten", ZombieBiters.Num() > 0);
}

void UStudentPerceptorDelieWout::ForgetItem(ABaseItem* Item)
{
	RememberedItems.Remove(Item);
}