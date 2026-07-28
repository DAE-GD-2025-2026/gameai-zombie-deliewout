// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptorDelieWout.generated.h"

class ABaseItem;
class AHouse;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DELIEWOUTZOMBIERUNTIME_API UStudentPerceptorDelieWout : public UActorComponent
{
	GENERATED_BODY()

public:
	UStudentPerceptorDelieWout();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void ForgetItem(ABaseItem* Item);

	UFUNCTION()
	void MarkHouseChecked(AHouse* House);

	UFUNCTION()
	bool GetNextExploreTarget(const FVector& PawnLocation, const AActor* Threat, FVector& OutTarget);
	UFUNCTION()
	void MarkCellUnreachable(const FVector& Location);
private: 
	UPROPERTY()
	TArray<ABaseItem*> RememberedItems;

	UPROPERTY()
	TArray<AHouse*> RememberedHouses;

	UPROPERTY()
	TArray<AHouse*> CheckedHouses;

	UPROPERTY()
	AHouse* CurrentTargetHouse{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Exploration")
	float CellSize{ 800.f };

	UPROPERTY(EditAnywhere, Category = "Exploration")
	float VisitRadius{ 800.f };

	//if an enemy is seen then the threat penalty is applied
	UPROPERTY(EditAnywhere, Category = "Exploration")
	float ThreatPenalty{ 3000.f };
	UPROPERTY(EditAnywhere, Category = "Exploration")
	int SpiralReanchorCells{ 4 };   // restart the spiral if displaced this many cells past its radius
	UPROPERTY(EditAnywhere, Category = "Exploration|Debug")
	bool bDrawExplorationGrid{ true };

	void InitExplorationGrid();
	void MarkVisitedCells(const FVector& Location);
	int CellIndexFromLocation(const FVector& Location) const;
	FVector CalculateCellCenter(int Index)const;
	void DrawExplorationGrid(const FVector& PawnLocation) const;

	FVector2D GridOrigin{ FVector2D::ZeroVector };
	int NumCellsX{ 0 };
	int NumCellsY{ 0 };
	TArray<int> CellVisited;
	bool bGridInitialized{ false };

	// Only for debug drawing: last target handed out by GetNextExploreTarget.
	FVector LastExploreTarget{ FVector::ZeroVector };
	bool bHasExploreTarget{ false };
	// Spiral search: the current outward spiral expands from this anchor cell.
	int SpiralCenterX{ -1 };
	int SpiralCenterY{ -1 };
	int SpiralRing{ 0 };
	bool bSpiralActive{ false };
};
