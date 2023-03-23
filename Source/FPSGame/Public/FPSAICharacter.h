// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSAICharacter.generated.h"

class UPawnSensingComponent;

// We give BlueprintType to it so that we can use it in BP. It is because it's BlueprintType that we use uint8 else we could've used anything, even could have just used enum AICharState{}
UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle,
	Suspicious,
	Alerted
};

UCLASS()
class FPSGAME_API AFPSAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSAICharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UPawnSensingComponent* PawnSensingComp;

	// Has to have UFunction() as it's the only way UE knows it's this function that is to binded with pawn sensing's on see pawn add dynamic
	UFUNCTION()
		void OnSeenPawn(APawn* SeenPawn);
	// const in function declaration means that the value of that parameter can't be changed in the function
	/* Here it's used with FVector& Location. This is useful because the Location is a reference so any change in it's value in the function
	* will impact the value which is going to be used elsewhere. We wont pass it by value as creating many copies is suboptimal 
	* So we use const ot prevent the user from changing the value. */
	/* In the defenition of the delegate OnHearNoise, APawn* Instigator is used 
	* However, AActor aldready has an Instigator variable so the compiler wont know which variable we are refering to inside the function
	* this results in an error so we change the name to NoiseInstigator here */
	UFUNCTION()
		void OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume);

	/*Note on AIPerception-
	Sight is given precedense over sound. When the character sees you it will no longer hear you.
	This can be verified by looking at the debug spheres. Once it sees the player the sight debug spheres will be drawn but the sound debug spheres wont.
	Once the character can no longer see the player but the noise made by the player is heard, sound debug spheres will be drawn.

	Character can't here 2 noises made by the same instigator at the same time. LOOK INTO THIS. NOT SURE
	*/

	UFUNCTION()
		void ResetOrientation();
	/* We make the timer handle a global variable instead of a local variable.
	* This is because if the OnNoiseHeard fn is triggered repeatedly if the handle was a local variable, multiple timers would be set
	* This would result in the reset orientation fn being called the moment each timer goes off. 
	* The character would go back to initial rotation regardless of when the rest of the timers go off.
	* Global timer ensures that it is this timer that gets reset on repeated calls of the same fn.*/
	FTimerHandle TimerHandle_ResetOrientation;
	FRotator OriginalRotation; // No need to expose it to BP as we wont be using it outside of this

	/* The AI parts of the code only gets run on server. So when the OnSee, OnHear parts run & the GuardState is updated,
	* the client machine's copy of GuardState isn't updated. So we need to replicate GuardState.
	* ReplicatedUsing=OnRep_GuardState means that every time the GuardState is updated, OnRep_GuardState fn runs in each client.
	* The fn is automatically run on client when GuardState is replicated. However, if you want the same fn to run on the server you can simply call it.
	* It is just a function. The ReplicatedUsing is what makes it run on the clients automatically. OnRep_GuardState is just another fn we create*/
	UPROPERTY(ReplicatedUsing=OnRep_GuardState)
		EAIState GuardState;

	UFUNCTION()
		void OnRep_GuardState();

	void ChangeGuardState(EAIState NewState);
	UFUNCTION(BlueprintImplementableEvent)
		void OnGuardStateChanged(EAIState NewState);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Don't need this as it will never take input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
