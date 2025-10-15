

#include "Controller/SPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Input/SInputComponent.h" // 커스텀 입력 컴포넌트
#include "SGameplayTags.h" // GameplayTag를 사용하기 위해

ASPlayerController::ASPlayerController()
{
}

void ASPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputConfig)
		{
			Subsystem->AddMappingContext(InputConfig->InputMappingContext, 0);
		}
	}}

void ASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	MyInputComponent = CastChecked<USInputComponent>(InputComponent);

	// 바인딩 로직: 컨트롤러의 콜백 함수와 연결합니다.
	MyInputComponent->BindInputAction(InputConfig, FSGameplayTags::Get().InputTag_Move, ETriggerEvent::Triggered, this, &ASPlayerController::Move_Input);
	MyInputComponent->BindInputAction(InputConfig, FSGameplayTags::Get().InputTag_Look, ETriggerEvent::Triggered, this, &ASPlayerController::Look_Input);

}

void ASPlayerController::Move_Input(const FInputActionValue& InputAction)
{
	const FVector2D InputAxisVector = InputAction.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void ASPlayerController::Look_Input(const FInputActionValue& InputAction)
{
	const FVector2D LookAxisVector = InputAction.Get<FVector2D>();
	
	if (LookAxisVector.X != 0.f)
	{
		AddYawInput(LookAxisVector.X);
	}

	if (LookAxisVector.Y != 0.f)
	{
		AddPitchInput(LookAxisVector.Y);
	}
}
