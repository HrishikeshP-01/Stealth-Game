// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSObjectiveActor.h"
/* We only want to include stuff like sphere component, gameplay statics etc.
in the cpp file as they aren't used in the header file & it reduces compliation time 
The references to sphere component in header file can be solved with forward declaration*/
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "FPSCharacter.h"

// Sets default values
AFPSObjectiveActor::AFPSObjectiveActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(MeshComp);
	// We don't want physics collision only stuff like line traces & overlaps etc
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

// Called when the game starts or when spawned
void AFPSObjectiveActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFPSObjectiveActor::PlayEffect() 
{
	UGameplayStatics::SpawnEmitterAtLocation(this, EmitterFX, GetActorLocation());
}

void AFPSObjectiveActor::NotifyActorBeginOverlap(AActor* OtherActor) 
{
	Super::NotifyActorBeginOverlap(OtherActor);

	PlayEffect();
	AFPSCharacter* Character = Cast<AFPSCharacter>(OtherActor);
	if (Character)
	{
		Character->bIsCarryingObjective = true;
		Destroy();
	}
}