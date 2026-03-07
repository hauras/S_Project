# 🗺️ 하정빈 포트폴리오
> "프레임의 낭만, 끝까지 쫓다."
> 
> <b>C++, 언리얼 기반 게임 클라이언트 프로그래머. <br>
>
> 1 프레임의 성능 최적화를 위해 끝까지 파고드는 개발자 하정빈입니다. "동작하는 코드"를 넘어 "성능과 구조가 아름다운 코드"를 지향하며, 엔진 레벨의 깊이 있는 이해를 바탕으로 문제를 해결합니다.

---

## 목차<a name="table-of-contents"></a>

<table>
  <thead>
    <tr>
      <th>🎮 GAS 프로젝트 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>

  
  </thead>
<tbody>
    <tr>
      <td valign="top">
        <br>
        <b><a href="#eternal-return-main">🎮 프로젝트 메인</a></b><br>
        <b><a href="#-게임-개요">📖 게임 개요</a></b><br>
        <b><a href="#-학습-목표-및-달성">📌 학습 목표 및 달성</a></b><br>
        <b><a href="#-주요-개발">🔨 주요 개발</a></b><br>
        <b><a href="#troubleshooting-eternal-return">🛠️ 문제 해결</a></b><br>
        &nbsp;&nbsp; └ <a href="#deferred-rendering"> 레벨 전환 시 캐릭터 태그 데이터 유실 및 동기화 이슈 </a><br>
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
| 🎯 **장르** | 멀티,   |
| ⏱️ **개발 기간** | 2개월 |
| 👥 **개발 인원** | 1인 (프로그래머로 참여) |
| 🛠️ **개발 환경** | C++, UE5 |
| 🎬 **시연 영상** | [YouTube 바로가기]() |
| 💾 **GitHub** | [소스코드]() |

## 📑 프로젝트 목차<a name="toc-eternal"></a>

**1. 📖 [게임 개요](#-게임-개요)**

**2. 📌 [학습 목표 및 달성](#-학습-목표-및-달성)**

**3. 🔨 [주요 개발 기능](#주요개발-eternal-return)** <br>
&nbsp;&nbsp; └ [스팀 연동](#deferred-rendering)<br>
&nbsp;&nbsp; └ [멀티플레이](#gpu-instancing)<br>
&nbsp;&nbsp; └ [캐릭터 태그 시스템](#quad-tree)<br>
&nbsp;&nbsp; └ [던전 생성](#navmesh)<br>
&nbsp;&nbsp; └ [몬스터 AI 시스템](#fsm-to-bt-feature)


<br>

### 🏰 [던전 생성] BFS 기반 절차적 던전 생성 및 최적화 <a name="dungeon-generation"></a>
멀티플레이어 환경에 맞춰 동적으로 로그라이크 맵을 생성하고, 대규모 맵에서도 메모리와 프레임을 방어할 수 있도록 구조적 최적화를 진행했습니다.

**1. 무작위 너비 우선 탐색을 활용한 레벨 디자인**

* **도입 배경:** 멀티플레이 시 유저들의 동선이 과도하게 멀어지는 DFS(선형 구조)의 단점을 보완하기 위해, 시작점 기준 사방으로 밀집되어 퍼져나가는 BFS 알고리즘을 채택했습니다. [[📄방 생성 로직]](https://github.com/hauras/S_Project/blob/main/Source/SProject/Private/Actor/Map/DungeonGenerator.cpp#L61-L109)
* **Depth 기반 밸런싱:** 노드별 최단 거리(`Depth`)를 추적하여, 가장 먼 `Max Depth` 노드에 **보스 방**을 배치하고, 통로가 1개인 막다른 길(Dead End) 노드에 확률적으로 **보물 방**을 배치해 탐험의 서사를 수학적으로 제어했습니다.

**2. 비트마스크(Bitmask)를 이용한 방 연결 및 데이터 경량화**
* 방 4면의 문(Gate) 연결 상태를 `int32`의 단 **4비트(1:N, 2:S, 4:W, 8:E)** 로 압축했습니다. 
* 이를 통해 멀티플레이어 환경에서 던전 레이아웃 데이터(`DungeonLayout`) 전체를 복제(Replication)할 때 발생하는 **네트워크 대역폭을 획기적으로 절약**했습니다.
* 방 생성 단계에서 `OppositeIndex`를 통해 역방향 연결을 즉시 처리하여 고립된 방이 없는 경로 무결성을 보장했습니다.

**3. 프레임 방어를 위한 렌더링 및 CPU 최적화**
* **동적 레벨 스트리밍 (VRAM 최적화):** 플레이어의 월드 좌표를 2D 그리드로 실시간 변환 후, **맨해튼 거리(Manhattan Distance)** 가 1 이하인 인접 방만 렌더링(`SetShouldBeVisible`)하여 대규모 던전의 Draw Call과 VRAM 낭비를 억제했습니다.
* **이벤트 주도 초기화 및 캐싱 (CPU 최적화):** 레벨 초기화 시 발생하는 병목을 해결하기 위해, 언리얼 엔진의 `OnLevelShown` 델리게이트를 활용해 로드가 완료된 방만 조기 종료 패턴으로 초기화했습니다. 또한, 런타임에 반복적으로 발생하는 무거운 검색(`ActorHasTag`) 연산을 제거하기 위해 필요한 액터 포인터를 방 생성 시점에 배열로 미리 캐싱하여 CPU 점유율을 대폭 낮췄습니다.

<br>
**3. 🛠️ [문제 해결 (Troubleshooting)](#troubleshooting-eternal-return)** <br>



