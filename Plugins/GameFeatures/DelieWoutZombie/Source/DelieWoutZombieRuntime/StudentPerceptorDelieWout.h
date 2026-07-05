// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptorDelieWout.generated.h"

class ABaseItem;
class AHouse;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DELIEWOUTZOMBIERUNTIME_API UStudentPerceptorDelieWout : public UActorComponent
{
	GENERATED_BODY()

public:
	UStudentPerceptorDelieWout();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void ForgetItem(ABaseItem* Item);

	UFUNCTION()
	void MarkHouseChecked(AHouse* House);
private: 
	UPROPERTY()
	TArray<ABaseItem*> RememberedItems;

	UPROPERTY()
	TArray<AHouse*> RememberedHouses;

	UPROPERTY()
	TArray<AHouse*> CheckedHouses;

	UPROPERTY()
	AHouse* CurrentTargetHouse{ nullptr };
};
