// Fill out your copyright notice in the Description page of Project Settings.


#include "SeesEnemyDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

bool USeesEnemyDelieWout::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)return false;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB)return false;

	return BB->GetValueAsObject("NearestZombie") != nullptr;
}
