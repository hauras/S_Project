
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "Portal.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class SPROJECT_API APortal : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	APortal();

	virtual void Interact_Implementation(AActor* InInteractor) override;
	virtual void ShowInteractionWidget_Implementation() override;
	virtual void HideInteractionWidget_Implementation() override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	FName TargetLevelName;
	
	UPROPERTY(VisibleAnywhere, Category = "Portal")
	TObjectPtr<UBoxComponent> PortalCollision;

	UPROPERTY(EditAnywhere, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> PortalMesh;
};
