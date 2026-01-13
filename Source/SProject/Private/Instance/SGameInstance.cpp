

#include "Instance/SGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemNames.h"

USGameInstance::USGameInstance()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();

        SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateSessionComplete);
        SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionComplete);
        SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete); 
    }
}

void USGameInstance::HostRoom()
{
    if (!SessionInterface.IsValid()) return;
    
    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = 2;
    SessionSettings.bIsLANMatch = false;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUsesPresence = true;
    SessionSettings.bAllowJoinInProgress = true;

    SessionInterface->CreateSession(0, FName("MyGameSession"), SessionSettings);
    UE_LOG(LogTemp, Warning, TEXT("방 만들기 요청 보냄..."));
}

void USGameInstance::FindRoom()
{
    if (!SessionInterface.IsValid()) return;

    // 1. 검색 상자 준비 (TSharedPtr이므로 ToSharedRef()로 넘겨야 함)
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    
    // 2. 검색 설정
    SessionSearch->MaxSearchResults = 20;
    SessionSearch->bIsLanQuery = false; // ★ bIsLANMatch가 아니라 bIsLanQuery입니다.

    // 3. 필터 설정 (SEARCH_PRESENCE를 쓰려면 상단 헤더 include 필요)
    SessionSearch->QuerySettings.Set(FName(TEXT("SEARCH_PRESENCE")), true, EOnlineComparisonOp::Equals);

    // 4. 검색 시작!
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
    
    UE_LOG(LogTemp, Warning, TEXT("방 찾는 중..."));
}

void USGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {

        UGameplayStatics::OpenLevel(GetWorld(), FName("DevMap"), true, "listen");
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("방 만들기 실패..."));

    }
}

void USGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
    if (bWasSuccessful && SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("방 찾기 완료! 검색된 개수: %d"), SessionSearch->SearchResults.Num());

        if (SessionSearch->SearchResults.Num() > 0)
        {
            // 가장 먼저 검색된 첫 번째 방에 입장을 시도합니다.
            // (나중에 UI를 만들면 목록에서 선택하게 바꿀 수 있습니다.)
            SessionInterface->JoinSession(0, FName("MyGameSession"), SessionSearch->SearchResults[0]);
        }
    }
}

void USGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        if (APlayerController* PC = GetFirstLocalPlayerController())
        {
            FString Address;
            // 스팀 서버로부터 접속할 주소를 받아옵니다.
            if (SessionInterface->GetResolvedConnectString(SessionName, Address))
            {
                // ★ 핵심: 해당 주소로 나의 레벨을 이동시킵니다 (원정 시작)
                PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
            }
        }
    }
}
