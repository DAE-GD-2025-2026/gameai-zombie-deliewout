// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "HasLowHealthDelieWout.generated.h"

/**
 * 
 */
UCLASS()
class DELIEWOUTZOMBIERUNTIME_API UHasLowHealthDelieWout : public UBTDecorator
{
	GENERATED_BODY()

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private: 
	UPROPERTY(EditDefaultsOnly)
	int MinHealth{5};
	
};
