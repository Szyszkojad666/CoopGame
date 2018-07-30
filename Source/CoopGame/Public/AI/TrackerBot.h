// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TrackerBot.generated.h"

class UStaticMeshComponent;
class UParticleSystem;
class UHealthComponent;
class UAudioComponent;
class USphereComponent;
class URadialForceComponent;
class UMaterialInterface;
class USoundCue;

UCLASS()
class COOPGAME_API ATrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATrackerBot();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UHealthComponent* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USphereComponent* SphereCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UAudioComponent* AudioComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UParticleSystem* FX;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		URadialForceComponent* RadialForceComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UMaterialInterface* DestroyedMaterial;
	
	UFUNCTION(BlueprintCallable)
		FORCEINLINE UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UPROPERTY(Replicated, BlueprintReadOnly)
		bool bDestroyed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxPowerLevel;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float PowerLevel;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	FVector GetNextPathPoint();

	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Navigation")
	float MovementForce;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Navigation")
	bool bUseVelocityChange;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Navigation")
	float RequiredDistanceToPathPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float ExplosionDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
		float ExplosionRadius;

	UMaterialInstanceDynamic* MatInst;

	FTimerHandle TimerHande_DamageSelf;

	FTimerHandle TimerHandle_UpdatePowerLevel;

	bool bSelfDestructionSequence;

	void DamageSelf();

	


public:
	UFUNCTION()
		void OnHealthChanged(UHealthComponent* HealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

		void MoveToPathPoint();

	UFUNCTION(BlueprintImplementableEvent)
		void OnDestroyedImplementation();

	void Destroy();

	UFUNCTION(NetMultiCast, Reliable)
		void MultiCastDestroy();

	UFUNCTION()
		void InnitiateSelfDestruct();

	UFUNCTION(NetMultiCast, Reliable)
		void MultiCastInnitiateSelfDestruct();

	UFUNCTION()
		void FlashMaterial();

	UFUNCTION(NetMultiCast, Reliable)
		void MultiCastFlashMaterial();
	
	UFUNCTION()
		void UpdatePowerLevel();
	
	UFUNCTION()
		void UpdateMaterialParam();

	UFUNCTION(NetMultiCast, Reliable)
		void MultiCastUpdateMaterialParam();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	
};
