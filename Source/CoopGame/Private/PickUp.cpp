// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUp.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "PowerUp.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APickUp::APickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(RootComponent);
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64, 75, 75);

	RespawnCooldown = 10.0f;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
	Super::BeginPlay();
	if (Role == ROLE_Authority)
	{
		RespawnPickUp();
	}
	
}

void APickUp::RespawnPickUp()
{
	if (PowerUpClass == nullptr || PowerUpInstance != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerUp Class is null, please update the blueprint"), *GetName());
		return;
	}
	FActorSpawnParameters SpawnParams;
	PowerUpInstance = GetWorld()->SpawnActor<APowerUp>(PowerUpClass, GetActorTransform(), SpawnParams);
}

void APickUp::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (Role == ROLE_Authority && PowerUpInstance)
	{
		PowerUpInstance->ActivatePowerUp(Cast<APawn>(OtherActor));
		PowerUpInstance = nullptr;

		//Set Timer to respawn
		GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &APickUp::RespawnPickUp, RespawnCooldown);

	}
}


