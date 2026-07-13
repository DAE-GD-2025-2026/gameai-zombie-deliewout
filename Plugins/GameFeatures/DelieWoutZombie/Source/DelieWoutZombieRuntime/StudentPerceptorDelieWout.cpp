// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptorDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Zombies/BaseZombie.h"
#include "Village/House/House.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"
#include "Common/InventoryComponent.h"
#include "NavigationSystem.h"

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
	if (!bGridInitialized) InitExplorationGrid();
		MarkVisitedCells(PawnLocation);
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

bool UStudentPerceptorDelieWout::GetNextExploreTarget(const FVector& PawnLocation, const AActor* Threat, FVector& OutTarget)
{
	if (!bGridInitialized) return false;

	FVector ThreatDir = FVector::ZeroVector;
	if (IsValid(Threat))
		ThreatDir = (Threat->GetActorLocation() - PawnLocation).GetSafeNormal2D();

	int32 BestIndex = -1;
	float BestScore = FLT_MAX;
	for (int32 i = 0; i < CellVisited.Num(); ++i)
	{
		if (CellVisited[i]) continue;

		const FVector Center = CalculateCellCenter(i);
		float Score = FVector::Dist2D(Center, PawnLocation);
		if (!ThreatDir.IsNearlyZero())
		{
			const FVector ToCell = (Center - PawnLocation).GetSafeNormal2D();
			Score += ThreatPenalty * FMath::Max(0.f, FVector::DotProduct(ToCell, ThreatDir));
		}
		if (Score < BestScore) { BestScore = Score; BestIndex = i; }
	}

	if (BestIndex == -1)
	{
		// Whole map covered: reset for a fresh sweep, caller wanders this once.
		CellVisited.Init(0, CellVisited.Num());
		return false;
	}

	OutTarget = CalculateCellCenter(BestIndex);
	return true;
}

void UStudentPerceptorDelieWout::MarkCellUnreachable(const FVector& Location)
{
	if (!bGridInitialized) return;
	CellVisited[CellIndexFromLocation(Location)] = 1;
}

void UStudentPerceptorDelieWout::InitExplorationGrid()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return;

	const FBox NavBounds = NavSys->MainNavData->GetBounds();
	if (!NavBounds.IsValid) return;

	GridOrigin = FVector2D(NavBounds.Min.X, NavBounds.Min.Y);
	NumCellsX = FMath::Max(1, FMath::CeilToInt((NavBounds.Max.X - NavBounds.Min.X) / CellSize));
	NumCellsY = FMath::Max(1, FMath::CeilToInt((NavBounds.Max.Y - NavBounds.Min.Y) / CellSize));
	CellVisited.Init(0, NumCellsX * NumCellsY);
	bGridInitialized = true;
}

void UStudentPerceptorDelieWout::MarkVisitedCells(const FVector& Location)
{
	if (!bGridInitialized) return;

	const int CellRange = FMath::CeilToInt(VisitRadius / CellSize);
	const int CenterX = FMath::FloorToInt((Location.X - GridOrigin.X) / CellSize);
	const int CenterY = FMath::FloorToInt((Location.Y - GridOrigin.Y) / CellSize);

	for (int Y = CenterY - CellRange; Y <= CenterY + CellRange; ++Y)
	{
		if (Y < 0 || Y >= NumCellsY) continue;
		for (int X = CenterX - CellRange; X <= CenterX + CellRange; ++X)
		{
			if (X < 0 || X >= NumCellsX) continue;
			const int Index = Y * NumCellsX + X;
			if (FVector::Dist2D(CalculateCellCenter(Index), Location) <= VisitRadius)
				CellVisited[Index] = 1;
		}
	}
}

int UStudentPerceptorDelieWout::CellIndexFromLocation(const FVector& Location) const
{
	const int X = FMath::Clamp(FMath::FloorToInt((Location.X - GridOrigin.X) / CellSize), 0, NumCellsX - 1);
	const int Y = FMath::Clamp(FMath::FloorToInt((Location.Y - GridOrigin.Y) / CellSize), 0, NumCellsY - 1);
	return Y * NumCellsX + X;
}

FVector UStudentPerceptorDelieWout::CalculateCellCenter(int Index) const
{
	const int X = Index % NumCellsX;
	const int Y = Index / NumCellsX;
	return FVector(GridOrigin.X + (X + 0.5f) * CellSize, GridOrigin.Y + (Y + 0.5f) * CellSize, 0.f);
}
