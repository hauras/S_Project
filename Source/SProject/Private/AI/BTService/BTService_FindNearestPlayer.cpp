

#include "AI/BTService/BTService_FindNearestPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/PlayerCharacter.h"
#include "GameFramework/Pawn.h" // APawn을 사용하기 위해
#include "Kismet/GameplayStatics.h"

UBTService_FindNearestPlayer::UBTService_FindNearestPlayer()
{
	NodeName = TEXT("Find Nearest Player");
}

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
 
	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (OwningPawn == nullptr) return;

	const TSubclassOf<APawn> TargetClass = APlayerCharacter::StaticClass()->GetClass();
	TArray<AActor*> ActorOfClass;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetClass, ActorOfClass);
	if (ActorOfClass.Num() == 0) return;

	AActor* ClosestActor = nullptr;
	float ClosestDistance = TNumericLimits<float>::Max();

	for (AActor* Actor : ActorOfClass)
	{
		if (IsValid(Actor))
		{
			const float Distance = OwningPawn->GetDistanceTo(Actor);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = Actor;
			}
		}
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr) return;
	
	// 6. 에디터에서 연결한 Selector 변수를 사용하여 블랙보드에 값을 설정합니다.
	BlackboardComp->SetValueAsObject(TargetToSelector.SelectedKeyName, ClosestActor);
	BlackboardComp->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, ClosestDistance);
}
