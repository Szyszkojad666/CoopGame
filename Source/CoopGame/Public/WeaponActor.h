// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Structs.h"
#include "WeaponActor.generated.h"



class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UParticleSystemComponent;
class AUnit;
class UAnimMontage;
class UCameraShake;



UCLASS()
class COOPGAME_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponActor();

	
		

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Damage")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float CritDamageMultiplier;
	/*Shots per minute*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float RateOfFire; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
		float MagSize; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
		float WeaponRange;

	/*value 0.1 to 1.0*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim", meta = (ClampMin = 0.1f, ClampMax = 1.0f))
		float BulletSpreadAmount;

	UFUNCTION(BlueprintImplementableEvent)
		void OnFire();
	
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
		UAnimMontage* ReloadMontage;

	UFUNCTION(BlueprintCallable)
		void Reload();

	UFUNCTION(BlueprintCallable)
		float GetAmmoInMag() const { return AmmoInMag; }

	UFUNCTION(BlueprintCallable)
		float GetMagSize() const { return MagSize; }

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();
	
	UFUNCTION(NetMultiCast, Reliable, WithValidation)
		void MultiCastReload();
	

protected:


	AUnit* MyOwnerRef;

	bool bWantsToZoom;
	float DefaultFOV;
	float TimeBetweenShots;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
		USkeletalMeshComponent* WeaponMesh;

	UFUNCTION(BlueprintCallable)
		void SetupAttachment(FName SocketName);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
		float ZoomFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Aim", meta = (ClampMin = 0.1f, ClampMax = 100.0f))
		float ZoomInterpSpeed;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UCameraShake> FireCameraShake;

	// Scaled by the recoil force, applied every shot, Z is irrelevant
	UPROPERTY(EditDefaultsOnly, Category = "Aim")
		FRotator RecoilRotator;

	void ToggleIsReloading();

	void SpawnFireEffects(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	virtual void Fire();

	FTimerHandle TimerHandle_TimeBetweenShots;

	FTimerHandle TimerHandle_Reload;

	float LastFireTime;
	
	UPROPERTY(Replicated)
	float AmmoInMag;
	
	float CurrentBulletSpreadAmount;
	
	bool bIsFiring;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsReloading;

	float FirstDelay;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
		void OnRep_HitScanTrace();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void AltFire(bool WantsToZoom);

	void StartFire();

	void StopFire();
	
};
