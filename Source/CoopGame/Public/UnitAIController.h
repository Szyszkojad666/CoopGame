// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "UnitAIController.generated.h"

/**
 * 
 */
class UBehaviorTreeComponent;
class AUnit;

UCLASS()
class COOPGAME_API AUnitAIController : public AAIController

{
	GENERATED_BODY()
	
private:
	
	UBlackboardComponent* BlackboardComp;

	UBehaviorTreeComponent* BehaviorTreeComp;

	const FName BlackboardEnemyKey = FName("Enemy");

	AUnit* ControlledUnit;

	UAISenseConfig_Sight* Sight;
	
protected:

	UPROPERTY(EditAnywhere)
		UBehaviorTree* BaseBehavior;

	UPROPERTY(EditAnywhere)
		UBehaviorTree* CombatBehavior;

	UPROPERTY(VisibleAnywhere)
		UAIPerceptionComponent* PerceptionComp;

public:
	AUnitAIController();

	virtual void Possess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
	AUnit* GetEnemy();

	UFUNCTION(BlueprintImplementableEvent)
		void OnPerceptionUpdated(const TArray<AActor*> &UpdatedActors);

	UFUNCTION(BlueprintCallable, Category = "Unit")
		FORCEINLINE AUnit* GetControlledUnit() const { if (ControlledUnit) return ControlledUnit; else return nullptr; }

	UFUNCTION(BlueprintCallable)
		void SetEnemy(AUnit* InEnemy);
	
};
