// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponActor.h"
#include "WeaponActorProjectileBase.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class COOPGAME_API AWeaponActorProjectileBase : public AWeaponActor
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf <AProjectile> ProjectileClass;


public:
	virtual void Fire() override;
	
};
