

#include "State/SPlayerState.h"

#include "Ability/SAbilitySystemComponent.h"
#include "Ability/SAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Actor/Component/InventoryComponent.h"

ASPlayerState::ASPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<USAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<USAttributeSet>("AttributeSet");

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");

}

void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerState, PlayerData);
	DOREPLIFETIME(ASPlayerState, CurrentCharacterIndex);
	DOREPLIFETIME(ASPlayerState, CurrentCharacterTag);
}


UAbilitySystemComponent* ASPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASPlayerState::OnRep_CurrentCharacterTag(FGameplayTag OldTag)
{
}

void ASPlayerState::OnRep_CurrentCharacterIndex(int32 OldIndex)
{
	OnCharacterIndexChangedDelegate.Broadcast(CurrentCharacterIndex);
}

