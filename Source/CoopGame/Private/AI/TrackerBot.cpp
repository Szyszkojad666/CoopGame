// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackerBot.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Kismet/Gameplaystatics.h"
#include "Classes/GameFramework/Character.h"
#include "Classes/AI/Navigation/NavigationSystem.h"
#include "Classes/AI/Navigation/NavigationPath.h"
#include "HealthComponent.h"
#include "Classes/Components/AudioComponent.h"
#include "Classes/Particles/ParticleSystem.h"
#include "Classes/Components/SphereComponent.h"
#include "Classes/PhysicsEngine/RadialForceComponent.h"
#include "CoopGame.h"
#include "Net/UnrealNetwork.h"
#include "Classes/Materials/MaterialInterface.h"
#include "Unit.h"
#include "Classes/Sound/SoundCue.h"


// Sets default values
ATrackerBot::ATrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	MeshComp->SetSimulatePhysics(true);
	bCanAffectNavigationGeneration = false;
	MovementForce = 1000.0f;
	RequiredDistanceToPathPoint = 100.0f;
	bUseVelocityChange = true;
	ExplosionDamage = 100.0f;
	ExplosionRadius = 200.0f;

	FX = CreateDefaultSubobject<UParticleSystem>(TEXT("FX"));

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));

	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);

	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SphereCollision->SetupAttachment(MeshComp);

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));

	bReplicateMovement = true;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ATrackerBot::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnHealthChanged.AddDynamic(this, &ATrackerBot::OnHealthChanged);
	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_UpdatePowerLevel, this, &ATrackerBot::UpdatePowerLevel, 1.0f, true, 1.0f);
	}
	
}

FVector ATrackerBot::GetNextPathPoint()
{
	
	ACharacter* ActorToPathTo = UGameplayStatics::GetPlayerCharacter(this, 0);
	
	if(ActorToPathTo)
	{ 
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), ActorToPathTo);
		if(NavPath)
		{
			if (NavPath && NavPath->PathPoints.Num() > 1)
			{
				return NavPath->PathPoints[1];
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Nav Mesh is missing! Please add a navmesh to the level"));
	return GetActorLocation();
}

void ATrackerBot::InnitiateSelfDestruct()
{
	bSelfDestructionSequence = true;
	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().SetTimer(TimerHande_DamageSelf, this, &ATrackerBot::DamageSelf, 0.5f, true, 0.0f);
	}
	UGameplayStatics::PlaySoundAtLocation(this, SelfDestructSound, GetActorLocation());
}

void ATrackerBot::MultiCastFlashMaterial_Implementation()
{
	FlashMaterial();
}


void ATrackerBot::FlashMaterial()
{
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	else
	{
		MatInst->SetScalarParameterValue("LastTimeDamaged", GetWorld()->TimeSeconds);
	}
}

void ATrackerBot::MultiCastInnitiateSelfDestruct_Implementation()
{
	InnitiateSelfDestruct();
}

void ATrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20.0f, GetController(), this, nullptr);
}

void ATrackerBot::UpdatePowerLevel()
{
	
	const float Radius = 500.0f;

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(Radius);
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollisionShape);

	int32 NumberOfBots = 0;

	for (FOverlapResult Result : Overlaps)
	{
		ATrackerBot* Bot = Cast<ATrackerBot>(Result.GetActor());
		if (Bot && Bot != this)
		{
			NumberOfBots++;
		}
	}
	
	
	PowerLevel = FMath::Clamp(NumberOfBots, 0, MaxPowerLevel);
	
	if (Role == ROLE_Authority)
	{
		MultiCastUpdateMaterialParam();
	}	
	
	
	
}

void ATrackerBot::UpdateMaterialParam()
{
	
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	else 
	{
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
		
	}
}

void ATrackerBot::MultiCastUpdateMaterialParam_Implementation()
{
	UpdateMaterialParam();
}

void ATrackerBot::OnHealthChanged(UHealthComponent * HealthComp, float Health, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0 && bDestroyed == false)
	{
		// Dealing Damage goes through the server so there is no need for client-server RPC
		MultiCastDestroy();
	}

	MultiCastFlashMaterial();

}

void ATrackerBot::MoveToPathPoint()
{
	if ((GetActorLocation() - NextPathPoint).Size() <= RequiredDistanceToPathPoint)
	{
		NextPathPoint = GetNextPathPoint();
	}

	else
	{
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();
		ForceDirection *= MovementForce;
		MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
	}
}


void ATrackerBot::Destroy()
{
	if (bDestroyed == true)
	{
		return;
	}
	
	bDestroyed = true;
	this->SetLifeSpan(3.0f);
	/*
	SphereCollision->SetCollisionResponseToChannel(ECollisionChannel::COLLISION_WEAPON, ECR_Ignore);
	SphereCollision->SetSimulatePhysics(true);
	SphereCollision->SetEnableGravity(true);
	*/
	
	MeshComp->SetMaterial(0, DestroyedMaterial);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, GetActorTransform());
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
	
	if (Role == ROLE_Authority)
	{
		RadialForceComponent->FireImpulse();
		TArray<AActor*>IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, GetInstigatorController());
		OnDestroyedImplementation();
	}
	
}

void ATrackerBot::MultiCastDestroy_Implementation()
{
	Destroy();
}

// Called every frame
void ATrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(bDestroyed == false)
		
		if (Role == ROLE_Authority)
		{
			MoveToPathPoint();
		}
	
	
}

// Called to bind functionality to input
void ATrackerBot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATrackerBot::NotifyActorBeginOverlap(AActor * OtherActor)
{
	if (Role == ROLE_Authority && OtherActor)
	{
		if (!bSelfDestructionSequence)
		{
			AUnit* OtherUnit = Cast<AUnit>(OtherActor);
			if (OtherUnit && OtherUnit->IsPlayerControlled())
			{
				MultiCastInnitiateSelfDestruct();
			}

		}
	}
}

void ATrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrackerBot, bDestroyed);
}