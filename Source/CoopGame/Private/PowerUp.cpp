// Fill out your copyright notice in the Description page of Project Settings.

#include "PowerUp.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
// Sets default values
APowerUp::APowerUp()
{
	EffectPeriod = 0;
	NumberOfTicks = 0.0f;
	SetReplicates(true);
	bIsPowerUpActive = false;
}

void APowerUp::OnTickPowerUp()
{
	static int32 TicksProcessed;
	if (TicksProcessed >= NumberOfTicks)
	{
		OnExpired();
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerUpTick);
		TicksProcessed = 0;
		bIsPowerUpActive = false;
		OnRep_PowerUpActive();
	}
	else
	{
		TicksProcessed++;
		OnTicked();
	}

}

void APowerUp::ActivatePowerUp(APawn* Pawn)
{
	OnActive(Pawn);
	bIsPowerUpActive = true;
	OnRep_PowerUpActive();

	if (EffectPeriod > 0)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerUpTick, this, &APowerUp::OnTickPowerUp, EffectPeriod, true);
	}

	else
	{
		OnTickPowerUp();
	}
}

// Called when the game starts or when spawned
void APowerUp::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void APowerUp::OnRep_PowerUpActive()
{
	OnPowerUpStateChange(bIsPowerUpActive);
}


void APowerUp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APowerUp, bIsPowerUpActive);
	
}

