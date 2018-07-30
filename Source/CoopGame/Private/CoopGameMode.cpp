// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopGameMode.h"
#include "TimerManager.h"
#include "TrackerBot.h"
#include "Unit.h"
#include "TrackerBot.h"
#include "Components/HealthComponent.h"
#include "Classes/Engine/World.h"
#include "Structs.h"
#include "CoopPlayerState.h"
#include "CoopGameState.h"

ACoopGameMode::ACoopGameMode()
{
	TimeBetweenWaves = 2;
	SpawnedActorCount = 0;
	OpponentsToSpawn = 1;
	WaveCount = 0;
	GameStateClass = ACoopGameState::StaticClass();
	PlayerStateClass = ACoopPlayerState::StaticClass();
}

void ACoopGameMode::DynamicSpawn(FVector SpawnLocation, UClass* ClassToSpawn)
{

	if (OpponentsToSpawn > 0)
	{
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnParams;

		AActor* Owner = NULL;
		APawn* Instigator = NULL;
		bool bNoCollisionFail = false;
		UWorld* const World = GetWorld();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (World && ClassToSpawn)
		{
			AActor* SpawnedActor = World->SpawnActor<AActor>(ClassToSpawn, SpawnLocation + FVector(0.0f, 0.0f, 50.0f), Rotation, SpawnParams);
			SpawnedActors.Add(SpawnedActor);
			SpawnedActorCount++;
			OpponentsToSpawn--;

		}
	}
	else
	{
		SetWaveState(EWaveState::WaitingToComplete);
	}
}

bool ACoopGameMode::WaveCleared()
{
	if (SpawnedActors.Num() <= 0)
	{
		return false;
	}
	ValidateSpawnedActors();
	if (SpawnedActors.Num() > 0)
	{
		for (int32 i = 0; i < SpawnedActors.Num(); i++)
		{
			AUnit* UnitToCheck = Cast<AUnit>(SpawnedActors[i]);
			ATrackerBot* BotToCheck = nullptr;
			UHealthComponent* HealthComp = nullptr;
			if (UnitToCheck == nullptr)
			{
				BotToCheck = Cast<ATrackerBot>(SpawnedActors[i]);
				if (BotToCheck)
				{
					HealthComp = BotToCheck->GetHealthComponent();
				}

			}
			else
			{
				HealthComp = UnitToCheck->GetHealthComponent();
			}

			if (HealthComp && HealthComp->GetHealth() > 0)
			{
				return false;
				break;
			}
		}


	}
	return true;
}

void ACoopGameMode::ValidateSpawnedActors()
{
	if (SpawnedActors.Num() > 0)
	{
		for (int32 i = 0; i < SpawnedActors.Num(); i++)
		{
			if (!IsValid(SpawnedActors[i]))
			{
				SpawnedActors.RemoveAt(i);
			}
		}
	}
}

void ACoopGameMode::CheckPlayerState()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			AUnit* Player = Cast<AUnit>(PC->GetPawn());
			if (Player)
			{
				UHealthComponent* HealthComp = Player->GetHealthComponent();
				if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
				{
					return;
				}
			}
		}
	}

	GameOver();
}

void ACoopGameMode::GameOver()
{
	EndWave();
	SetWaveState(EWaveState::GameOver);
	UE_LOG(LogTemp, Log, TEXT("GAME OVER! All the player are dead"));
}

void ACoopGameMode::StartWave()
{
	SetWaveState(EWaveState::WaitingToStart);
	WaveCount++;


	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ACoopGameMode::SpawnBotTimerElapsed, SpawnInterval, true, 0.0f);
}

void ACoopGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	PrepareForNextWave();
}

void ACoopGameMode::PrepareForNextWave()
{
	FTimerHandle TimerHandle_NextWaveStart;
	if (WaveData.Num() > WaveCount)
	{
		OpponentsToSpawn = WaveData[WaveCount].NumberOfOpponents;
		GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ACoopGameMode::StartWave, 1.0f, false, 0.0f);
		return;
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_NextWaveStart);
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}

void ACoopGameMode::SpawnBotTimerElapsed()
{

	OnSpawnedActor();
	SetWaveState(EWaveState::PreparingWave);
	if (WaveCleared())
	{
		SetWaveState(EWaveState::WaveComplete);
		EndWave();
	}
}

void ACoopGameMode::SetWaveState(EWaveState NewState)
{
	ACoopGameState* GameState = GetGameState<ACoopGameState>();
	if (ensureAlways(GameState))
	{
		GameState->SetNewWaveState(NewState);
	}
}

void ACoopGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}

void ACoopGameMode::StartPlay()
{
	Super::StartPlay();


	GetWorldTimerManager().SetTimer(TimerHandle_PlayerState, this, &ACoopGameMode::CheckPlayerState, 1.0f, true, 3.0f);
	PrepareForNextWave();
}


