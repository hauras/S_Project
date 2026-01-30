

#include "Controller/SPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Input/SInputComponent.h" 
#include "SGameplayTags.h" 
#include "Ability/SAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Ability/SAttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "Character/PlayerCharacter.h"
#include "State/SPlayerState.h"
#include "UI/HUD/SHUD.h"
#include "UI/Widget/DamageTextComponent.h"
#include "Data/PlayerDataAsset.h"    
#include "GameFramework/CharacterMovementComponent.h"
#include "Instance/SGameInstance.h"
#include "Actor/Component/InventoryComponent.h"
ASPlayerController::ASPlayerController()
{
	PlayerTeamId = FGenericTeamId(0);
}

FGenericTeamId ASPlayerController::GetGenericTeamId() const
{
	return PlayerTeamId;
}

void ASPlayerController::SetCrosshairVisibility(bool bVisible)
{
	if (CrosshairWidgetClassInstance)
	{
		ESlateVisibility NewVisibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		CrosshairWidgetClassInstance->SetVisibility(NewVisibility);
	}
}

void ASPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bIsSynergy)
{
	if (IsValid(TargetCharacter) && DamageTextClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextClass);
		DamageText->RegisterComponent();
		
		FVector Center = TargetCharacter->GetActorLocation();

		// 1. [디테일] 시너지 데미지라면 일반 데미지보다 살짝 더 위에서 나오게 설정 (가독성 업)
		float MinZ = bIsSynergy ? 100.f : 60.f;
		float MaxZ = bIsSynergy ? 130.f : 90.f;

		FVector RandomOffset = FVector(
			FMath::RandRange(-35.f, 35.f), 
			FMath::RandRange(-35.f, 35.f), 
			FMath::RandRange(MinZ, MaxZ)
		);

		DamageText->SetWorldLocation(Center + RandomOffset);

		// 2. [핵심] 시너지 여부에 따라 색상 지정
		// SetDamageColor는 UDamageTextComponent에 정의되어 있어야 합니다 (아래 참고)
		if (bIsSynergy)
		{
			// 시너지: 보라색 (사이킥+얼음 느낌)
			DamageText->SetDamageColor(FLinearColor(0.6f, 0.1f, 1.0f)); 
		}
		else
		{
			// 일반: 하얀색
			DamageText->SetDamageColor(FLinearColor::White);
		}

		DamageText->SetDamageText(DamageAmount);
	}
}
void ASPlayerController::Server_RequestCharacterSwap_Implementation(int32 NewIndex)
{
	ASPlayerState* PS = GetPlayerState<ASPlayerState>();
	APlayerCharacter* OldCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!PS || !OldCharacter || !PS->PlayerData.IsValidIndex(NewIndex)) return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(OldCharacter->CharacterTag);
		ASC->ClearAllAbilities(); 
	}

	// 3. 위치/속도 백업
	FTransform SpawnTransform = OldCharacter->GetActorTransform();
	FVector OldVelocity = OldCharacter->GetVelocity();
	
	// 5. 교체 진행
	UnPossess();
	OldCharacter->Destroy();

	UClass* NewClass = PS->PlayerData[NewIndex]->CharacterInfo.CharacterClass;
	APlayerCharacter* NewCharacter = GetWorld()->SpawnActor<APlayerCharacter>(NewClass, SpawnTransform);

	if (NewCharacter)
	{
		NewCharacter->CharacterTag = PS->PlayerData[NewIndex]->CharacterInfo.CharacterTag;
		PS->CurrentCharacterIndex = NewIndex;
		PS->CurrentCharacterTag = NewCharacter->CharacterTag;

		Possess(NewCharacter);


		if (NewCharacter->GetCharacterMovement())
		{
			NewCharacter->GetCharacterMovement()->Velocity = OldVelocity;
		}
	}
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
	}

	if (IsLocalController() && CrosshairWidgetClass)
	{
		CrosshairWidgetClassInstance = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
		if (CrosshairWidgetClassInstance)
		{
			CrosshairWidgetClassInstance->AddToViewport();
			
		}
	}
}

void ASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	MyInputComponent = CastChecked<USInputComponent>(InputComponent);

	MyInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASPlayerController::Move_Input);
	MyInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASPlayerController::Look_Input);
	MyInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASPlayerController::Interact_Input);
	MyInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ASPlayerController::Inventory_Input);

	
	MyInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);

}

void ASPlayerController::Move_Input(const FInputActionValue& InputAction)
{

	if (GetASC() && GetASC()->HasMatchingGameplayTag(FSGameplayTags::Get().State_Stun))
	{
		return;
	}
	
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

void ASPlayerController::Interact_Input(const FInputActionValue& InputAction)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (PlayerCharacter)
	{
		if (PlayerCharacter->GetTarget() != nullptr)
		{
			IInteractionInterface::Execute_Interact(PlayerCharacter->GetTarget(), PlayerCharacter);
			
		}
	}
}

void ASPlayerController::Inventory_Input(const FInputActionValue& InputAction)
{

	ASHUD* SHUD = Cast<ASHUD>(GetHUD());

	if (SHUD)
	{
		SHUD->ToggleInventory();
	}
}

USAbilitySystemComponent* ASPlayerController::GetASC()
{
	if (SAbilitySystemComponent == nullptr)
	{
		SAbilitySystemComponent = Cast<USAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return SAbilitySystemComponent;
}

void ASPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FSGameplayTags::Get().InputTag_Tag))
	{
		ASPlayerState* PS = GetPlayerState<ASPlayerState>();
		if (PS && PS->PlayerData.Num() > 1)
		{
			int32 NextIndex = (PS->CurrentCharacterIndex + 1) % PS->PlayerData.Num();
			
			Server_RequestCharacterSwap(NextIndex);
		}
		return; 
	}
	
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagPressed(InputTag);
}

void ASPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

void ASPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagHeld(InputTag);
}

