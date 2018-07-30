// Fill out your copyright notice in the Description page of Project Settings.

#include "Destructible_Base.h"
#include "Classes/Components/AudioComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Particles/ParticleSystem.h"
#include "Classes/Components/CapsuleComponent.h"
#include "HealthComponent.h"
#include "Classes/PhysicsEngine/RadialForceComponent.h"
#include "CoopGame.h"
#include "Classes/Kismet/Gameplaystatics.h"
#include "Net/UnrealNetwork.h"
#include "Classes/Materials/MaterialInterface.h"

// Sets default values
ADestructible_Base::ADestructible_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	FX = CreateDefaultSubobject<UParticleSystem>(TEXT("FX"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));

	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));

	CapsuleCollision->SetCollisionObjectType(ECollisionChannel::ECC_Destructible);

	CapsuleCollision->bGenerateOverlapEvents = true;

	CapsuleCollision->SetCollisionResponseToChannel(ECollisionChannel::COLLISION_WEAPON, ECR_Block);

	CapsuleCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));

	RootComponent = CapsuleCollision;

	MeshComp->SetupAttachment(CapsuleCollision);

	RadialForceComponent->SetupAttachment(CapsuleCollision);

	AudioComp->SetupAttachment(CapsuleCollision);
	
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bReplicateMovement = true;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ADestructible_Base::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnHealthChanged.AddDynamic(this, &ADestructible_Base::OnHealthChanged);
}
void ADestructible_Base::OnHealthChanged(UHealthComponent * HealthComp, float Health, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0 && bDestroyed == false)
	{
	// Dealing Damage goes through the server so there is no need for client-server RPC
			MultiCastDestroy();
	}
}

void ADestructible_Base::Destroy()
{
	bDestroyed = true;
	this->SetLifeSpan(3.0f);
	CapsuleCollision->SetCollisionResponseToChannel(ECollisionChannel::COLLISION_WEAPON, ECR_Ignore);
	CapsuleCollision->SetSimulatePhysics(true);
	CapsuleCollision->SetEnableGravity(true);
	RadialForceComponent->FireImpulse();
	MeshComp->SetMaterial(0, DestroyedMaterial);
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FX, GetActorTransform());
	OnDestroyedImplementation();
}


void ADestructible_Base::MultiCastDestroy_Implementation()
{
	Destroy();
}


// Called every frame


void ADestructible_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADestructible_Base, bDestroyed);
}