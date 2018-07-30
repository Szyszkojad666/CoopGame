// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Unit.h"

AUnitAIController::AUnitAIController()
{
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(FName("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(FName("BlackboardComp"));
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(FName("Perception"));


	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(FName("Sight Config"));

	Sight->SightRadius = 1000.f;
	Sight->LoseSightRadius = 1500.f;
	Sight->PeripheralVisionAngleDegrees = 130.f;

	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;


	PerceptionComp->ConfigureSense(*Sight);
}

void AUnitAIController::Possess(APawn * InPawn)
{
	Super::Possess(InPawn);

	if (BaseBehavior)
	{
		BlackboardComp->InitializeBlackboard(*BaseBehavior->BlackboardAsset);
		BehaviorTreeComp->StartTree(*BaseBehavior);
		BlackboardComp->SetValueAsObject("SelfActor", K2_GetPawn());
	}
	PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AUnitAIController::OnPerceptionUpdated);
	ControlledUnit = Cast<AUnit>(InPawn);
}



void AUnitAIController::SetEnemy(AUnit* InEnemy)
{

	if (InEnemy && InEnemy->GetTeamID() != ControlledUnit->GetTeamID() && InEnemy->IsAliveAndWell())
	{
		BlackboardComp->SetValueAsObject(BlackboardEnemyKey, InEnemy);
		return;
	}
	return;
}

AUnit * AUnitAIController::GetEnemy()
{
	UObject* object = BlackboardComp->GetValueAsObject(BlackboardEnemyKey);

	return object ? Cast <AUnit>(object) : nullptr;
	return nullptr;
}
