// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUp.generated.h"

UCLASS()
class COOPGAME_API APowerUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APowerUp();

	UFUNCTION(BlueprintImplementableEvent)
		void OnActive(APawn* Pawn);


	UFUNCTION(BlueprintImplementableEvent)
		void OnTicked();

	UFUNCTION(BlueprintImplementableEvent)
		void OnExpired();

	UFUNCTION()
		void OnTickPowerUp();

	FTimerHandle TimerHandle_PowerUpTick;
	
	/*Total number of times the effect is applied*/
	UPROPERTY(EditDefaultsOnly, Category = "PowerUp")
		int32 NumberOfTicks;

	UPROPERTY(EditDefaultsOnly, Category = "PowerUp")
		float EffectPeriod;

	

	void ActivatePowerUp(APawn* Pawn);

	
	
	

	UFUNCTION(BlueprintImplementableEvent, Category = "PowerUp")
		void OnPowerUpStateChange(bool bNewIsActive);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_PowerUpActive)
		bool bIsPowerUpActive;
	
	UFUNCTION()
		void OnRep_PowerUpActive();
	
};
