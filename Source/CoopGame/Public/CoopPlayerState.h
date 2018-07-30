// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CoopPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ACoopPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		void AddScore(float ScoreDelta);
	
	
};
