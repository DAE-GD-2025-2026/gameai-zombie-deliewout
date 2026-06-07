// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptorDelieWout.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Zombies/BaseZombie.h"


UStudentPerceptorDelieWout::UStudentPerceptorDelieWout()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptorDelieWout::BeginPlay()
{
	Super::BeginPlay();

	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptorDelieWout::OnPerceptionUpdated);
	}
}

void UStudentPerceptorDelieWout::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw Something!")));

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	AAIController* AIController = Cast<AAIController>(Pawn->GetController());
	if (!AIController) return;

	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (!BB) return;

	UAIPerceptionComponent* PerceptionComp = Pawn->FindComponentByClass<UAIPerceptionComponent>();
	if (!PerceptionComp) return;

	TArray<AActor*> SeenActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);

	ABaseZombie* NearestZombie=nullptr;

	FVector PawnLocation = Pawn->GetActorLocation();
	float EnemyDistance = FLT_MAX;

	for (AActor* SeenActor : SeenActors)
	{
		float Distance = FVector::Dist(PawnLocation, SeenActor->GetActorLocation());
		if (ABaseZombie* Zombie = Cast<ABaseZombie>(SeenActor))
		{
			if (Distance < EnemyDistance)
			{
				EnemyDistance = Distance;
				NearestZombie = Zombie;
			}
		}
	}

	BB->SetValueAsObject("NearestZombie", NearestZombie);

	//set the wasbitten value depending on if the player was damaged
	TArray<AActor*> ZombieBiters;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Damage::StaticClass(), ZombieBiters);
	BB->SetValueAsBool("WasBitten", ZombieBiters.Num() > 0);
}
