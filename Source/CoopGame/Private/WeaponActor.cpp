// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Unit.h"
#include "DrawDebugHelpers.h"
#include "Classes/GameFramework/DamageType.h"
#include "Classes/Kismet/Gameplaystatics.h"
#include "Classes/Particles/ParticleSystem.h"
#include "Classes/Particles/ParticleSystemComponent.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "Classes/Animation/AnimMontage.h"
#include "Classes/Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"

// Sets default values

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("d.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw debug traces when firing"),
	ECVF_Cheat);



AWeaponActor::AWeaponActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	TracerTargetName = "Target";
	CritDamageMultiplier = 2.0f;
	BaseDamage = 20.0f;
	RateOfFire = 600.0f;
	WeaponRange = 2000.0f;
	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AWeaponActor::Reload()
{
	if (Role < ROLE_Authority)
	{
		ServerReload();
	}
	if (AmmoInMag != MagSize && bIsReloading == false)
	{ 
		UE_LOG(LogTemp, Warning, TEXT("Reloading Character's Name is %s"), *MyOwnerRef->GetName());
		USkeletalMeshComponent* Mesh = MyOwnerRef->GetMesh();
		if (Mesh)
		{ 
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
			if (AnimInstance && ReloadMontage)
			{ 
			AnimInstance->Montage_Play(ReloadMontage, 1.0f, EMontagePlayReturnType::MontageLength);
				if (Role == ROLE_Authority)
				{ 
				bIsReloading = true;
				UE_LOG(LogTemp, Warning, TEXT("Your message %s"), bIsReloading ? TEXT("True") : TEXT("False"));
				float MontageDuration = ReloadMontage->GetPlayLength();
				GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &AWeaponActor::ToggleIsReloading, MontageDuration, false);
				}
			}
		}
	}
}

void AWeaponActor::ServerReload_Implementation()
{
	Reload();
}

bool AWeaponActor::ServerReload_Validate()
{
	return true;
}

void AWeaponActor::MultiCastReload_Implementation()
{
	Reload();
}

bool AWeaponActor::MultiCastReload_Validate()
{
	return true;
}

// Called when the game starts or when spawned
void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	
	MyOwnerRef = Cast<AUnit>(GetOwner());
	if (MyOwnerRef)
	{
		DefaultFOV = MyOwnerRef->Camera->FieldOfView;
	}

	TimeBetweenShots = 60 / RateOfFire;
	AmmoInMag = MagSize;
}

void AWeaponActor::SetupAttachment(FName SocketName)
{
	if (SocketName != "None")
	{ 
	AUnit* Owner = Cast<AUnit>(GetOwner());
	AttachToComponent(Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	}
	else
	{
		AWeaponActor* Weapon = this;
		UE_LOG(LogTemp, Warning, TEXT("Attach socket name is empty for %s"));
	}

}



void AWeaponActor::ToggleIsReloading()
{
	if (bIsReloading == false)
	{
		bIsReloading = true;
	}
	else
	{ 
		bIsReloading = false;
		AmmoInMag = MagSize;
	}
	UE_LOG(LogTemp, Warning, TEXT("Your message %s"), bIsReloading ? TEXT("True") : TEXT("False"));
}

void AWeaponActor::SpawnFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, WeaponMesh, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* Controller = Cast<APlayerController>(Owner->GetController());
		if (Controller)
		{
			Controller->ClientPlayCameraShake(FireCameraShake);
		}
	}

}

void AWeaponActor::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}
	// Spawn impact particle

	if (SelectedEffect)
	{
		FVector MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void AWeaponActor::AltFire(bool WantsToZoom)
{
	bWantsToZoom = WantsToZoom;
}

void AWeaponActor::StartFire()
{
	if (bIsReloading == false && AmmoInMag > 0)
	{ 
	FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AWeaponActor::Fire, TimeBetweenShots, true, FirstDelay);
	bIsFiring = true;
	}
}

void AWeaponActor::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	bIsFiring = false;
}

// Shot calculations
void AWeaponActor::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}
	if (AmmoInMag > 0)
	{
		//Query params
		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		//Trace variables declaration
		FRotator RandomAimRot = FRotator(RecoilRotator + RecoilRotator) * CurrentBulletSpreadAmount; // Bullet spread calculation
		FVector CameraLocation;
		FRotator ShotDirection;
		FVector TraceEnd;
		FVector TracerEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
	
		//Trace variables definition
		if (MyOwnerRef)
		{ 
		ShotDirection =MyOwnerRef->Camera->GetComponentRotation() + RandomAimRot;
		CameraLocation = MyOwnerRef->Camera->GetComponentLocation();
		TraceEnd = MyOwnerRef->Camera->GetComponentLocation() + (ShotDirection.Vector() * WeaponRange);
		TracerEnd = TraceEnd;
		UE_LOG(LogTemp, Warning, TEXT("Your message %s"), *RandomAimRot.ToString());
		}


		if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{


			// Determine hit surface and impact particle

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			
			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			// Apply damage
		
			float CalculatedDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				CalculatedDamage = BaseDamage * CritDamageMultiplier;
			}

			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				UGameplayStatics::ApplyPointDamage(HitActor, CalculatedDamage, ShotDirection.Vector(), Hit, MyOwnerRef->GetInstigatorController(), MyOwnerRef, DamageType);
			}

			// If hit location isn't null, set tracer end to hit location
			if (Hit.Location != FVector(0, 0, 0))
			{
				TracerEnd = Hit.Location;
			}

		
			// Debug tracing
			if (DebugWeaponDrawing > 0)
				{
					DrawDebugLine(GetWorld(), CameraLocation, Hit.TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
				}
		}
		LastFireTime = GetWorld()->TimeSeconds;
		OnFire();
		AmmoInMag--;
		// Spawn tracer and other effects
		SpawnFireEffects(TracerEnd);
		
		if (CurrentBulletSpreadAmount <= 1.0f)
		{
			CurrentBulletSpreadAmount += BulletSpreadAmount;
			CurrentBulletSpreadAmount = FMath::Min(CurrentBulletSpreadAmount, 1.0f);

		}
		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEnd;
			HitScanTrace.SurfaceType = SurfaceType;
		}
	}
	else
		Reload();
	
}

void AWeaponActor::OnRep_HitScanTrace()
{
	SpawnFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

// Called every frame
void AWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MyOwnerRef)
	{ 
		float TargetFOV = bWantsToZoom ? ZoomFOV : DefaultFOV;
		float NewFOV = FMath::FInterpTo(MyOwnerRef->Camera->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

		MyOwnerRef->Camera->SetFieldOfView(NewFOV);
	}

	if (CurrentBulletSpreadAmount > 0 && bIsFiring == false)
	{
		CurrentBulletSpreadAmount -= DeltaTime * 1;
		if (CurrentBulletSpreadAmount < 0)
		{ 
			CurrentBulletSpreadAmount= 0;
		}
	}
}


void AWeaponActor::ServerFire_Implementation()
{
	Fire();
}

bool AWeaponActor::ServerFire_Validate()
{
	return true;
}

void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeaponActor, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME (AWeaponActor, bIsReloading);
	DOREPLIFETIME(AWeaponActor, AmmoInMag);
}