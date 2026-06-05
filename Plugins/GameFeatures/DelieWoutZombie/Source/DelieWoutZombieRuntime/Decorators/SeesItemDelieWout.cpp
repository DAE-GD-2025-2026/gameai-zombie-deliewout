// Fill out your copyright notice in the Description page of Project Settings.

#include "SeesItemDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Items/BaseItem.h"

bool USeesItemDelieWout::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return false;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return false;

	UAIPerceptionComponent* PerceptionComp = Pawn->GetComponentByClass<UAIPerceptionComponent>();
	if (!PerceptionComp) return false;

	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	for (AActor* Actor : PerceivedActors)
	{
		if (Cast<ABaseItem>(Actor))
			return true;
	}

	return false;
}
