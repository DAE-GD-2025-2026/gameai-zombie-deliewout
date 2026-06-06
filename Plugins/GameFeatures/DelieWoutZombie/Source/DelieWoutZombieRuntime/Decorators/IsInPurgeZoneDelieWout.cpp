// Fill out your copyright notice in the Description page of Project Settings.


#include "IsInPurgeZoneDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "PurgeZones/PurgeZone.h"

bool UIsInPurgeZoneDelieWout::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return false;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return false;

	TArray<AActor*> OverlappingActors;
	Pawn->GetOverlappingActors(OverlappingActors, APurgeZone::StaticClass());

	return OverlappingActors.Num()>0;
}
