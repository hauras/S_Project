

#include "Ability/GA/SGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Ability/SAttributeSet.h"
#include "Kismet/KismetSystemLibrary.h"

void USGameplayAbility::ExecuteSynergyLogic(AActor* Target)
{
	if (!Target) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();

	if (TargetASC && MyASC)
	{
		// 1. 상대방의 표식이 있는가?
		if (TargetASC->HasMatchingGameplayTag(MarkTag))
		{
			// 표식 제거
			TargetASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(MarkTag));

			// [핵심] 컨텍스트 생성
			FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
            
			// 가해자 정보 주입 (UI 데미지 숫자를 위해 필요)
			Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
			Context.AddSourceObject(GetAvatarActorFromActorInfo());

			/** 
			 * [이펙트 해결사] 위치 정보 주입 
			 * 이 코드가 없으면 GameplayCueParameters.Location이 (0,0,0)으로 전달되어 
			 * 이펙트가 맵 중앙 바닥에서 터지거나 아예 안 보일 수 있습니다.
			 */
			Context.AddOrigin(Target->GetActorLocation());

			FGameplayEffectSpecHandle Spec = MyASC->MakeOutgoingSpec(SynergyEffectClass, GetAbilityLevel(), Context);
			if (Spec.IsValid())
			{
				// [수정] 내 ASC에서 타겟의 ASC로 효과를 적용합니다.
				MyASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
			UE_LOG(LogTemp, Warning, TEXT("Synergy Exploded!!!"));
		}
		else
		{
			const USAttributeSet* AS = Cast<USAttributeSet>(MyASC->GetAttributeSet(USAttributeSet::StaticClass()));
			if (AS)
			{
				float Chance = AS->GetMarkChance(); 
				float RandomRoll = FMath::FRandRange(0.f, 100.f);

				if (RandomRoll <= Chance)
				{
					// [수정] 표식 생성 시에도 동일하게 위치 정보를 담은 컨텍스트 사용
					FGameplayEffectContextHandle MarkContext = MyASC->MakeEffectContext();
					MarkContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
					MarkContext.AddOrigin(Target->GetActorLocation());

					FGameplayEffectSpecHandle MarkSpec = MyASC->MakeOutgoingSpec(MarkEffectClass, GetAbilityLevel(), MarkContext);
					if (MarkSpec.IsValid())
					{
						MyASC->ApplyGameplayEffectSpecToTarget(*MarkSpec.Data.Get(), TargetASC);
					}
				}
			}
		}
	}
}
TArray<FHitResult> USGameplayAbility::GetHitResultFromTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, bool bDrawDebug, bool bIgnoreSelf) const
{
	TArray<FHitResult> HitResults;

	for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
	{
		FVector StartLoc = TargetData->GetOrigin().GetTranslation();
		FVector EndLoc = TargetData->GetEndPoint();

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<AActor*> ActorsToIgnore;
		if (bIgnoreSelf)
		{
			ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
		}

		EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		TArray<FHitResult> Results;
		UKismetSystemLibrary::SphereTraceMultiForObjects(this, StartLoc, EndLoc, SphereSweepRadius,ObjectTypes, false, ActorsToIgnore, DrawDebugTrace, Results, false );
	}
	return HitResults;
}

