// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ExploreDelieWout.generated.h"

struct FExploreMemory
{
	float WanderAngle;
};

UCLASS()
class DELIEWOUTZOMBIERUNTIME_API UExploreDelieWout : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UExploreDelieWout();

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FExploreMemory); }
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Explore")
	float SearchRadius{1000.f};

	UPROPERTY(EditAnywhere, Category = "Explore")
	float WanderWeight{1.f};

	UPROPERTY(EditAnywhere, Category = "Explore")
	float FleeWeight{2.f};

	UPROPERTY(EditAnywhere, Category = "Explore")
	float WanderJitter{45.f};
};
