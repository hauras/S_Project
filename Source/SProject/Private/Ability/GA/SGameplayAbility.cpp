

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
            
			Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
			Context.AddSourceObject(GetAvatarActorFromActorInfo());
			Context.AddOrigin(Target->GetActorLocation());

			FGameplayEffectSpecHandle Spec = MyASC->MakeOutgoingSpec(SynergyEffectClass, GetAbilityLevel(), Context);
			if (Spec.IsValid())
			{
				MyASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
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

