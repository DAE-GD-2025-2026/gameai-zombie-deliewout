// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GoToItemDelieWout.generated.h"

class ABaseItem;

UCLASS()
class DELIEWOUTZOMBIERUNTIME_API UGoToItemDelieWout : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UGoToItemDelieWout();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TWeakObjectPtr<ABaseItem> CachedItem;
	int32 CachedSlot{-1};
};
