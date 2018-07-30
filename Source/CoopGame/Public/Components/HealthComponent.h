// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// On Health Changed event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

class AUnit;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
		float MaxHealth;

	UPROPERTY(Replicated, BlueprintReadOnly)
		float Health;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	bool bIsDead;
public:	
	
	
	UFUNCTION(BlueprintCallable)
		FORCEINLINE  float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable)
		void Heal(float HealAmount);
		
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnHealthChangedSignature OnHealthChanged;
	
	UFUNCTION(BlueprintCallable, Category = "Unit")
		static bool IsFriendly(AUnit * UnitA, AUnit * UnitB);

private:
	AUnit* UnitOwner;
};
