

#include "Data/EnemyStats.h"

FEnemyClassDefaultInfo UEnemyStats::GetClassDefaultInfo(EEnemyType EnemyClass)
{
	return EnemyClassInfo.FindChecked(EnemyClass);
}
