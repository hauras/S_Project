#include "Character/BossCharacter.h"
#include "Components/TimelineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"

ABossCharacter::ABossCharacter()
{
	// 타임라인 컴포넌트 생성
	HitTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("HitTimeline"));
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 1. 동적 머티리얼 인스턴스 생성
	if (GetMesh() && GetMesh()->GetMaterial(0))
	{
		BodyMID = GetMesh()->CreateDynamicMaterialInstance(0, GetMesh()->GetMaterial(0));
	}

	// 2. 피격 타임라인 바인딩
	if (HitCurve)
	{
		FOnTimelineFloat HitUpdateFunc;
		HitUpdateFunc.BindUFunction(this, FName("UpdateHitFxSwitch"));
		HitTimeline->AddInterpFloat(HitCurve, HitUpdateFunc);
	}

	// 3. 죽음 디졸브 타임라인 바인딩
	if (DissolveCurve)
	{
		FOnTimelineFloat DissolveUpdateFunc;
		DissolveUpdateFunc.BindUFunction(this, FName("UpdateDeathDissolve"));
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveUpdateFunc);

		// 타임라인 종료 이벤트 연결
		FOnTimelineEvent FinishedFunc;
		FinishedFunc.BindUFunction(this, FName("OnDissolveTimelineFinished"));
		DissolveTimeline->SetTimelineFinishedFunc(FinishedFunc);
	}
}

void ABossCharacter::PlayHitReactEffect()
{
	if (HitTimeline)
	{
		HitTimeline->PlayFromStart();
	}
}

void ABossCharacter::Die()
{
	// 부모의 Die를 호출하여 기본 죽음 처리 수행
	Super::Die();

	// 보스는 디졸브 연출 완료 후 직접 Destroy 할 것이므로 자동 삭제(LifeSpan) 방지
	SetLifeSpan(0.0f); 
}

void ABossCharacter::MulticastHandleDeath_Implementation()
{
	// 1. 죽음 몽타주 재생 (부모 클래스 로직 실행)
	Super::MulticastHandleDeath_Implementation();

	// 2. ★ 수정 포인트: 3.35초 대신 3.0초~3.1초 정도로 앞당깁니다.
	// 몽타주가 끝나고 Idle로 돌아가려고 하기 '직전'에 고정해야 "일어나는 현상"이 없습니다.
	const float DeathAnimDuration = 3.1f; 

	FTimerHandle DeathEffectTimerHandle;
	GetWorldTimerManager().SetTimer(DeathEffectTimerHandle, [this]()
	{
		// A. 쓰러진 포즈 그대로 고정 (몽타주 도중에 멈추므로 Idle 팝업 방지)
		if (GetMesh())
		{
			GetMesh()->bPauseAnims = true;
		}

		// B. 나이아가라 이펙트 스폰
		if (DeathNiagaraSystem)
		{
			// Mesh Reproduction 방식은 오프셋이 0이어야 몸체와 딱 맞게 소환됩니다.
			DeathNiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				DeathNiagaraSystem,
				GetMesh(),
				NAME_None,
				FVector::ZeroVector, 
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true
			);
		}

		// C. 머티리얼 디졸브 시작
		if (DissolveTimeline)
		{
			DissolveTimeline->PlayFromStart();
		}

	}, DeathAnimDuration, false);
}

void ABossCharacter::UpdateHitFxSwitch(float HitFxValue)
{
	if (BodyMID)
	{
		// 피격 시 번쩍이는 효과 파라미터 업데이트
		BodyMID->SetScalarParameterValue(FName("HitFxSwitch"), HitFxValue);
	}
}

void ABossCharacter::UpdateDeathDissolve(float DissolveValue)
{
	// 1. 머티리얼을 서서히 투명하게 만듦
	if (BodyMID)
	{
		BodyMID->SetScalarParameterValue(FName("DissolveAmount"), DissolveValue);
	}

	// 2. 나이아가라 이펙트에게 현재 디졸브 진행도를 전달
	if (DeathNiagaraComp)
	{
		// 나이아가라 에디터 하단 'User Parameters'에 정의된 이름과 똑같아야 합니다.
		// 보통 이런 이펙트는 "User.DissolveAmount"나 "User.Progress" 같은 이름을 씁니다.
		DeathNiagaraComp->SetNiagaraVariableFloat(TEXT("User.DissolveAmount"), DissolveValue);
	}
}

void ABossCharacter::OnDissolveTimelineFinished()
{
	// 모든 연출이 끝났으므로 월드에서 제거
	Destroy();
}