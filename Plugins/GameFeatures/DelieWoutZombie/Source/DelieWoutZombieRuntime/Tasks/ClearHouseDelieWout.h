// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ClearHouseDelieWout.generated.h"

/**
 * 
 */
UCLASS()
class DELIEWOUTZOMBIERUNTIME_API UClearHouseDelieWout : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UClearHouseDelieWout();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TUniquePtr<AHouse> TargetHouse;
};
