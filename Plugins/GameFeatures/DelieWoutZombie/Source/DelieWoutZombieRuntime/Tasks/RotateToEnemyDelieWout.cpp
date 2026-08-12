// Fill out your copyright notice in the Description page of Project Settings.


#include "RotateToEnemyDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Zombies/BaseZombie.h"

URotateToEnemyDelieWout::URotateToEnemyDelieWout()
{

}

EBTNodeResult::Type URotateToEnemyDelieWout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB)return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)return EBTNodeResult::Failed;

	FVector TargetLocation;
	if (ABaseZombie* Zombie = Cast<ABaseZombie>(BB->GetValueAsObject("NearestZombie")))
	{
		TargetLocation = Zombie->GetActorLocation();
	}
	else if (BB->GetValueAsBool("WasBitten"))
	{
		TargetLocation = BB->GetValueAsVector("LastBiteLocation");
	}
	else
	{
		return EBTNodeResult::Failed;
	}


	FVector Direction = TargetLocation - Pawn->GetActorLocation();
	FRotator NewRotation(0.f, Direction.Rotation().Yaw, 0.f);
	Pawn->SetActorRotation(NewRotation);

	return EBTNodeResult::Succeeded;
}
