// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSCharacter.h"
#include "FPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PawnNoiseEmitterComponent.h"


AFPSCharacter::AFPSCharacter()
{
	// Create a CameraComponent	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0, 0, BaseEyeHeight)); // Position the camera
	CameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1PComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	Mesh1PComponent->SetupAttachment(CameraComponent);
	Mesh1PComponent->CastShadow = false;
	Mesh1PComponent->SetRelativeRotation(FRotator(2.0f, -15.0f, 5.0f));
	Mesh1PComponent->SetRelativeLocation(FVector(0, 0, -160.0f));

	// Create a gun mesh component
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	GunMeshComponent->CastShadow = false;
	GunMeshComponent->SetupAttachment(Mesh1PComponent, "GripPoint");

	NoiseEmittingComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmittingComp"));
}


void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AFPSCharacter::Fire()
{
	/*We make the server spawn the projectile & replicate it in clients. However other things unique to clients such as sound, animation etc are called on each client*/
	ServerFire();

	// try and play the sound if specified
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1PComponent->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "Arms", 0.0f);
		}
	}
}

// We don't implement server functions normally, we have _Implementation() & so on..
void AFPSCharacter::ServerFire_Implementation()
{
	/* We can't direct the server to replicate a projectile. Instead we let the server spawn projectiles.
	 * Moreover, the server aldready replicates projectiles so letting the client do so will be redundant & will create duplicate copies.
	 * So we only let the server fire projectiles.*/
	// try and fire a projectile
	if (ProjectileClass)
	{
		FVector MuzzleLocation = GunMeshComponent->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = GunMeshComponent->GetSocketRotation("Muzzle");

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		// We set instigator to the character so that it can be used in the make noise fn
		ActorSpawnParams.Instigator = this;

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<AFPSProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
	}
}

bool AFPSCharacter::ServerFire_Validate()
{
	/*This function is used on server side for sanity checks & lets us perform checks & detect cheating etc.
	* For now we assume the validation is true & the function is always executed properly */
	return true;
}

void AFPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}


void AFPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*Networking the pitch
	* You will find that without networking the pitch of the hand upward & downward movements of the other players are not replicated
	* This leads to the projectile being spawned at the wrong locations as the fire function uses the location & orientation of the copy of the player on the server
	* As the orientation is not networked the default orientation is taken & therefore the spawing of projectile is wrong.
	* Also the position of the hands of the other players will not be the same in any of the machines as they are not networked.
	
	* We use this method where we update the pitch of all the players that are not being controlled by the current player.
	* Say I am a player on my machine the character that I control must be tweaked by the input that I give. 
	* The other players' characters are actually copies on my machine and players on theirs.
	* My machine's character pitch is being manipulated using the input component.
	* To simulate the pitch of the other players' characters, we change the pitch of the local copies of the player on our machine
	* To do this we use the in-built RemoteViewPitch which stores the pitch of the other clients*/
	if (!IsLocallyControlled()) // Do this for player characters not controlled by us
	{
		FRotator NewRotation = CameraComponent->GetRelativeRotation(); 
		/* We didn't use something more direct such as the mesh component because the origin of the mesh component was at a different postion 
		& applying the changes there caused huge variation in rotation. I THINK RemoteViewPitch works best when the origin of the compoent is close to camera or is camera itself */
		NewRotation.Pitch = RemoteViewPitch * 360.0f / 255.0f;
		/* RemoteViewPitch is a uint8. It can't store any negative values & the result isn't between 0-360 degrees.
		This will case discrepencies in the orientation of the copies when their player has an orientation exeeding the uint8's limit.
		To avoid this first we have to convert the RemoteViewPitch data to an angle that's between 0-360 degrees.
		
		We look at the defintion of RemoteViewPitch in engine.
		RemoteViewPitch = (uint8)(NewRemotePitch * 255.0f/360.0f)
		So we get the NewRemotePitch value as that will be between 0-360*/
		CameraComponent->SetRelativeRotation(NewRotation);
	}
}