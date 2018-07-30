// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Structs.h"
#include "GameFramework/GameModeBase.h"
#include "CoopGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, InstigatorController);

/**
 * 
 */
UCLASS()
class COOPGAME_API ACoopGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	virtual void StartPlay() override;

	ACoopGameMode();

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;

protected:
	
	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_PlayerState;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		float SpawnInterval;

	//Number of bots to spawn in a single wave
	
		

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		int32 MaxAliveUnits;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
		float TimeBetweenWaves;

	UPROPERTY(EditAnywhere, Category = "GameMode")
		TArray<FWaveData> WaveData;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
		void OnSpawnedActor();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
		void DynamicSpawn(FVector SpawnLocation, UClass* ClassToSpawn);

	void ValidateSpawnedActors();

	void CheckPlayerState();

	void GameOver();

	int32 OpponentsToSpawn;
	bool WaveCleared();

	int32 WaveCount;
	
	int32 SpawnedActorCount;

	void StartWave();

	void EndWave();
	
	void PrepareForNextWave();

	void SpawnBotTimerElapsed();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();

	TArray<AActor*> SpawnedActors;
};
