// Fill out your copyright notice in the Description page of Project Settings.

#include "Unit.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "WeaponActor.h"
#include "Classes/Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "HealthComponent.h"
#include "CoopGame.h"
#include "Net/UnrealNetwork.h"
#include "../Public/Unit.h"
#include "Classes/Components/SkeletalMeshComponent.h"



// Sets default values
AUnit::AUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation;
	Camera->SetupAttachment(SpringArm);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	PerceptionAttachBoneName = "Head";

	TeamID = 255;
	
}


// Called when the game starts or when spawned
void AUnit::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddDynamic(this, &AUnit::OnHealthChanged);
}

void AUnit::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}


void AUnit::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void AUnit::BeginCrouch()
{
	Crouch();
}

void AUnit::EndCrouch()
{
	UnCrouch();
}

void AUnit::ExecuteJump()
{
	Jump();
}

void AUnit::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void AUnit::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void AUnit::Reload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

// Called every frame
void AUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void AUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUnit::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUnit::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AUnit::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &AUnit::AddControllerYawInput);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AUnit::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AUnit::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AUnit::ExecuteJump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AUnit::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AUnit::StopFire);

	PlayerInputComponent->BindAction("FireAlt", IE_Pressed, this, &AUnit::AltFire);
	PlayerInputComponent->BindAction("FireAlt", IE_Released, this, &AUnit::AltFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AUnit::Reload);
}

FVector AUnit::GetPawnViewLocation() const
{
	if (Camera)
	{
		return Camera->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void AUnit::EquipWeapon(TSubclassOf<AWeaponActor> WeaponToEquip)
{
	if (Role < ROLE_Authority)
	{
		ServerEquipWeapon(WeaponToEquip);
		return;
	}
		if (WeaponToEquip)
		{
			if (CurrentWeapon && CurrentWeapon->GetClass() == WeaponToEquip)
			{
				return;
			}
			else if(CurrentWeapon)
			{
				CurrentWeapon->Destroy();
			}
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			UE_LOG(LogTemp, Warning, TEXT("Weapon owner is %s"), *SpawnParams.Owner->GetName());
			SpawnParams.Instigator = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			CurrentWeapon = GetWorld()->SpawnActor<AWeaponActor>(WeaponToEquip, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}

	}

void AUnit::ServerEquipWeapon_Implementation(TSubclassOf<AWeaponActor> WeaponToEquip)
{
	EquipWeapon(WeaponToEquip);
}

bool AUnit::ServerEquipWeapon_Validate(TSubclassOf<AWeaponActor> WeaponToEquip)
{
	return true;
}


bool AUnit::IsAliveAndWell()
{
	if (HealthComponent->GetHealth() > 0)
	{
		return true;
	}
	return false;
}

void AUnit::Die()
{
	if (bIsDead == false)
	{ 
	bIsDead = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMovementComponent()->StopMovementImmediately();
	DetachFromControllerPendingDestroy();
	SetLifeSpan(10.0f);
	}
}


void AUnit::OnHealthChanged(UHealthComponent * HealthComp, float Health, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0)
	{ 
		Die();
	}
}

void AUnit::GetActorEyesViewPoint(FVector & OutLocation, FRotator & OutRotation) const
{
	GetPerceptionBoneAttachPoint(OutLocation, OutRotation);
}

void AUnit::GetPerceptionBoneAttachPoint(FVector &Location, FRotator &Rotation) const
{
	if (GetMesh())
	{
		USkeletalMeshComponent* Mesh = GetMesh();
		Location = Mesh->GetBodyInstance(PerceptionAttachBoneName)->GetUnrealWorldTransform().GetLocation();
		Rotation = Mesh->GetBodyInstance(PerceptionAttachBoneName)->GetUnrealWorldTransform().Rotator();
	}
}

void AUnit::AltFire()
{
	if (CurrentWeapon)
	{
		static bool bAltFire;
		if (bAltFire)
		{
			bAltFire = false;
		}
		else
			bAltFire = true;
		CurrentWeapon->AltFire(bAltFire);
	}
}

void AUnit :: GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnit, CurrentWeapon);
	DOREPLIFETIME(AUnit, bIsDead);
}