// Fill out your copyright notice in the Description page of Project Settings.


#include "HasLowStaminaDelieWout.h"
#include "Common/StaminaComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"

bool UHasLowStaminaDelieWout::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return false;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)return false;

	UStaminaComponent* Stamina = Pawn->FindComponentByClass<UStaminaComponent>();
	if (!Stamina)return false;

	return Stamina->GetCurrentStamina() <= Stamina->GetMaxStamina() / 100 * MinStamina;
}