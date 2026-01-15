

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

}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitAbilityActorInfo(); 
	AddCharacterAbilities();

	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());
	ASPlayerState* PS = GetPlayerState<ASPlayerState>();
	
	if (GI && PS)
	{
		FString PlayerID = PS->GetPlayerName();
		
		// ★ 핵심 1: 금고에 내 데이터가 "진짜" 있을 때만 로드를 실행합니다.
		if (GI->PlayerData.Contains(PlayerID) && GI->PlayerData[PlayerID].bIsDataValid)
		{
			// 짐 풀기 (여기서 장비도 입히고 현재 피도 맞춥니다)
			LoadProgressFromGameInstance();
		}
		else
		{
			// ★ 핵심 2: 금고가 비어있을 때만(게임 첫 시작 등) 기본 초기화를 합니다.
			// 이렇게 하면 중복으로 스탯이 더해지는 것을 원천 차단할 수 있습니다.
			InitializeDefaultAttributes();
		}
	}
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
	if (!HasAuthority()) return; 

	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());
	ASPlayerState* PS = GetPlayerState<ASPlayerState>();
	USAttributeSet* AS = Cast<USAttributeSet>(AttributeSet);

	if (GI && PS && AS && PS->Inventory)
	{
		FString MyID = PS->GetPlayerName();

		if (GI->PlayerData.Contains(MyID))
		{
			FSPlayerData& MyData = GI->PlayerData[MyID];
			if (MyData.bIsDataValid)
			{
				// 서버가 직접 데이터를 복구합니다.
				AS->SetMaxHealth(MyData.MaxHealth);
				// ... (기존 로드 로직 동일) ...
				PS->Inventory->LoadInventoryData(MyData.Inventory, MyData.EquippedItems);
				AS->SetHealth(MyData.Health);
				
				// [중요] 서버가 값을 바꿨으니, 리플리케이션을 통해 클라이언트에게 전달됩니다.
				GI->PlayerData.Remove(MyID);
			}
		}
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
