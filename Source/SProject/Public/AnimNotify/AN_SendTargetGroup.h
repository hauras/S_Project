
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h" // UAnimNotify -> UAnimNotifyState로 변경
#include "GameplayTagContainer.h"
#include "AN_SendTargetGroup.generated.h"

UCLASS()
class SPROJECT_API UAN_SendTargetGroup : public UAnimNotifyState // 상속 클래스 변경
{
	GENERATED_BODY()

public:
	UAN_SendTargetGroup();

protected:
	// UAnimNotify의 Notify 함수 대신, NotifyState의 Begin/Tick 함수를 사용합니다.
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	
private:
	// 인스펙터에서 편집할 수 있도록 프로퍼티를 만듭니다.
	UPROPERTY(EditAnywhere, Category = "Trace")
	FName StartSocketName; // 시작 소켓 (예: WeaponBaseSocket)

	UPROPERTY(EditAnywhere, Category = "Trace")
	FName EndSocketName; // 끝 소켓 (예: weaponSocket)

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceRadius; // 트레이스 반경

	UPROPERTY(EditAnywhere, Category = "Trace")
	FGameplayTag EventTag; // 보낼 이벤트 태그

	// 이전 프레임의 소켓 위치를 저장할 변수들
	FVector PreviousStartLocation;
	FVector PreviousEndLocation;

	// 한 번의 스윙에 여러 번 맞지 않도록, 이미 맞은 액터들을 저장하는 배열
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AlreadyHitActors;
};