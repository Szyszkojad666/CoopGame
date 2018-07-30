// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Classes/GameFramework/Actor.h"
#include "Unit.h"
#include "CoopGameMode.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	MaxHealth = 100.0f;
	SetComponentTickEnabled(false);
	SetIsReplicated(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	AActor* Owner = GetOwner();
	
	// Only bind take damage if owner is the server
	if (Owner && GetOwnerRole() == ROLE_Authority)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
		UnitOwner = Cast<AUnit>(Owner);
	}

	// ...

}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// Check if damage was higher than 0
	if (Damage <= 0)
	{
		return;
	}

	// Check if actor was friendly
	if (DamagedActor != DamageCauser && IsFriendly(UnitOwner, Cast<AUnit>(DamageCauser)))
	{
		return;
	}

	// Deal damage
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	//UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));

	// Broadcast health changed
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (Health <= 0 && !bIsDead)
	{
		ACoopGameMode* GM = Cast<ACoopGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
			bIsDead = true;
		}
	}
	
}

void UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0 || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s + (%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

}

bool UHealthComponent::IsFriendly(AUnit * UnitA, AUnit * UnitB)
{
	if (!UnitA || !UnitB)
	{
		return false;
	}

	if (UnitA->GetTeamID() == UnitB->GetTeamID())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
}