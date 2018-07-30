// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Structs.h"
#include "CoopGameState.generated.h"


/**
 * 
 */
UCLASS()
class COOPGAME_API ACoopGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
		void WaveStateChanged(EWaveState NewState, EWaveState OldState);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
		EWaveState WaveState;

public:

	
	
	UFUNCTION(BlueprintCallable)
		void SetNewWaveState(EWaveState NewState);
	
};
