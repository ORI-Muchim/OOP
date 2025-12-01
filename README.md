# 주식 거래 시뮬레이션 프로그램

## 프로젝트 구조

단일 파일: `main.cpp`

---

## 팀원 A 구현 완료

### Stock 클래스 (26-66 라인)

| 멤버/함수 | 설명 |
|-----------|------|
| `name`, `currentPrice`, `previousPrice` | 주식 기본 정보 |
| `volume`, `averageVolume` | 거래량 및 EMA 평균 |
| `Stock(name, initPrice, initVolume)` | 생성자 |
| `updatePrice(price, volume)` | 가격/거래량 업데이트 |
| `getFluctuationRate()` | 변동률 계산 |
| `setVolume(volume)` | 거래량 설정 + EMA 적용 |
| `getVolumeRatio()` | 거래량 비율 |

### Transaction 클래스 (68-109 라인)

| 멤버/함수 | 설명 |
|-----------|------|
| `stockName`, `type`, `quantity`, `price` | 거래 기본 정보 |
| `totalAmount`, `isGapTrade`, `timestamp` | 추가 정보 |
| `printLog()` | 거래 내역 출력 |
| `getType()`, `setType()` | 타입 getter/setter |
| `setGapTradeFlag()` | 갭 트레이드 플래그 설정 |

### Market 클래스 (111-224 라인)

| 멤버/함수 | 설명 |
|-----------|------|
| `allStocks`, `transactionHistory` | 주식 목록, 거래 내역 |
| `~Market()` | 소멸자 (Stock 메모리 해제) |
| `addStock()` | 주식 추가 |
| `simulateFluctuation()` | 시장 변동 시뮬레이션 (95%/5%) |
| `getStockByName()` | 이름으로 주식 검색 |
| `getMarketVolatility()` | 시장 변동성 (표준편차) |
| `getAverageVolume()` | 시장 평균 거래량 |
| `addTransaction()` | 거래 내역 추가 |
| `printMarketStatus()` | 시장 현황 출력 |
| `printTransactionHistory()` | 거래 내역 출력 |

### 핵심 알고리즘

```
시장 변동 시뮬레이션:
- 95% 확률: -2% ~ +2% (정상)
- 5% 확률: -10% ~ -30% (블랙스완)

EMA 평균 거래량:
averageVolume = averageVolume × 0.95 + newVolume × 0.05

시장 변동성:
volatility = sqrt(Σ(변동률 - 평균)² / N)
```

---

## 팀원 B 구현 필요

### Position 구조체 (229-236 라인)

```cpp
struct Position {
    Stock* stock;
    int quantity;
    double avgBuyPrice;
    time_t buyTimestamp;

    // TODO: 생성자 구현
};
```

### Trader 클래스 (304-327 라인)

| 함수 | 설명 |
|------|------|
| `Trader(name, initialCash)` | 생성자 |
| `buy(stockName, qty, market)` | 일반 매수 |
| `sell(stockName, qty, market)` | 일반 매도 |
| `autoTrade(market)` | 자동 거래 |
| `buyGapPosition(stockName, qty, market)` | 갭 포지션 매수 |
| `closeGapPosition(stockName, market)` | 갭 포지션 청산 |
| `closeAllGapPositions(market)` | 전체 갭 포지션 청산 |
| `hasOpenGapPositions()` | 갭 포지션 보유 여부 |
| `getPositionQuantity(stockName)` | 보유 수량 조회 |
| `getTotalAssetValue(market)` | 총 자산 계산 |
| `getGapPositionProfit(stockName)` | 갭 포지션 수익률 |

---

## 팀원 C 구현 필요

### MarketState 추상 클래스 (241-246 라인)

이미 선언 완료. 구현 불필요.

### NormalState 클래스 (251-259 라인)

| 함수 | 설명 |
|------|------|
| `handle(trader, market)` | 보수적 리밸런싱 |
| `checkGapPositionRecovery(trader, market)` | 10% 수익 달성 시 청산 |

### DisruptedState 클래스 (264-276 라인)

| 함수 | 설명 |
|------|------|
| `handle(trader, market)` | 역발상 전략 실행 |
| `executeGapTrading(trader, market)` | 갭 트레이딩 |
| `executeContrarianBuy(trader, market)` | 역발상 매수 |
| `selectBestTarget(market, gapPositions)` | 최적 매수 대상 선정 |
| `calculateInvestAmount(cash, targets)` | 투자 금액 계산 |

### DisruptionDetector 클래스 (281-299 라인)

| 함수 | 설명 |
|------|------|
| `DisruptionDetector()` | 생성자 |
| `analyzeMarket(market)` | 시장 분석 + 상태 전환 |
| `changeState(newState)` | 상태 변경 |
| `executeStrategy(trader, market)` | 전략 실행 |
| `calculateScore(market)` | 위험도 점수 계산 |
| `checkRecovery()` | 회복 여부 확인 |

### 핵심 알고리즘

```
위험도 계산:
Score = (거래량/평균거래량) × 변동성 × 1000

상태 전환:
- Score > 70 → DisruptedState
- Score < 50 → NormalState

역발상 매수 조건:
- 하락률 < -15%
- 거래량 > 평균 × 200%
- 갭 포지션 < 3개
```

---

## 컴파일 및 실행

```bash
g++ -std=c++14 -o main main.cpp
./main
```

---

## 메모리 관리 주의사항

| 클래스 | 포인터 | 관리 |
|--------|--------|------|
| Market → Stock | `vector<Stock*>` | Market 소멸자에서 delete |
| Position → Stock | `Stock*` | 약한 참조 (delete 금지) |
| Trader → DisruptionDetector | `unique_ptr` | 자동 해제 |
| DisruptionDetector → MarketState | `unique_ptr` | 자동 해제 |
