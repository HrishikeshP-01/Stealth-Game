// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSObjectiveActor.generated.h"

// Forward Declaration
class USphereComponent;

UCLASS()
class FPSGAME_API AFPSObjectiveActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSObjectiveActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,Category="Components")
		UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Components")
		USphereComponent* SphereComp;

	void PlayEffect(); // don't want anyone else to access this fn
	UPROPERTY(EditDefaultsOnly, Category = "FX")
		UParticleSystem* EmitterFX;

public:	
	// This doesn't have to tick
	// virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
