// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SProjectTarget : TargetRules
{
	public SProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6; // V5를 V6로 수정
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest; // (선택사항) Unreal5_5 대신 Latest로 두면 다음 업데이트 시 더 편합니다.
		ExtraModuleNames.Add("SProject");
	}
}
