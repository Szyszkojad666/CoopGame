// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Destructible_Base.generated.h"

class UStaticMeshComponent;
class UParticleSystem;
class UHealthComponent;
class UAudioComponent;
class UCapsuleComponent;
class URadialForceComponent;
class UMaterialInterface;

UCLASS()
class COOPGAME_API ADestructible_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADestructible_Base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	bool bDestroyed;

public:	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UAudioComponent* AudioComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UParticleSystem* FX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UCapsuleComponent* CapsuleCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		URadialForceComponent* RadialForceComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UMaterialInterface* DestroyedMaterial;

	UFUNCTION()
		void OnHealthChanged(UHealthComponent* HealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDestroyedImplementation();

	void Destroy();

	UFUNCTION(NetMultiCast, Reliable)
		void MultiCastDestroy();

};
