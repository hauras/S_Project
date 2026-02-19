
#include "UI/Widget/MiniMap.h"

#include "Actor/Map/DungeonGenerator.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/Widget/RoomIconWidget.h"
#include "Components/Image.h"

void UMiniMap::UpdateMap()
{
	// RoomCanvas만 비웁니다. PlayerMarker가 있는 EntityCanvas는 건드리지 않습니다.
	if (!MapCanvas || !RoomIconClass) return;
	MapCanvas->ClearChildren();

	ADungeonGenerator* Generator = Cast<ADungeonGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonGenerator::StaticClass()));
	if (!Generator) return;

	const TArray<FRoomData>& Layout = Generator->GetDungeonLayout();
	
	for (const FRoomData& Data : Layout)
	{
		URoomIconWidget* IconWidget = CreateWidget<URoomIconWidget>(GetWorld(), RoomIconClass);
		if (IconWidget)
		{
			// 1. 캔버스에 추가 (반환된 슬롯을 바로 사용)
			UCanvasPanelSlot* MapSlot = MapCanvas->AddChildToCanvas(IconWidget);
			
			// 2. 방 타입 비주얼 설정 (보스면 해골 등)
			IconWidget->SetRoomTypeVisuals(Data.RoomType);

			// 3. 위치 배치 (X*50, Y*-50)
			if (MapSlot)
			{
				FVector2D MapPos;
				MapPos.X = Data.GridLocation.X * SlotSize;
				MapPos.Y = Data.GridLocation.Y * -SlotSize;

				MapSlot->SetPosition(MapPos);
				MapSlot->SetSize(FVector2D(SlotSize, SlotSize));
				MapSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 중앙 정렬
			}
		}
	}
}

void UMiniMap::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!PlayerMarker) return;

	ADungeonGenerator* Generator = Cast<ADungeonGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), ADungeonGenerator::StaticClass()));
	if (!Generator) return;

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!PlayerPawn) return;

	// 1. 현재 월드 좌표를 그리드 좌표로 변환
	FIntPoint GridPos = Generator->GetGridCoordsFromWorldLocation(PlayerPawn->GetActorLocation());

	// 2. 마커의 UI 위치 계산
	FVector2D MarkerPos;
	MarkerPos.X = GridPos.X * SlotSize;
	MarkerPos.Y = GridPos.Y * -SlotSize;

	// 3. 마커 위치 업데이트 (Canvas Slot으로 형변환하여 접근)
	UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(PlayerMarker->Slot);
	if (MarkerSlot)
	{
		MarkerSlot->SetPosition(MarkerPos);
	}
}
