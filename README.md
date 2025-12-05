# 주식 거래 프로그램 (Stock Trading Program)

객체지향 프로그래밍 과제 - State Pattern 기반 자동매매 시스템

---

## 1. 프로젝트 개요

본 프로젝트는 **State Pattern**을 활용한 주식 자동매매 시스템입니다. 시장 상황에 따라 `NormalState`와 `DisruptedState`로 상태가 전환되며, 각 상태에서 다른 투자 전략을 실행합니다.

### 핵심 기능
- 시장 변동 시뮬레이션 (95% 정상, 5% 블랙스완)
- 시장 위험도 자동 분석 및 상태 전환
- 역발상 매수 전략 (폭락장 대응)
- 갭 포지션 자동 청산 (목표 수익률 도달 시)

---

## 2. 클래스 구조

### 2.1 클래스 다이어그램 요약

```
┌─────────────────────────────────────────────────────────────────┐
│                           Market                                 │
│  ◆── Stock (Composition)                                        │
│  ◇── Transaction (Aggregation)                                  │
└─────────────────────────────────────────────────────────────────┘
                              ↑ Dependency
┌─────────────────────────────────────────────────────────────────┐
│                           Trader                                 │
│  ◇── Position (portfolio, gapPositions)                         │
│  ◆── DisruptionDetector (unique_ptr)                            │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     DisruptionDetector                           │
│  ◆── MarketState (unique_ptr)                                   │
└─────────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌──────────────────────┐        ┌──────────────────────┐
│     NormalState      │        │    DisruptedState    │
│  ──▷ MarketState     │        │  ──▷ MarketState     │
└──────────────────────┘        └──────────────────────┘
```

### 2.2 클래스별 역할

| 클래스 | 역할 |
|--------|------|
| **Stock** | 개별 주식의 가격/거래량 정보 관리 |
| **Transaction** | 거래 내역 기록 및 출력 |
| **Market** | 주식 시장 관리, 가격 변동 시뮬레이션 |
| **Position** | 보유 주식의 수량과 매수 정보 (구조체) |
| **Trader** | 투자자 포트폴리오 관리 및 매매 실행 |
| **MarketState** | 시장 상태 인터페이스 (추상 클래스) |
| **NormalState** | 정상 시장 상태 전략 |
| **DisruptedState** | 폭락 시장 상태 전략 |
| **DisruptionDetector** | 시장 위험도 분석 및 상태 전환 관리 |

---

## 3. 핵심 알고리즘

### 3.1 시장 변동 시뮬레이션

```cpp
if (randomValue < 0.95) {
    // 95% 확률: 정상 등락
    changeRate = random * 0.04 - 0.02;  // -2% ~ +2%
} else {
    // 5% 확률: 블랙스완 이벤트
    changeRate = random * (-0.20) - 0.10;  // -10% ~ -30%
}
```

### 3.2 평균 거래량 계산 (EMA)

```cpp
averageVolume = averageVolume × 0.95 + newVolume × 0.05
```

- 지수이동평균(Exponential Moving Average) 방식
- 별도 이력 저장 없이 누적 평균으로 관리

### 3.3 시장 변동성 계산

```cpp
volatility = √(Σ(각 종목 변동률 - 평균 변동률)² / N)
```

- 전체 종목 변동률의 표준편차
- 반환값 범위: 0.0 ~ 1.0 (예: 0.05 = 5%)

### 3.4 위험도 점수 (Disruption Score)

```cpp
DisruptionScore = (현재거래량 / 평균거래량) × 변동성 × 1000
```

**상태 전환 기준:**
| 조건 | 결과 |
|------|------|
| Score > 70.0 | DisruptedState 전환 |
| Score < 50.0 | NormalState 복귀 |
| 50.0 ≤ Score ≤ 70.0 | 현재 상태 유지 |

**계산 예시:**
- 거래량 비율 2.5, 변동성 0.08 → Score = 2.5 × 0.08 × 1000 = **200** → Disrupted
- 거래량 비율 1.2, 변동성 0.03 → Score = 1.2 × 0.03 × 1000 = **36** → Normal

### 3.5 역발상 매수 전략 (Contrarian Buy)

**발동 조건:**
| 조건 | 값 |
|------|-----|
| 가격 하락률 | < -15% |
| 거래량 | > 평균 거래량 × 200% |
| 현재 갭 포지션 수 | < MAX_POSITIONS (3개) |

**매수 대상 선정 우선순위:**
1. 하락률 최대 (가장 많이 하락한 종목)
2. 거래량 비율 최대 (동일 하락률 시)
3. 기존 미보유 (이미 갭 포지션 보유 종목 제외)

**투자 금액 계산:**
```cpp
종목당 투자금액 = (현금 × 0.50) / 조건충족_종목수
```

### 3.6 갭 포지션 청산 조건

| 조건 | 내용 |
|------|------|
| 목표 수익 달성 | 수익률 ≥ 10% 시 해당 포지션 청산 |
| 손절 | 수익률 ≤ -20% 시 손절 (구현 완료) |

**수익률 계산:**
```cpp
profitRate = (현재가 - 평균매수가) / 평균매수가
```

---

## 4. 명세서 일치 검증 결과

### 4.1 클래스별 검증

| 클래스 | 멤버변수 | 멤버함수 | 일치 |
|--------|----------|----------|------|
| Stock | 5/5 | 7/7 | O |
| Market | 2/2 | 5/5 | O |
| Transaction | 7/7 | 2/2 | O |
| Position | 4/4 | - | O |
| Trader | 5/5 | 10/10 | O |
| DisruptionDetector | 5/5 | 7/7 | O |
| MarketState | - | 3/3 | O |
| NormalState | 1/1 | 3/3 | O |
| DisruptedState | 2/2 | 5/5 | O |

### 4.2 알고리즘 검증

| 알고리즘 | 명세서 섹션 | 구현 상태 |
|----------|-------------|-----------|
| 시장 변동 시뮬레이션 | 3.1 | O |
| EMA 평균 거래량 | 3.2 | O |
| 시장 변동성 계산 | 3.3 | O |
| 위험도 계산 공식 | 3.4 | O |
| 역발상 매수 전략 | 3.5 | O |
| 갭 포지션 청산 | 3.6 | O |

---

## 5. 명세서 모호성 및 해석

### 5.1 "백분율" 해석 문제

**명세서 섹션 1.2:**
> `getFluctuationRate()` - 가격 변동률 계산 **(백분율)**

**문제점:**
"백분율"이라는 표현이 두 가지로 해석 가능:
1. 5%를 `5.0`으로 반환 (백분율 수치)
2. 5%를 `0.05`로 반환 (비율, 백분율을 표현)

**해결:**
명세서 섹션 3.3과 3.4의 예시를 참조하여 해석:
- 섹션 3.3: "0.0 ~ 1.0 사이의 실수값 (예: 0.05 = 5% 변동성)"
- 섹션 3.4: "변동성 = 0.08 (8%)"

→ **비율 형식(0.05 = 5%)으로 구현**

```cpp
// Stock::getFluctuationRate()
double getFluctuationRate() const {
    if (previousPrice == 0) return 0.0;
    return (currentPrice - previousPrice) / previousPrice;  // 비율 반환
}
```

### 5.2 Transaction type 문자열

**명세서:**
> type: string - 매수/매도 구분 ("BUY"/"SELL")

**실제 구현:**
한국어 UI를 위해 `"매수"`, `"청산"`, `"갭 매수"`, `"갭 청산"` 사용

→ 영어 대신 한국어 문자열로 의도적 변경

### 5.3 명세서에 없지만 필요한 함수들

코드 동작을 위해 추가된 접근자(getter) 함수들:

| 클래스 | 함수 | 필요 이유 |
|--------|------|-----------|
| Stock | `getName()` | 종목명 조회 |
| Stock | `getCurrentPrice()` | 현재가 조회 |
| Stock | `getVolume()` | 거래량 조회 |
| Trader | `getCash()` | `calculateInvestAmount()`에서 사용 |
| Trader | `getGapPositions()` | State 클래스에서 포지션 순회 |
| Trader | `getGapPositionCount()` | MAX_POSITIONS 검사 |
| DisruptionDetector | `getCurrentStateName()` | 상태 전환 판단 |
| Market | `getAllStocks()` | `selectBestTarget()`에서 사용 |

→ 이들은 명세서에 명시되지 않았지만, **논리적으로 필수적인 구현**

### 5.4 손절 기능 (구현 완료)

**명세서 섹션 3.6:**
> 손절 (선택) - 수익률 < -20% 시 손절 **(구현 선택사항)**

→ **선택사항이지만 구현 완료**

```cpp
// NormalState 클래스
static constexpr double STOP_LOSS_RATE = -0.20;  // 손절 기준 -20%

// checkGapPositionRecovery() 에서 체크
if (profitRate <= STOP_LOSS_RATE) {
    // 손절 수행
}
```

**동작 예시:**
```
[NormalState] 갭 포지션 손절: SK하이닉스 (수익률: -28.75%) 손절 수행
```

---

## 6. 컴파일 및 실행

### 6.1 컴파일

```bash
g++ -std=c++17 -Wall -o main main.cpp
```

### 6.2 실행

```bash
./main
```

### 6.3 필요 환경

- C++17 이상
- 표준 라이브러리만 사용 (외부 의존성 없음)

---

## 7. 실행 예시

```
==============================
     [주식 거래 프로그램]
==============================

[초기 상태]
====== 시장 현황 ======
시장 변동성: 0.0000%
평균 거래량: 560000

[종목별 현황]
삼성전자 | 현재가: 70000.00 | 변동성: 0.00% | 거래량 비율: 100.00%
SK하이닉스 | 현재가: 130000.00 | 변동성: 0.00% | 거래량 비율: 100.00%
...

--- [Day 6]---
카카오 | 현재가: 41623.61 | 변동성: -19.65% | 거래량 비율: 251.08%

[DisruptionDetector] 상태 전환 -> Disrupted
[자동매매] 현재 상태: Disrupted | DisruptionScore: 190.48
[DisruptedState] 역발상 매수: 카카오 수량: 120 가격: 41623.61
```

**동작 설명:**
1. Day 6에서 카카오가 -19.65% 급락 (블랙스완 이벤트)
2. 거래량 251% 폭증 → DisruptionScore 190.48
3. Score > 70 → **Disrupted 상태 전환**
4. 역발상 매수 조건 충족 (하락률 < -15%, 거래량 > 200%)
5. 카카오 120주 갭 매수 실행

---

## 8. 파일 구조

```
OOP/
├── main.cpp                      # 전체 구현 코드
├── Specification document.pdf    # 설계 명세서
├── UML for Object Oriented Programming.pdf  # UML 다이어그램
└── README.md                     # 본 문서
```

---

## 9. 팀 역할 분담 (명세서 기준)

| 팀원 | 담당 클래스 |
|------|-------------|
| 팀원 A | Stock, Transaction, Market |
| 팀원 B | Position, Trader |
| 팀원 C | MarketState, NormalState, DisruptedState, DisruptionDetector |

---

## 10. 설계 패턴

### State Pattern 적용

```
┌─────────────────┐
│  MarketState    │  ← 추상 인터페이스
│  (Abstract)     │
├─────────────────┤
│ +handle()       │
│ +getStateName() │
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
    ▼         ▼
┌────────┐  ┌────────────┐
│Normal  │  │ Disrupted  │
│State   │  │ State      │
├────────┤  ├────────────┤
│갭 포지션│  │역발상 매수  │
│청산 검사│  │갭 트레이딩  │
└────────┘  └────────────┘
```

**장점:**
- 상태별 로직 분리로 코드 가독성 향상
- 새로운 상태 추가 시 기존 코드 수정 최소화
- 상태 전환 로직의 명확한 관리

---

## 11. 메모리 관리

| 클래스 | 포인터 타입 | 관리 방식 |
|--------|-------------|-----------|
| Market → Stock | `vector<Stock*>` | Market 소멸자에서 delete |
| Trader → DisruptionDetector | `unique_ptr` | 자동 해제 |
| DisruptionDetector → MarketState | `unique_ptr` | changeState() 시 자동 교체 |
| Position → Stock | `Stock*` (약한 참조) | 해제 책임 없음 |

**주의:** Position 내의 Stock 포인터는 Market이 소유한 객체를 참조만 하므로, Position에서 delete하면 안됨

---

## 12. 상수값 정리

| 상수 | 값 | 위치 | 설명 |
|------|-----|------|------|
| THRESHOLD | 70.0 | DisruptionDetector | 폭락 판단 임계값 |
| RECOVERY_THRESHOLD | 50.0 | DisruptionDetector | 회복 판단 임계값 |
| TARGET_PROFIT_RATE | 0.10 | NormalState | 갭 포지션 목표 수익률 (10%) |
| STOP_LOSS_RATE | -0.20 | NormalState | 손절 기준 (-20%) |
| INVESTMENT_RATIO | 0.50 | DisruptedState | 투자 비율 (현금의 50%) |
| MAX_POSITIONS | 3 | DisruptedState | 최대 동시 갭 포지션 수 |

---

## 13. 클래스 간 관계 상세

| 관계 유형 | 표기 | 클래스 | 설명 |
|----------|------|--------|------|
| Composition | ◆── | Market → Stock | 시장이 주식을 소유 (강한 생명주기) |
| Aggregation | ◇── | Market → Transaction | 시장이 거래 내역 기록 (약한 관계) |
| Aggregation | ◇── | Trader → Position | 투자자가 포지션 보유 |
| Composition | ◆── | Trader → DisruptionDetector | 투자자가 분석기 소유 |
| Composition | ◆── | DisruptionDetector → MarketState | 분석기가 상태 소유 |
| Dependency | ..> | Trader → Market | 거래 시 시장 정보 사용 |
| Inheritance | ──▷ | NormalState → MarketState | 상속 |
| Inheritance | ──▷ | DisruptedState → MarketState | 상속 |

---

## 14. 참고 사항

- 난수 시드는 `time(nullptr)` 사용으로 실행마다 다른 결과
- 초기 자본금: 1,000만원
- 시뮬레이션: 10일간 자동매매
- 5개 종목: 삼성전자, SK하이닉스, 네이버, 카카오, LG에너지솔루션

---

## 15. 주요 코드 위치 (라인 번호)

| 클래스/함수 | 시작 라인 |
|-------------|-----------|
| Stock 클래스 | 28 |
| Transaction 클래스 | 77 |
| Market 클래스 | 125 |
| Position 구조체 | 264 |
| Trader 클래스 | 280 |
| MarketState 클래스 | 524 |
| NormalState 클래스 | 534 |
| DisruptedState 클래스 | 569 |
| DisruptionDetector 클래스 | 678 |
| Trader::autoTrade() 구현 | 774 |
| main() 함수 | 791 |
