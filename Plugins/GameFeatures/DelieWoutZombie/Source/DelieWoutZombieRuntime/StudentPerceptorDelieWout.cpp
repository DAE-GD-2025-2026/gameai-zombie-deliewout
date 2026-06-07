// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptorDelieWout.h"


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
}
