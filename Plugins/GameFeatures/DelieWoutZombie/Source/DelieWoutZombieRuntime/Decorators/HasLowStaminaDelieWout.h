// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "HasLowStaminaDelieWout.generated.h"

/**
 * 
 */
UCLASS()
class DELIEWOUTZOMBIERUNTIME_API UHasLowStaminaDelieWout : public UBTDecorator
{
	GENERATED_BODY()
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private: 
	UPROPERTY(EditDefaultsOnly)
	float MinStamina{ 50.0f };
};
