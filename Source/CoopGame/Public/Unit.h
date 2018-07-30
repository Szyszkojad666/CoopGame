// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Unit.generated.h"


class USpringArmComponent;
class UCameraComponent;
class WeaponActor;
class UHealthComponent;

UCLASS()
class COOPGAME_API AUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrouch();
	void ExecuteJump();
	void Reload();
	


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly)
		UCameraComponent* Camera;


	UPROPERTY(EditDefaultsOnly)
		USpringArmComponent* SpringArm;

	UPROPERTY(EditDefaultsOnly, Category = "Unit")
		TArray <TSubclassOf<AWeaponActor>> WeaponsArray;

	UPROPERTY(EditDefaultsOnly, Category = "Unit")
		TSubclassOf<AWeaponActor> DefaultWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
		FName PerceptionAttachBoneName;

	UPROPERTY(Replicated, BlueprintReadOnly)
		bool bIsDead;

	virtual FVector GetPawnViewLocation() const override;

	UPROPERTY(Replicated, BlueprintReadOnly)
		AWeaponActor* CurrentWeapon;

	UFUNCTION(BlueprintCallable)
		void EquipWeapon(TSubclassOf<AWeaponActor> WeaponToEquip);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerEquipWeapon(TSubclassOf<AWeaponActor> WeaponToEquip);

	UFUNCTION(BlueprintCallable)
		FORCEINLINE UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintCallable, Category = "Unit")
		FORCEINLINE uint8 GetTeamID() const { return TeamID; }

	UFUNCTION(BlueprintCallable, Category = "Unit")
		bool IsAliveAndWell();

	FORCEINLINE UFUNCTION(BlueprintCallable, Category = "Unit")
		 void SetNewTeamID(uint8 InNewTeamID) { TeamID = InNewTeamID; }

	UFUNCTION(BlueprintCallable)
		void Die();

	UFUNCTION()
		void OnHealthChanged(UHealthComponent* HealthComp, float Health, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	void GetPerceptionBoneAttachPoint(FVector &Location, FRotator &Rotation) const;

	void AltFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void StopFire();

private:
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Unit")
		uint8 TeamID;
};
