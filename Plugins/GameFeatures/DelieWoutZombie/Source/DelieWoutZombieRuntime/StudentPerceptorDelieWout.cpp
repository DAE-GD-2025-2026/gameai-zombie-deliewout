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
#include "DrawDebugHelpers.h"

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
	if (bDrawExplorationGrid) DrawExplorationGrid(PawnLocation);
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

bool UStudentPerceptorDelieWout::GetNextExploreTarget(const FVector& PawnLocation, const AActor* /*Threat*/, FVector& OutTarget)
{
	if (!bGridInitialized) return false;

	const int Idx = CellIndexFromLocation(PawnLocation);
	const int CurX = Idx % NumCellsX;
	const int CurY = Idx / NumCellsX;

	// (Re)anchor the spiral at the survivor's current cell when starting fresh,
	// or when a detour (pickup, flee) has carried it well past the spiral.
	const int DistFromCenter = bSpiralActive? FMath::Max(FMath::Abs(CurX - SpiralCenterX), FMath::Abs(CurY - SpiralCenterY)) : 0;
	if (!bSpiralActive || DistFromCenter > SpiralRing + SpiralReanchorCells)
	{
		SpiralCenterX = CurX;
		SpiralCenterY = CurY;
		SpiralRing = 0;
		bSpiralActive = true;
	}

	// Walk an expanding square spiral outward from the anchor and return the first in-bounds, unvisited cell.
	const int MaxRing = FMath::Max(NumCellsX, NumCellsY);
	const int TotalCells = (2 * MaxRing + 1) * (2 * MaxRing + 1);

	int x = 0;
	int y = 0;     // offset from the anchor
	int dx = 1;
	int dy = 0;    // step direction (start +X)
	int legLen = 1;
	int legStep = 0;
	int legsDone = 0;

	for (int Step = 0; Step < TotalCells; ++Step)
	{
		const int CX = SpiralCenterX + x;
		const int CY = SpiralCenterY + y;
		if (CX >= 0 && CX < NumCellsX && CY >= 0 && CY < NumCellsY)
		{
			const int CellIdx = CY * NumCellsX + CX;
			if (!CellVisited[CellIdx])
			{
				SpiralRing = FMath::Max(FMath::Abs(x), FMath::Abs(y));
				OutTarget = CalculateCellCenter(CellIdx);
				LastExploreTarget = OutTarget;
				bHasExploreTarget = true;
				return true;
			}
		}

		// Advance along the spiral (legs 1,1,2,2,3,3,... turning 90°).
		x += dx; 
		y += dy;

		if (++legStep == legLen)
		{
			legStep = 0;
			const int ndx = -dy;
			const int ndy = dx;   // rotate direction
			dx = ndx; 
			dy = ndy;
			if (++legsDone == 2) 
				{ legsDone = 0; ++legLen; }
		}
	}

	// Whole map covered: reset for a fresh sweep, caller wanders this once.
	CellVisited.Init(0, CellVisited.Num());
	bSpiralActive = false;
	bHasExploreTarget = false;
	return false;
}

void UStudentPerceptorDelieWout::MarkCellUnreachable(const FVector& Location)
{
	if (!bGridInitialized) return;
	CellVisited[CellIndexFromLocation(Location)] = 1;
}

void UStudentPerceptorDelieWout::InitExplorationGrid()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys || !NavSys->MainNavData) return;

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

void UStudentPerceptorDelieWout::DrawExplorationGrid(const FVector& PawnLocation) const
{
	if (!bGridInitialized) return;

	UWorld* World = GetWorld();
	const float HalfCell = CellSize * 0.5f;
	const FVector BoxExtent(HalfCell - 20.f, HalfCell - 20.f, 20.f);

	// Red = unexplored, green = visited/unreachable. Drawn flat at the agent's height.
	for (int i = 0; i < CellVisited.Num(); ++i)
	{
		FVector Center = CalculateCellCenter(i);
		Center.Z = PawnLocation.Z;
		const FColor Color = CellVisited[i] ? FColor::Green : FColor::Red;
		const float Thickness = CellVisited[i] ? 4.f : 20.f;
		DrawDebugBox(World, Center, BoxExtent, Color, false, -1.f, 0, Thickness);
	}

	// The radius that marks cells as visited.
	DrawDebugCircle(World, PawnLocation, VisitRadius, 32, FColor::Cyan, false, -1.f, 0, 10.f,
		FVector(1, 0, 0), FVector(0, 1, 0), false);

	// Where the explore task is currently headed.
	if (bHasExploreTarget)
	{
		FVector Target = LastExploreTarget;
		Target.Z = PawnLocation.Z;
		DrawDebugSphere(World, Target, 100.f, 12, FColor::Yellow, false, -1.f, 0, 6.f);
		DrawDebugDirectionalArrow(World, PawnLocation, Target, 500.f, FColor::Yellow, false, -1.f, 0, 10.f);
	}
}
