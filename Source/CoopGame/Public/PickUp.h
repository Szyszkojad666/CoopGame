// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

class USphereComponent;
class UDecalComponent;
class APowerUp;

UCLASS()
class COOPGAME_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USphereComponent* SphereComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickUp Actor")
		float RespawnCooldown;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, Category = "PickUp Actor")
		TSubclassOf<APowerUp> PowerUpClass;
	UFUNCTION()
	void RespawnPickUp();

	APowerUp* PowerUpInstance;

	FTimerHandle TimerHandle_Respawn;

public:	
	// Called every frame
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	
};
