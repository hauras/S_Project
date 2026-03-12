# 🗺️ 하정빈 포트폴리오
> "프레임의 낭만, 끝까지 쫓다."
> 
> **C++, 언리얼 기반 게임 클라이언트 프로그래머.** <br>
> 1 프레임의 성능 최적화를 위해 끝까지 파고드는 개발자 하정빈입니다. "동작하는 코드"를 넘어 "성능과 구조가 아름다운 코드"를 지향하며, 엔진 레벨의 깊이 있는 이해를 바탕으로 문제를 해결합니다.

---

## 목차<a name="table-of-contents"></a>

<table>
  <thead>
    <tr>
      <th>🎮 GAS 프로젝트 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>
    </tr>
  </thead>
<tbody>
    <tr>
      <td valign="top">
        <br>
        <b><a href="#eternal-return-main">🎮 프로젝트 메인</a></b><br>
        <b><a href="#game-overview">📖 게임 개요</a></b><br>
        <b><a href="#learning-objectives">📌 학습 목표 및 달성</a></b><br>
        <b><a href="#main-features">🔨 주요 개발</a></b><br>
        <b><a href="#troubleshooting-eternal-return">🛠️ 문제 해결</a></b><br>
        &nbsp;&nbsp; └ <a href="#deferred-rendering"> 레벨 전환 및 태그 시 상태 데이터 유실 이슈 </a><br>
        &nbsp;&nbsp; └ <a href="#navmesh-optimization">"추후 추가"</a><br>
        &nbsp;&nbsp; └ <a href="#quadtree-optimization">"추후 추가"</a><br>
        &nbsp;&nbsp; └ <a href="#fsm-to-bt">"추후 추가"</a>
      </td>
      </tr>
  </tbody>
</table>
<br>
<br>

---
<br>
<br>

# 🎮 GAS 프로젝트<a name="eternal-return-main"></a>

### 📌 프로젝트 정보

| 항목 | 내용 |
|:---:|:---:|
| 🎯 **장르** | 멀티플레이어 액션 로그라이크 |
| ⏱️ **개발 기간** | 2개월 |
| 👥 **개발 인원** | 1인 (프로그래머로 참여) |
| 🛠️ **개발 환경** | C++, Unreal Engine 5, GAS |
| 🎬 **시연 영상** | [YouTube 하이라이트 영상 바로가기](💡유튜브_전체시연영상_링크_삽입) |
| 💾 **GitHub** | [소스코드 레포지토리 바로가기](💡깃허브_메인_링크_삽입) |

## 📑 프로젝트 목차<a name="toc-eternal"></a>

**1. 📖 [게임 개요](#game-overview)**

**2. 📌 [학습 목표 및 달성](#learning-objectives)**

**3. 🔨 [주요 개발 기능](#main-features)** <br>
&nbsp;&nbsp; └ <a href="#dungeon-generation">던전 생성 (절차적 맵 생성 최적화)</a><br>
&nbsp;&nbsp; └ <a href="#character-tag-system">캐릭터 태그 (실시간 스왑 및 시너지)</a><br>
&nbsp;&nbsp; └ <a href="#combat-system">전투 시스템 (GAS 코어 아키텍처)</a><br>
&nbsp;&nbsp; └ <a href="#multiplayer-sync">멀티플레이 (추후 추가)</a><br>
&nbsp;&nbsp; └ <a href="#monster-ai">몬스터 AI (추후 추가)</a>

<br>

---

## 1. 📖 게임 개요 <a name="game-overview"></a>
> **"절차적으로 생성되는 미지의 던전, 실시간 태그와 시너지로 돌파하라!"**

언리얼 엔진의 **GAS(Gameplay Ability System)**를 심도 있게 활용하여 개발한 멀티플레이 액션 로그라이크 게임입니다. 플레이어는 전사와 마법사 등 다양한 클래스를 실시간으로 교체(태그)하며 전투를 벌입니다. 단순히 캐릭터를 바꾸는 것을 넘어, '전사로 표식을 남기고 마법사로 폭발시키는' 형태의 **'시너지 어택(Synergy Attack)'**을 통해 액션과 전략의 재미를 극대화했습니다. 

> **💡 [최고의 하이라이트 움짤 1개 삽입 추천]**
> * **추천 장면:** 전사로 대쉬 공격 ➔ 공중에서 마법사로 태그 ➔ 폭발 스킬 적중 (가장 화려하고 시스템이 잘 보이는 5초 내외 움짤)

<br>

## 2. 📌 학습 목표 및 달성 <a name="learning-objectives"></a>
본 프로젝트는 언리얼 엔진의 코어 시스템을 완벽히 통제하고, 실무 수준의 아키텍처 설계와 최적화를 경험하는 것을 목표로 진행되었습니다.

* **GAS 아키텍처의 완벽한 내재화:** 단순한 스킬 구현을 넘어 `ExecutionCalculation`을 활용한 복잡한 데미지 연산과 데이터 드리븐(Data-Driven) 전투 파이프라인을 성공적으로 구축했습니다.
* **대규모 맵 환경의 메모리/프레임 최적화:** BFS 알고리즘과 비트마스크 연산을 결합해 로그라이크 맵을 생성하고, 동적 렌더링 컬링(`SetShouldBeVisible`)으로 멀티플레이 환경의 프레임 드랍을 완벽히 방어했습니다.
* **안정적인 생명주기(Lifecycle) 및 네트워크 동기화:** 잦은 액터 파괴/생성 시 발생하는 데이터 유실 문제를 `PlayerState` 기반의 ASC 관리와 플래그 제어를 통해 원천 차단했습니다.

<br>

## 3. 🔨 주요 개발 기능 <a name="main-features"></a>

### 🏰 [던전 생성] BFS 기반 절차적 던전 생성 및 최적화 <a name="dungeon-generation"></a>
멀티플레이어 환경에 맞춰 동적으로 로그라이크 맵을 생성하고, 대규모 맵에서도 메모리와 프레임을 방어할 수 있도록 구조적 최적화를 진행했습니다.

<div align="center">
  <img width="276" height="318" alt="image" src="https://github.com/user-attachments/assets/27940c70-544f-4265-b358-a26db1e98f69" />
  <img width="238" height="184" alt="image" src="https://github.com/user-attachments/assets/91fd0230-8810-451b-ae8e-40ce18b886b1" />
  <img width="379" height="382" alt="image" src="https://github.com/user-attachments/assets/2e07fc2a-3af7-4046-a827-928a4c8d2afb" />
  <br>
  <i>"방 개수 조절을 통한 맵 확장성 및 프레임 드랍(렉) 방지를 위해 플레이어 인접 방만 실시간으로 렌더링하는 동적 시야(Dynamic Culling) 최적화 시스템"</i>
</div>
<br>

**1. 무작위 너비 우선 탐색(BFS)을 활용한 레벨 디자인**
* **도입 배경:** 멀티플레이 시 유저들의 동선이 과도하게 멀어지는 DFS(선형 구조)의 단점을 보완하기 위해, 시작점 기준 사방으로 밀집되어 퍼져나가는 BFS 알고리즘을 채택했습니다. [[📄방 생성 로직]](https://github.com/hauras/S_Project/blob/main/Source/SProject/Private/Actor/Map/DungeonGenerator.cpp#L59-L106)
* **Depth 기반 밸런싱:** 노드별 최단 거리(`Depth`)를 추적하여, 가장 먼 `Max Depth` 방에 **보스 방**을, 막다른 길(Dead End)에 확률적으로 **보물 방**을 배치해 탐험의 서사를 제어했습니다.

**2. 비트마스크(Bitmask)를 이용한 방 연결 및 데이터 경량화**
* 방 4면의 문(Gate) 연결 상태를 `int32`의 단 **4비트(1:N, 2:S, 4:W, 8:E)** 로 압축하여 멀티플레이 환경의 네트워크 복제(Replication) 대역폭을 획기적으로 절약했습니다.
* 방 생성 단계에서 `OppositeIndex`를 통해 역방향 연결을 즉시 처리하여 고립된 방이 없는 경로 무결성을 보장했습니다.

**3. 프레임 방어를 위한 렌더링 및 CPU 최적화**
* **동적 레벨 스트리밍 (VRAM 최적화):** 플레이어 좌표 기준 **맨해튼 거리(Manhattan Distance)** 가 1 이하인 인접 방만 렌더링(`SetShouldBeVisible`)하여 대규모 던전의 Draw Call 낭비를 억제했습니다.
* **이벤트 주도 초기화 및 캐싱 (CPU 최적화):** `OnLevelShown` 델리게이트를 활용해 로드 완료된 방만 초기화하고, 런타임에 반복되는 무거운 검색(`ActorHasTag`) 연산을 제거하기 위해 필요한 액터 포인터를 방 생성 시점에 미리 캐싱했습니다.
<img width="1387" height="752" alt="image" src="https://github.com/user-attachments/assets/e540e50e-c73c-4178-a147-c1498be18d69" />
플레이어가 있는 방을 기준으로 한칸의 방만 나타나게 하였습니다. 

**4. 기획적 의도를 반영한 미니맵 시스템 (전략적 동선 유도)**
* **설계 의도:** BFS 기반 맵 특성상 시작점과 보스방이 직통으로 연결될 수 있는 구조적 특징이 있습니다. 이를 보완하기 위해 억지로 문을 잠그거나(Key-Lock) 미니맵을 가리는(Fog of War) 대신, **모든 방의 위치를 투명하게 공개하는 방식**을 택했습니다.
* **UX 최적화:** 보스의 초기 스탯을 매우 강력하게 설정하여, 유저가 미니맵을 보고 "보스방에 바로 갈 수 있지만, 먼저 보물방(Dead End)에 들러 시너지 아이템을 파밍하고 성장해야겠다"고 스스로 전략적인 동선을 짜도록 레벨 디자인과 시스템을 융합했습니다.

<br>

### 🔄 [캐릭터 태그] GAS 기반 실시간 캐릭터 스왑 및 시너지 연계 <a name="character-tag-system"></a>
플레이어가 여러 캐릭터를 실시간으로 교체(태그)하며 싸우는 시스템입니다. 교체 플레이 자체에 전략적 의미를 부여하기 위해 **'시너지 어택(Synergy Attack)'** 기능을 더했습니다.
> <div align="center"> 
  <i>"전사로 적에게 표식을 부여한 뒤, 마법사로 실시간 스왑하여 폭발 데미지 / 마법사로 표식 부여한 뒤 전사로 스왑하여 추가타 발생"</i> 
</div>

![SProject - Unreal Editor 2026-03-12 15-59-33](https://github.com/user-attachments/assets/fc98edba-1e68-43d6-a9d3-c1c01bbf62af)  ![SProject - Unreal Editor 2026-03-12 16-01-37](https://github.com/user-attachments/assets/e0778c49-58d9-4e2c-a27c-a8e5c8c9e757)


**1. 조작감이 끊기지 않는 런타임 태그 시스템**
* 이동 중에 태그해도 물리적 흐름이 끊기지 않도록, 이전 캐릭터의 이동 속도(Velocity)를 새로 생성된 캐릭터의 `CharacterMovement`에 그대로 넘겨주어 공중이나 질주 중에도 모멘텀이 이어지게 디테일을 살렸습니다. [[📄캐릭터 스왑 및 속도 유지 로직]](https://github.com/hauras/S_Project/blob/main/Source/SProject/Private/Controller/SPlayerController.cpp#L71-L110)

**2. GAS 태그(Gameplay Tag)를 활용한 시너지 판정**
* 전사와 마법사의 스킬 연계를 하드코딩으로 구현하지 않고, 언리얼 GAS의 **Gameplay Tag**를 적극 활용했습니다. [[📄시너지 표식 및 데미지 판정 로직]](💡SDamageExecution.cpp_또는_관련로직_링크_삽입)
* 전사로 적중 시 대상에게 '시너지 표식' 태그를 남기고, 마법사로 교체해 공격할 때 대상에게 해당 태그가 있는지 검사하여 폭발 데미지를 적용하는 유연한 구조를 만들었습니다.

<br>

### ⚔️ [전투 시스템] 유연한 확장성을 고려한 GAS 코어 아키텍처 설계 <a name="combat-system"></a>
새로운 스킬과 몬스터가 추가될 때마다 코드를 수정하는 하드코딩 방식을 탈피하고, 철저하게 **데이터 드리븐(Data-Driven) 및 객체 지향적 구조**로 전투 시스템을 설계했습니다.

> **💡 [스킬 몽타주 움짤 삽입 추천]**
> * **추천 장면:** 투사체 발사, 블랙홀, 회피, 빔 공격 등 직접 깎으신 여러 스킬들이 1~2초 간격으로 빠르게 전환되며 지나가는 스피디한 움짤 딱 1개

**1. Base 클래스화를 통한 스킬 모듈화**
* `GA_AttackBase`, `GA_ProjectileBase` 등 스킬의 형태(근접, 투사체, 광역 등)에 따라 부모 클래스를 세분화하여 공통 로직(몽타주 재생, 타격 판정 등)을 캡슐화했습니다. [[📄스킬 Base 클래스 설계]](💡GA_AttackBase.h/cpp_링크_삽입)
* 블루프린트에서는 부모 클래스를 상속받아 데이터만 할당하면 즉시 새로운 스킬이 완성되는 파이프라인을 구축했습니다.

<br>

## 4. 🛠️ 문제 해결 (Troubleshooting) <a name="troubleshooting-eternal-return"></a>

### 1. 레벨 전환 및 태그 시 캐릭터 데이터 유실 및 동기화 이슈 해결 <a name="deferred-rendering"></a>
* **🔴 문제 상황:** 실시간 캐릭터 태그 시 기존 캐릭터(Pawn) 액터를 파괴하고 새로 스폰하는 방식을 사용했는데, 이때 캐릭터가 지닌 체력 및 쿨타임 데이터가 날아가거나, 태그 직후 체력이 풀피로 가득 차버리는 버그가 발생했습니다.
* **🔍 원인 분석:** 1. `AbilitySystemComponent(ASC)`가 폰에 부착되어 있어, 폰 파괴 시 생명주기가 같이 끝나는 것이 첫 번째 원인이었습니다.
  2. 스왑 후 새 캐릭터에 빙의될 때마다 `PossessedBy`가 재호출되며, `InitializeDefaultAttributes` 로직이 실행되어 어트리뷰트가 강제로 초기화되는 것이 두 번째 원인이었습니다.
* **🟢 해결 방법 (PlayerState 이관 및 Initialization Flag 적용):**
  * ASC와 `AttributeSet`의 소유권(Owner)을 폰이 아닌 **PlayerState**로 이관하여, 폰의 파괴와 무관하게 데이터가 보존되도록 생명주기를 전면 분리했습니다. 
  * `PlayerState` 내부에 `bAttributesInitialized` 플래그를 추가하여, 최초 접속 시에만 `GameInstance`에서 데이터를 로드해 초기화하고, 이후 캐릭터 태그 과정에서는 어트리뷰트 초기화 로직을 건너뛰도록 제어 흐름을 수정했습니다. [[📄교체 로직]](https://github.com/hauras/S_Project/blob/main/Source/SProject/Private/Character/PlayerCharacter.cpp#L37-L61)

* **✨ 결과:** 폰이 수시로 파괴되고 스폰되는 멀티플레이어 환경에서도 플레이어의 상태 데이터가 완벽하게 보존되며, 데이터 꼬임이나 의도치 않은 회복 현상을 원천 차단했습니다.
