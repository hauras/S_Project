

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

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	SAIController = Cast<ASAIController>(NewController);
	SAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	SAIController->RunBehaviorTree(BehaviorTree);
	SAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	SAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), EnemyType != EEnemyType::Melee);
	
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
		
		OnHealthChanged.Broadcast(EnemyAS->GetHealth());
		OnMaxHealthChanged.Broadcast(EnemyAS->GetMaxHealth());
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
}

void AEnemyCharacter::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;}

AActor* AEnemyCharacter::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}
