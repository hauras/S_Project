

#include "Character/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Controller/SPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "State/SPlayerState.h"
#include "UI/HUD/SHUD.h"
#include "Interface/InteractionInterface.h"
#include "Actor/Component/InventoryComponent.h"

#include "DrawDebugHelpers.h" // 이 줄을 맨 위에 추가하세요.
#include "Ability/SAttributeSet.h"
#include "Instance/SGameInstance.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitAbilityActorInfo(); 
	
	// 2. 스킬 부여
	AddCharacterAbilities();

	// 3. 기본 스탯 
	InitializeDefaultAttributes();

	LoadProgressFromGameInstance(); 
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo();

	LoadProgressFromGameInstance(); 

}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PerformInteractionTrace();

}

void APlayerCharacter::PerformInteractionTrace()
{
	FVector Start = Camera->GetComponentLocation(); // 시작점
	FVector ForwardVector = Camera->GetForwardVector(); // 방향

	FVector End = Start + (ForwardVector * 800.f);

	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	AActor* HitActor = HitResult.GetActor();
	IInteractionInterface* Interface = Cast<IInteractionInterface>(HitActor);
	
	if (bHit && Interface)
	{
		if (HitActor != Target) 
		{
			if (Target)
			{
				IInteractionInterface::Execute_HideInteractionWidget(Target);
			}

			Target = HitActor;

			IInteractionInterface::Execute_ShowInteractionWidget(Target);
		}
	}
	else 
	{
		if (Target)
		{
			IInteractionInterface::Execute_HideInteractionWidget(Target);
			Target = nullptr; 
		}
	}
}

void APlayerCharacter::LoadProgressFromGameInstance()
{
	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());
	if (GI && GI->PlayerData.bIsDataValid)
	{
		UInventoryComponent* InvComp = FindComponentByClass<UInventoryComponent>();
		USAttributeSet* AS = Cast<USAttributeSet>(AttributeSet);

		if (InvComp && AS)
		{
			// 1. [먼저] 장비를 다시 입힙니다. ⭐
			// 이 함수가 실행되면서 GE가 적용되어 MaxHealth가 100 -> 120으로 자동으로 올라갑니다.
			InvComp->LoadInventoryData(GI->PlayerData.Inventory, GI->PlayerData.EquippedItems);

			// 2. [그 다음] 현재의 실시간 수치(Health, Mana)만 금고에서 꺼내 덮어씌웁니다.
			// 이미 장비로 인해 Max가 120이 된 상태이므로, 120을 넣어도 잘리지 않습니다.
			AS->SetHealth(GI->PlayerData.Health);
			AS->SetMana(GI->PlayerData.Mana);
            
			// 만약 '강화석'으로 올린 순수 공격력이 있다면 그건 따로 처리가 필요하겠지만,
			// 일단 장비 문제는 이렇게 하면 해결됩니다.
            
			UE_LOG(LogTemp, Warning, TEXT("데이터 로드 완료! 현재 체력: %f"), AS->GetHealth());
		}

		GI->PlayerData.bIsDataValid = false;
	}
}

void APlayerCharacter::InitAbilityActorInfo()
{
	ASPlayerState* SPlayerState = GetPlayerState<ASPlayerState>();
	check(SPlayerState);
	SPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(SPlayerState, this);
	AbilitySystemComponent = SPlayerState->GetAbilitySystemComponent();
	AttributeSet = SPlayerState->GetAttributeSet();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(CharacterTag);
		UE_LOG(LogTemp, Warning, TEXT("캐릭터 '%s' 태그가 ASC에 등록되었습니다!"), *CharacterTag.ToString());

	}
	if (ASPlayerController* SPlayerController = Cast<ASPlayerController>(GetController()))
	{
		if (ASHUD* SHUD = Cast<ASHUD>(SPlayerController->GetHUD()))
		{
			SHUD->InitOverlay(SPlayerController, SPlayerState, AbilitySystemComponent, AttributeSet);
			SHUD->InitInventory(SPlayerController, SPlayerState, AbilitySystemComponent, AttributeSet);

		}
	}

	
}
