// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AFPSProjectile::AFPSProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSProjectile::OnHit);	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	/*UE follows client-server architecture. The server tells the client that projectile actors need to be replicated.
	* Replication means that a copy of the projectile actor is spawned in the client. This copy will not have the same nature as the one on the server.
	* Therefore things such as movement must be replicated in the client.
	* We use replication as it ensures the clients are in sync with server.
	* We also need to ensure movement is replicated.
	Their movement */
	SetReplicates(true);
	SetReplicateMovement(true);
}


void AFPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	/* Only add impulseand destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}*/

	if (HasAuthority())  // (GetLocalRole() == ROLE_Authority)
	{
		/* AI code runs only on the server, so we need Make noise to run only in the server*/

		/*We need an instigator in make noise because MakeNoise checks if the Instigator actually has a noise emitter component & is capable of making noise.*/

		/* We can use the Instigator variable that's aldready in the AActor class althought, Instigator is usually used to know which actor does damage to the particular object
		Here however, we can use it in this function as we are not using it for anything else
		Also it can easily be set in the FPSCharacter class in the spawn params & we wont use an additional variable for it */
		MakeNoise(1.0f, AFPSProjectile::GetInstigator());

		/* The clients replicate the projectile so when the destroy function is called on server, the copies are destroyed too.
		So we should not have the destroy function in the clients.*/
		Destroy();
	}
	
}