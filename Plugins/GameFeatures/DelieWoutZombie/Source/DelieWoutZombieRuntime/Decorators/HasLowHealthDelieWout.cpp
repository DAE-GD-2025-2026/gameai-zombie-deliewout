// Fill out your copyright notice in the Description page of Project Settings.


#include "HasLowHealthDelieWout.h"
#include "Common/HealthComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

bool UHasLowHealthDelieWout::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return false;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)return false;

	UHealthComponent* Health = Pawn->FindComponentByClass<UHealthComponent>();
	if (!Health)return false;
	GEngine->AddOnScreenDebugMessage(6, 5.f, FColor::Blue, TEXT("Check for health"));
	return Health->GetHealth() <=  MinHealth;

}