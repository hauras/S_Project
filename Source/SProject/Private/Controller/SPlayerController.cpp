

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

void ASPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter)
{
	if (IsValid(TargetCharacter) && DamageTextClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount);
	}
}

void ASPlayerController::Server_RequestCharacterSwap_Implementation(int32 NewIndex)
{
	ASPlayerState* PS = GetPlayerState<ASPlayerState>();
	// 1. 안전장치: 데이터 유효성 확인
	if (!PS || !PS->PlayerData.IsValidIndex(NewIndex) || !PS->PlayerData[NewIndex]) return;

	APlayerCharacter* OldCharacter = Cast<APlayerCharacter>(GetPawn());
	if (!OldCharacter) return;

	// 위치/속도/회전 백업
	FTransform SpawnTransform = OldCharacter->GetActorTransform();
	FVector OldVelocity = OldCharacter->GetVelocity();

	// 짐 싸기 로직 (TMap 자료구조 대응)
	if (USGameInstance* GI = Cast<USGameInstance>(GetGameInstance()))
	{
		const USAttributeSet* AS = Cast<USAttributeSet>(OldCharacter->GetAttributeSet());
		UAbilitySystemComponent* ASC = OldCharacter->GetAbilitySystemComponent();

		if (AS && ASC && PS->Inventory)
		{
			// 2. [핵심 ⭐] 나만의 "개인용 보따리" 생성
			FSPlayerData MyData;

			// 가방 리스트 복사
			MyData.Inventory = PS->Inventory->GetInventoryList();

			// ★ [수정] 장착 아이템 변환 (Array -> Map)
			// 컴포넌트의 복제용 배열을 순회하며 금고의 TMap에 하나씩 채워넣습니다.
			for (const FEquippedItemInfo& EquipInfo : PS->Inventory->GetEquippedItemsArray())
			{
				MyData.EquippedItems.Add(EquipInfo.Slot, EquipInfo.ItemData);
			}

			// 스탯 기본값 저장 (중복 합산 방지 알고리즘)
			MyData.MaxHealth = ASC->GetNumericAttributeBase(AS->GetMaxHealthAttribute());
			MyData.MaxMana = ASC->GetNumericAttributeBase(AS->GetMaxManaAttribute());
			MyData.AttackPower = ASC->GetNumericAttributeBase(AS->GetAttackPowerAttribute());

			// 실시간 수치 저장
			MyData.Health = AS->GetHealth();
			MyData.Mana = AS->GetMana();
			MyData.bIsDataValid = true;

			// 3. [핵심 ⭐] PlayerDataMap에 내 이름표로 저장
			FString PlayerID = PS->GetPlayerName();
			GI->PlayerData.Add(PlayerID, MyData);
		}
	}
	
	// --- 아래 소환 및 빙의 로직은 동일합니다 ---
	
	UnPossess();
	OldCharacter->Destroy();
	
	UClass* NewCharacterClass = PS->PlayerData[NewIndex]->CharacterInfo.CharacterClass;
	FGameplayTag NewCharacterTag = PS->PlayerData[NewIndex]->CharacterInfo.CharacterTag;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	APlayerCharacter* NewCharacter = GetWorld()->SpawnActor<APlayerCharacter>(NewCharacterClass, SpawnTransform, SpawnParams);
	
	if (NewCharacter)
	{
		NewCharacter->CharacterTag = NewCharacterTag;
		PS->CurrentCharacterTag = NewCharacterTag;
		PS->CurrentCharacterIndex = NewIndex;

		Possess(NewCharacter);

		// [중요] 새로 소환된 몸이 GI에서 방금 저장한 내 짐을 다시 풉니다.
		NewCharacter->InitAbilityActorInfo();

		if (NewCharacter->GetCharacterMovement())
		{
			NewCharacter->GetCharacterMovement()->Velocity = OldVelocity;
		}

		UE_LOG(LogTemp, Warning, TEXT("[%s] 캐릭터 교체 및 데이터 보존 완료!"), *PS->GetPlayerName());
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
			// 현재 0번이면 1번으로, 1번이면 0번으로 바꾸는 수학적 공식
			int32 NextIndex = (PS->CurrentCharacterIndex + 1) % PS->PlayerData.Num();
			
			// 서버에 교체 요청!
			Server_RequestCharacterSwap(NextIndex);
		}
		return; // 교체 시에는 스킬 로직을 타지 않게 막습니다.
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

