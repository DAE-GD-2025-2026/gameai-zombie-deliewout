// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptorDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Zombies/BaseZombie.h"
#include "Village/House/House.h"
#include "Items/BaseItem.h"
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
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw Something!")));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	AAIController* Controller = Cast<AAIController>(Pawn->GetController());
	if (!Controller) return;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return;

	UAIPerceptionComponent* PerceptionComp = Pawn->GetComponentByClass<UAIPerceptionComponent>();
	if (!PerceptionComp) return;


	//Gets all the seen actors
	TArray<AActor*> SeenActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);

	ABaseZombie* NearestZombie=nullptr;
	AHouse* NearestHouse=nullptr;
	ABaseItem* NearestItem=nullptr;

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
			if (Distance < ItemDistance)
			{
				ItemDistance = Distance;
				NearestItem = Item;
			}
		}
	}

	BB->SetValueAsObject("NearestZombie", NearestZombie);
	BB->SetValueAsObject("NearestHouse", NearestHouse);
	BB->SetValueAsObject("NearestItem", NearestItem);

	//looks for a free item spot in the inventory
	UInventoryComponent* Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
	if (Inventory)
	{
		int FreeSlot = -1;
		for (int i = 0; i < Inventory->GetInventoryCapacity(); ++i)
		{
			if (Inventory->GetInventory()[i] == nullptr) { FreeSlot = i; break; }
		}
		BB->SetValueAsInt("FreeItemSlot", FreeSlot);
	}

	//set the wasbitten value depending on if the player was damaged
	TArray<AActor*> ZombieBiters;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Damage::StaticClass(), ZombieBiters);
	BB->SetValueAsBool("WasBitten", ZombieBiters.Num() > 0);
}
