

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
	
	AddCharacterAbilities();
	InitializeDefaultAttributes();
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo();
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
		if (HitActor != Target) // 보고 있는 놈이 바뀌었다면
		{
			// [1] 이전에 보던 놈이 있었다면 꺼줍니다.
			if (Target)
			{
				IInteractionInterface::Execute_HideInteractionWidget(Target);
			}

			// [2] 이제 새로운 대상을 기억합니다.
			Target = HitActor;

			// [3] 새로운 대상의 위젯을 켜줍니다.
			IInteractionInterface::Execute_ShowInteractionWidget(Target);
		}
	}
	else // 아무것도 안 맞았거나, 인터페이스가 없는 물체라면
	{
		// [4] 이전에 보던 놈이 아직 남아있다면 정리합니다.
		if (Target)
		{
			IInteractionInterface::Execute_HideInteractionWidget(Target);
			Target = nullptr; // 이제 아무것도 안 보고 있으니 비워줍니다.
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


	if (ASPlayerController* SPlayerController = Cast<ASPlayerController>(GetController()))
	{
		if (ASHUD* SHUD = Cast<ASHUD>(SPlayerController->GetHUD()))
		{
			SHUD->InitOverlay(SPlayerController, SPlayerState, AbilitySystemComponent, AttributeSet);
			SHUD->InitInventory(SPlayerController, SPlayerState, AbilitySystemComponent, AttributeSet);

		}
	}
}
