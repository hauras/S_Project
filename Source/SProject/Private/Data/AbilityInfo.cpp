

#include "Data/AbilityInfo.h"

FSAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FSAbilityInfo& Info : AbilityInfo)
	{
		if(Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT(   "어빌리티 정보 [%s]에서 태그정보[%s]를 찾을 수 없음 "), *AbilityTag.ToString(), *GetNameSafe(this));
	}
	return FSAbilityInfo();
}
