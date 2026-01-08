

#include "Ability/SDamageExecution.h"

#include "Ability/SAttributeSet.h"

struct SDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(IncomingDamage);

	SDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(USAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USAttributeSet, IncomingDamage, Target, false);
	}
};

static const SDamageStatics& DamageStatics()
{
	static SDamageStatics DamageStatics;
	return DamageStatics;
}

USDamageExecution::USDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().IncomingDamageDef);
}

void USDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                               FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float SourceAttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluateParams, SourceAttackPower);
	SourceAttackPower = FMath::Max<float>(0.f, SourceAttackPower);

	float FinalDamage = SourceAttackPower;

	const FGameplayModifierEvaluatedData EvaluatedData(DamageStatics().IncomingDamageProperty, EGameplayModOp::Additive, FinalDamage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
