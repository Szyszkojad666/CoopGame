// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActorProjectileBase.h"
#include "Projectile.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Classes/GameFramework/Controller.h"
#include "Classes/GameFramework/ProjectileMovementComponent.h"
#include "Unit.h"
#include "Classes/Camera/CameraComponent.h"

void AWeaponActorProjectileBase::Fire()
{
	if (ProjectileClass)
	{
		AActor* Owner = GetOwner();
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		if (MuzzleLocation != FVector (0, 0, 0) && Owner)
		{
			FRotator EyeRotation;
			FVector EyeLocation;
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
			GetWorld()->SpawnActor<AProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
		}
	}
	else 
		UE_LOG(LogTemp, Warning, TEXT("Please set the projectile class for the weapon"));
}
