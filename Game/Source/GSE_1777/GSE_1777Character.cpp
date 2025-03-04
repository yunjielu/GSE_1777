// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GSE_1777Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialNetDriver.h"

#include "UnrealNetwork.h"

#include "DummyCharacter.h"

//////////////////////////////////////////////////////////////////////////
// AGSE_1777Character

DEFINE_LOG_CATEGORY(LogGSE_1777Character);

AGSE_1777Character::AGSE_1777Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

												// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AGSE_1777Character::BeginPlay()
{
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGSE_1777Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGSE_1777Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGSE_1777Character::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGSE_1777Character::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGSE_1777Character::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AGSE_1777Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AGSE_1777Character::TouchStopped);

	PlayerInputComponent->BindAction("1", IE_Pressed, this, &AGSE_1777Character::ServerSpawnDummyCharacter);
	PlayerInputComponent->BindAction("2", IE_Pressed, this, &AGSE_1777Character::ServerIncreaseVariables);
}

void AGSE_1777Character::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AGSE_1777Character::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AGSE_1777Character::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGSE_1777Character::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGSE_1777Character::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AGSE_1777Character::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AGSE_1777Character::ServerIncreaseVariables_Implementation()
{
	UE_LOG(LogGSE_1777Character, Warning, TEXT("%s"), *FString(__FUNCTION__));

	for (TObjectIterator<ADummyCharacter> It; It; ++It)
	{
		ADummyCharacter* Node = *It;
		Node->ReplicateInt++;
		Node->ReplicateIntInitialOnly++;
	}
}

void AGSE_1777Character::ServerSpawnDummyCharacter_Implementation()
{
	UE_LOG(LogGSE_1777Character, Warning, TEXT("%s"), *FString(__FUNCTION__));

	FVector location = GetActorLocation();
	location.Z += 100;
	GetWorld()->SpawnActor<ADummyCharacter>(ADummyCharacter::StaticClass(), location, FRotator::ZeroRotator);

	int Count = 0;
	for (TObjectIterator<ADummyCharacter> It; It; ++It)
	{
		Count++;
	}

	UE_LOG(LogGSE_1777Character, Warning, TEXT("%s - TotalCount:[%d]"), *FString(__FUNCTION__), Count);
}

void AGSE_1777Character::ClientSendInitializeData_Implementation(const TArray<uint8>& data)
{
	UE_LOG(LogGSE_1777Character, Warning, TEXT("%s"), *FString(__FUNCTION__));

	TMap<int64, ADummyCharacter*> EntityCharMap;
	for (TObjectIterator<ADummyCharacter> It; It; ++It)
	{
		ADummyCharacter* DummyChar = *It;
		int64 EntityId = USpatialStatics::GetActorEntityId(DummyChar);

		EntityCharMap.Add(EntityId, DummyChar);
	}

	FBufferArchive Ar;
	Ar.Append(data.GetData(), data.Num());

	int16 count = 0;
	Ar << count;

	for (int i = 0; i < count; ++i)
	{
		int64 EntityId = -1;
		Ar << EntityId;

		auto It = EntityCharMap.Find(EntityId);
		if (It != nullptr)
		{
			ADummyCharacter* DummyChar = *It;
			DummyChar->DeserializeInitialOnlyData(Ar);
		}
	}
}

void AGSE_1777Character::OnAuthorityGained()
{
	Super::OnAuthorityGained();

	TArray<uint8> InitializingData;
	int16 count = 0;

	for (TObjectIterator<ADummyCharacter> It; It; ++It)
	{
		FBufferArchive Ar;
		ADummyCharacter* DummyChar = *It;
		int64 EntityId = USpatialStatics::GetActorEntityId(DummyChar);

		Ar << EntityId;
		DummyChar->SerializeInitialOnlyData(Ar);
		count++;

		TArray<uint8>& TmpArray = *static_cast<TArray<uint8>*>(&Ar);
		InitializingData.Append(TmpArray.GetData(), TmpArray.Num());
	}

	ClientSendInitializeData(InitializingData);
}

