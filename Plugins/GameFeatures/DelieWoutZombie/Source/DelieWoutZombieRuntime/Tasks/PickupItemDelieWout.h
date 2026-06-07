// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PickupItemDelieWout.generated.h"

/**
 * 
 */
UCLASS()
class DELIEWOUTZOMBIERUNTIME_API UPickupItemDelieWout : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UPickupItemDelieWout();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
