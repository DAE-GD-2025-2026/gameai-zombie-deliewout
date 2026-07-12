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

void UStudentPerceptorDelieWout::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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

	ABaseZombie* NearestZombie = nullptr;
	AHouse* NearestHouse = nullptr;

	const FVector PawnLocation = Pawn->GetActorLocation();
	float EnemyDistance = FLT_MAX;
	float HouseDistance = FLT_MAX;

	for (AActor* SeenActor : SeenActors)
	{
		if (!IsValid(SeenActor)) continue;
		const float Distance = FVector::Dist(PawnLocation, SeenActor->GetActorLocation());

		if (ABaseZombie* Zombie = Cast<ABaseZombie>(SeenActor))
		{
			if (Distance < EnemyDistance) { EnemyDistance = Distance; NearestZombie = Zombie; }
		}
		else if (AHouse* House = Cast<AHouse>(SeenActor))
		{
			if (Distance < HouseDistance) { HouseDistance = Distance; NearestHouse = House; }
		}
	}

	BB->SetValueAsObject("NearestZombie", NearestZombie);
	BB->SetValueAsObject("NearestHouse", NearestHouse);

	// Forget items that are gone or already collected.
	RememberedItems.RemoveAll([Inventory](ABaseItem* It)
	{
		return !IsValid(It) || (Inventory && Inventory->GetInventory().Contains(It));
	});

	// First free inventory slot (any type).
	int32 FreeSlot = -1;
	if (Inventory)
	{
		const TArray<ABaseItem*>& Items = Inventory->GetInventory();
		for (int32 i = 0; i < Items.Num(); ++i)
		{
			if (Items[i] == nullptr) { FreeSlot = i; break; }
		}
	}

	ABaseItem* Target = Cast<ABaseItem>(BB->GetValueAsObject("NearestItem"));
	const bool bTargetStillValid = IsValid(Target)
		&& !(Inventory && Inventory->GetInventory().Contains(Target));

	if (!bTargetStillValid)
	{
		Target = nullptr;
		float BestDistance = FLT_MAX;
		for (ABaseItem* Item : RememberedItems)
		{
			const float Distance = FVector::Dist(PawnLocation, Item->GetActorLocation());
			if (Distance < BestDistance) { BestDistance = Distance; Target = Item; }
		}
	}

	if (Target && FreeSlot >= 0)
	{
		BB->SetValueAsObject("NearestItem", Target);
		BB->SetValueAsInt("FreeItemSlot", FreeSlot);
	}
	else
	{
		BB->SetValueAsObject("NearestItem", nullptr);
		BB->SetValueAsInt("FreeItemSlot", -1);
	}

	TArray<AActor*> ZombieBiters;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Damage::StaticClass(), ZombieBiters);
	BB->SetValueAsBool("WasBitten", ZombieBiters.Num() > 0);

}

void UStudentPerceptorDelieWout::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())return;

	if (ABaseItem* Item = Cast<ABaseItem>(Actor))
		RememberedItems.AddUnique(Item);
	else if (AHouse* House = Cast<AHouse>(Actor))
		RememberedHouses.AddUnique(House);
}

void UStudentPerceptorDelieWout::ForgetItem(ABaseItem* Item)
{
	RememberedItems.Remove(Item);
}

void UStudentPerceptorDelieWout::MarkHouseChecked(AHouse* House)
{
	if (House) CheckedHouses.AddUnique(House);
}