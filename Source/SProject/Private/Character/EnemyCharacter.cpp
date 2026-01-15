

#include "Character/EnemyCharacter.h"

#include "SGameplayTags.h"
#include "Ability/SAbilityFunctionLibrary.h"
#include "Ability/SAbilitySystemComponent.h"
#include "Ability/SAttributeSet.h"
#include "AI/SAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/SUserWidgetBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameMode/SGameMode.h"

AEnemyCharacter::AEnemyCharacter()
{

	AbilitySystemComponent = CreateDefaultSubobject<USAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<USAttributeSet>("AttributeSet");

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 180.f, 0.f);

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	
	if (HasAuthority())
	{
		USAbilityFunctionLibrary::GiveStartupAbilities(this, AbilitySystemComponent, EnemyType);
	}
	if (USUserWidgetBase* SUserWidget = Cast<USUserWidgetBase>(HealthBar->GetUserWidgetObject()))
	{
		SUserWidget->SetWidgetController(this);
	}

	if (const USAttributeSet* EnemyAS = Cast<USAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(EnemyAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);	

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(EnemyAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
		
		AbilitySystemComponent->RegisterGameplayTagEvent(FSGameplayTags::Get().Ability_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&AEnemyCharacter::HitReactTagChanged
		);

		AbilitySystemComponent->RegisterGameplayTagEvent(FSGameplayTags::Get().State_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&AEnemyCharacter::StunTagChanged
		);	
		OnHealthChanged.Broadcast(EnemyAS->GetHealth());
		OnMaxHealthChanged.Broadcast(EnemyAS->GetMaxHealth());
	}
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 이 로직은 서버에서만 실행되어야 합니다.
	if (!HasAuthority()) 
	{
		return;
	}

	// 1. 이 캐릭터를 조종할 AI 컨트롤러로 캐스팅을 '시도'합니다.
	SAIController = Cast<ASAIController>(NewController);

	// 2. '만약' 캐스팅에 성공했고(SAIController가 유효하고),
	//    블루프린트에서 행동 트리 애셋이 제대로 할당되었다면...
	if (SAIController && BehaviorTree)
	{
		// 3. 컨트롤러에게 행동 트리를 실행하라고 '명령'합니다.
		// RunBehaviorTree 함수가 내부적으로 블랙보드 초기화를 안전하게 처리해 줍니다.
		SAIController->RunBehaviorTree(BehaviorTree);
		
		// 4. 행동 트리가 실행된 후에, 블랙보드에 초기값을 설정합니다.
		UBlackboardComponent* BlackboardComp = SAIController->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(FName("HitReacting"), false);
			BlackboardComp->SetValueAsBool(FName("RangedAttacker"), EnemyType != EEnemyType::Melee);
		}
	}
	
	// 만약 캐스팅에 실패했거나(AI 컨트롤러가 아니거나), BehaviorTree가 할당되지 않았다면,
	// 아무것도 하지 않고 함수를 안전하게 종료합니다.
}

void AEnemyCharacter::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;

	if (SAIController && SAIController->GetBlackboardComponent())
	{
		SAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void AEnemyCharacter::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	FString DebugMsg = FString::Printf(TEXT("적 스턴 상태: %s (태그 개수: %d)"), (NewCount > 0 ? TEXT("YES") : TEXT("NO")), NewCount);
	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, DebugMsg);
	
	bool bIsStunned = NewCount > 0;

	// 1. 물리적으로 멈추기 (선택 사항)
	if (bIsStunned) GetCharacterMovement()->MaxWalkSpeed = 0.f;
	else GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	// 2. [핵심 ⭐] 블랙보드에 "나 기절했어!"라고 알리기
	if (SAIController && SAIController->GetBlackboardComponent())
	{
		SAIController->GetBlackboardComponent()->SetValueAsBool(FName("IsStunned"), bIsStunned);
	}
}


void AEnemyCharacter::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Super::InitAbilityActorInfo();

	InitializeDefaultAttributes();	
}


void AEnemyCharacter::InitializeDefaultAttributes() const
{
	USAbilityFunctionLibrary::InitializeDefaultAttributes(this, EnemyType, Level, AbilitySystemComponent);
}

void AEnemyCharacter::Die()
{
	SetLifeSpan(LifeSpan);
	Super::Die();

	if (HasAuthority())
	{
		ASGameMode* GameMode = GetWorld()->GetAuthGameMode<ASGameMode>();
		if (GameMode)
		{
			GameMode->OnEnemyKilled();
		}
	}
	
}

void AEnemyCharacter::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;}

AActor* AEnemyCharacter::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

/*bool AEnemyCharacter::IsBoss_Implementation() const
{
	return EnemyType == EEnemyType::Boss;
}
*/