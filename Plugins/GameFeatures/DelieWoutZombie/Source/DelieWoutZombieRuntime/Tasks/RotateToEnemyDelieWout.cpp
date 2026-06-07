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

	ABaseZombie* Zombie = Cast<ABaseZombie>(BB->GetValueAsObject("NearestEnemy"));
	if (!Zombie) return EBTNodeResult::Failed;

	APawn* Pawn = Controller->GetPawn();
	FVector Direction = Zombie->GetActorLocation() - Pawn->GetActorLocation();
	FRotator NewRotation = Direction.Rotation();
	Pawn->SetActorRotation(NewRotation);

	return EBTNodeResult::Succeeded;
}
