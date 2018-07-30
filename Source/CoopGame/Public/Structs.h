// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs.generated.h"

/**
 * 
 */
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
		FVector_NetQuantize TraceTo;
};

USTRUCT(BlueprintType)
struct FWaveData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode")
		int32 NumberOfOpponents;

	FWaveData()
	{
		NumberOfOpponents = 1;
	}
};
	
UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,

	PreparingWave,

	WaveInProgress,

	//Not spawning bots, waiting for player to finish the wave
	WaitingToComplete,

	WaveComplete, 

	GameOver,
};

