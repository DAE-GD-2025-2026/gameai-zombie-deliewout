// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "IsInventoryFullDelieWout.generated.h"

/**
 * 
 */
UCLASS()
class DELIEWOUTZOMBIERUNTIME_API UIsInventoryFullDelieWout : public UBTDecorator
{
	GENERATED_BODY()
	
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
};
