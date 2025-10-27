

#include "Character/PlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Controller/SPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "State/SPlayerState.h"
#include "UI/HUD/SHUD.h"

APlayerCharacter::APlayerCharacter()
{
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
	
	// 2단계: 그 다음, 서버의 고유 권한인 '상태 결정' 담당자들을 순서대로 호출한다.
	AddCharacterAbilities();
	InitializeDefaultAttributes();
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo();

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
		}
	}
}
