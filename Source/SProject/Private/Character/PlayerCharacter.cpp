

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

	// ★ 수정: 금고 유무와 상관없이 무조건 기본 스탯(100)을 먼저 채웁니다!
	// 그래야 나중에 로드할 때 Clamp에 안 걸립니다.
	InitializeDefaultAttributes(); 

	// 그 다음, 금고에 데이터가 있다면 덮어씌웁니다.
	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());
	if (GI && GI->PlayerData.bIsDataValid)
	{
		LoadProgressFromGameInstance();
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
	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());
	if (GI && GI->PlayerData.bIsDataValid)
	{
		ASPlayerState* PS = GetPlayerState<ASPlayerState>();
		USAttributeSet* AS = Cast<USAttributeSet>(AttributeSet);

		if (PS && PS->Inventory && AS)
		{
	
			AS->SetMaxHealth(GI->PlayerData.MaxHealth);
			AS->SetMaxMana(GI->PlayerData.MaxMana);
			AS->SetAttackPower(GI->PlayerData.AttackPower);

		
			PS->Inventory->LoadInventoryData(GI->PlayerData.Inventory, GI->PlayerData.EquippedItems);


			AS->SetHealth(GI->PlayerData.Health);
			AS->SetMana(GI->PlayerData.Mana);

			UE_LOG(LogTemp, Warning, TEXT("데이터 로드 완료! (중복 합산 방지 적용)"));
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
