#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <iomanip>
#include <algorithm>

using namespace std;

// 전방 선언들
class Stock;
class Transaction;
class Market;
struct Position;
class Trader;
class MarketState;
class NormalState;
class DisruptedState;
class DisruptionDetector;

// ============================================================
// 팀원 A: Stock 클래스
// ============================================================
class Stock {
private:
    string name;
    double currentPrice;
    double previousPrice;
    double volume;
    double averageVolume;

public:
    Stock(const string& name, double initPrice, double initVolume)
        : name(name), currentPrice(initPrice), previousPrice(initPrice),
        volume(initVolume), averageVolume(initVolume) {
    }

    void updatePrice(double newPrice) {
        previousPrice = currentPrice;
        currentPrice = newPrice;
    }

    double getFluctuationRate() const {
        if (previousPrice == 0) return 0.0;
        return (currentPrice - previousPrice) / previousPrice;  // 비율 반환 (0.05 = 5%)
    }

    void setVolume(double newVolume) {
        volume = newVolume;
        // EMA 평균 거래량
        averageVolume = averageVolume * 0.95 + newVolume * 0.05;
    }

    double getAverageVolume() const { return averageVolume; }

    double getVolumeRatio() const {
        if (averageVolume == 0) return 0.0;
        return volume / averageVolume;
    }

    string getName() const { return name; }

    double getCurrentPrice() const { return currentPrice; }

    double getPreviousPrice() const { return previousPrice; }

    double getVolume() const { return volume; }
};

// ============================================================
// 팀원 A: Transaction 클래스
// ============================================================
class Transaction {
private:
    string stockName;
    string type;
    int quantity;
    double price;
    double totalAmount;
    bool isGapTrade;
    time_t timestamp;
    double profitRate;      // 청산 시 수익률 (매수 시에는 0)
    double profitAmount;    // 청산 시 손익 금액

public:
    Transaction(const string& stockName, const string& type, int quantity, double price, bool isGapTrade = false)
        : stockName(stockName), type(type), quantity(quantity), price(price), isGapTrade(isGapTrade),
          profitRate(0.0), profitAmount(0.0) {
        totalAmount = price * quantity;
        timestamp = time(nullptr);
    }

    void printLog() const {
        cout << "[" << put_time(localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << "] "
            << (isGapTrade ? "[갭] " : "")
            << type << " " << stockName
            << " | 수량: " << quantity
            << " | 가격: " << fixed << setprecision(2) << price
            << " | 총액: " << totalAmount;

        // 청산 거래인 경우 수익률 표시
        if (type.find("청산") != string::npos && (profitRate != 0.0 || profitAmount != 0.0)) {
            cout << " | 수익률: " << fixed << setprecision(2) << (profitRate * 100) << "%"
                 << " | 손익: " << fixed << setprecision(0) << profitAmount;
        }
        cout << endl;
    }

    void setProfitInfo(double rate, double amount) {
        profitRate = rate;
        profitAmount = amount;
    }

    string getType() const { return type; }

    void setType(const string& t) { type = t; }

    void setGapTradeFlag(bool flag) { isGapTrade = flag; }

    string getStockName() const { return stockName; }

    int getQuantity() const { return quantity; }

    double getPrice() const { return price; }

    double getTotalAmount() const { return totalAmount; }

    bool getIsGapTrade() const { return isGapTrade; }

    time_t getTimestamp() const { return timestamp; }
};

// ============================================================
// 팀원 A: Market 클래스
// ============================================================
class Market {
private:
    vector<Stock*> allStocks;
    vector<Transaction> transactionHistory;

public:
    Market() {}

    ~Market() {
        for (Stock* stock : allStocks) {
            delete stock;
        }

        allStocks.clear();
    }

    void addStock(Stock* stock) {
        allStocks.push_back(stock);
    }

    void simulateFluctuation() {
        for (Stock* stock : allStocks) {
			double randomValue = (double)rand() / RAND_MAX;  // 0.0 ~ 1.0
            double changeRate;
            double volumeMultiplier;

			// 시장 변동 시뮬레이션
            if (randomValue < 0.95) {
				// 95% 확률로 정상
				changeRate = ((double)rand() / RAND_MAX) * 0.04 - 0.02;  // -2% ~ +2%
                volumeMultiplier = 0.8 + ((double)rand() / RAND_MAX) * 0.4;
            }

            else {
				// 5% 확률로 블랙스완
				changeRate = ((double)rand() / RAND_MAX) * (-0.20) - 0.10;  // -10% ~ -30%
                volumeMultiplier = 2.0 + ((double)rand() / RAND_MAX) * 2.0;
            }

            double newPrice = stock->getCurrentPrice() * (1.0 + changeRate);

            if (newPrice < 0.01) newPrice = 0.01;

            double newVolume = stock->getAverageVolume() * volumeMultiplier;
            stock->updatePrice(newPrice);
            stock->setVolume(newVolume);
        }
    }

    Stock* getStockByName(const string& name) const {
        for (Stock* stock : allStocks) {
            if (stock->getName() == name) {
                return stock;
            }
        }

        return nullptr;
    }

    double getMarketVolatility() const {
        if (allStocks.empty()) return 0.0;

        double sum = 0.0;

        for (Stock* stock : allStocks) {
            sum += stock->getFluctuationRate();
        }

        double mean = sum / allStocks.size();

        double variance = 0.0;

        for (Stock* stock : allStocks) {
            double diff = stock->getFluctuationRate() - mean;
            variance += diff * diff;
        }

        variance /= allStocks.size();

        // 시장 변동성
        return sqrt(variance);  // 0.0 ~ 1.0
    }

    double getAverageVolume() const {
        if (allStocks.empty()) return 0.0;

        double sum = 0.0;

        for (Stock* stock : allStocks) {
            sum += stock->getVolume();
        }

        return sum / allStocks.size();
    }

    void addTransaction(const Transaction& t) {
        transactionHistory.push_back(t);
    }

    void printMarketStatus() const {
        cout << "\n====== 시장 현황 ======" << endl;

        cout << "시장 변동성: " << fixed << setprecision(4)
            << (getMarketVolatility() * 100) << "%" << endl;

        cout << "평균 거래량: " << fixed << setprecision(0)
            << getAverageVolume() << endl;

        cout << "\n[종목별 현황]" << endl;

        for (Stock* stock : allStocks) {
            cout << stock->getName()
                << " | 현재가: " << fixed << setprecision(2) << stock->getCurrentPrice()
                << " | 변동성: " << fixed << setprecision(2) << (stock->getFluctuationRate() * 100) << "%"
                << " | 거래량 비율: " << fixed << setprecision(2) << (stock->getVolumeRatio() * 100) << "%"
                << " | 평균 거래량: " << (long long)stock->getAverageVolume()
                << endl;
        }

        cout << "=======================\n" << endl;
    }

    void printTransactionHistory() const {
        cout << "\n====== 거래 내역 ======" << endl;

        for (const Transaction& t : transactionHistory) {
            t.printLog();
        }

        cout << "=======================\n" << endl;
    }

    const vector<Stock*>& getAllStocks() const { return allStocks; }
    const vector<Transaction>& getTransactionHistory() const { return transactionHistory; }
};

// ============================================================
// 팀원 B: Position 구조체
// ============================================================
struct Position {
    Stock* stock;
    int quantity;
    double avgBuyPrice;
    time_t buyTimestamp;

    // 생성자
    Position(Stock* s = nullptr, int qty = 0, double price = 0.0)
        : stock(s), quantity(qty), avgBuyPrice(price) {
        buyTimestamp = time(nullptr);
    }
};

// ============================================================
// 팀원 B: Trader 클래스
// ============================================================
class Trader {
private:
    string username;
    double cash;
    map<string, Position> portfolio;
    map<string, Position> gapPositions;
    unique_ptr<DisruptionDetector> detector;
    
public:
    Trader(const string& name, double initialCash)
        : username(name), cash(initialCash) {
        detector = make_unique<DisruptionDetector>();
    }

    bool buy(const string& stockName, int qty, Market& m) {
        if (qty <= 0) return false;

        Stock* stock = m.getStockByName(stockName);

        if (!stock) return false;

        double price = stock->getCurrentPrice();
        double cost = price * qty;

        if (cost > cash) return false;

        // 거래 내역 기록
        Transaction t(stockName, "매수", qty, price, false);
        m.addTransaction(t);

        // 포트폴리오 업데이트 (가중 평균 단가)
        auto it = portfolio.find(stockName);

        if (it == portfolio.end()) {
            portfolio[stockName] = Position(stock, qty, price);
        }

        else {
            Position& pos = it->second;
            int newQty = pos.quantity + qty;
            double newAvg = (pos.avgBuyPrice * pos.quantity + price * qty) / newQty;
            pos.quantity = newQty;
            pos.avgBuyPrice = newAvg;
            pos.buyTimestamp = time(nullptr);
        }

        // 현금 차감
        cash -= cost;

        return true;
    }

    bool sell(const string& stockName, int qty, Market& m) {
        if (qty <= 0) return false;

        auto it = portfolio.find(stockName);

        if (it == portfolio.end()) return false;

        Position& pos = it->second;

        if (qty > pos.quantity) return false;

        Stock* stock = pos.stock ? pos.stock : m.getStockByName(stockName);

        if (!stock) return false;

        double price = stock->getCurrentPrice();

        // 거래 내역 기록
        Transaction t(stockName, "청산", qty, price, false);
        m.addTransaction(t);

        double revenue = price * qty;

        // 포지션 및 현금 업데이트
        pos.quantity -= qty;

        if (pos.quantity == 0) {
            portfolio.erase(it);
        }

        cash += revenue;

        return true;
    }

    void autoTrade(Market& m);  // DisruptionDetector 정의 후 구현

    bool buyGapPosition(const string& stockName, int qty, Market& m) {
        if (qty <= 0) return false;

        Stock* stock = m.getStockByName(stockName);

        if (!stock) return false;

        double price = stock->getCurrentPrice();
        double cost = price * qty;

        if (cost > cash) return false;

        auto it = gapPositions.find(stockName);

        if (it == gapPositions.end()) {
            gapPositions[stockName] = Position(stock, qty, price);
        }

        else {
            Position& pos = it->second;
            int newQty = pos.quantity + qty;
            double newAvg = (pos.avgBuyPrice * pos.quantity + price * qty) / newQty;
            pos.quantity = newQty;
            pos.avgBuyPrice = newAvg;
            pos.buyTimestamp = time(nullptr);
        }

        Transaction t(stockName, "갭 매수", qty, price, true);
        m.addTransaction(t);

        cash -= cost;

        return true;
    }

    void closeAllGapPositions(Market& m) {
        // 키를 복사해두고 순회 중 맵을 수정
        vector<string> keys;
        keys.reserve(gapPositions.size());

        for (const auto& kv : gapPositions) {
            keys.push_back(kv.first);
        }

        for (const string& name : keys) {
            closeGapPosition(name, m);
        }
    }

    bool closeGapPosition(const string& stockName, Market& m) {
        auto it = gapPositions.find(stockName);

        if (it == gapPositions.end()) return false;

        Position& pos = it->second;

        if (pos.quantity <= 0) return false;

        Stock* stock = pos.stock ? pos.stock : m.getStockByName(stockName);

        if (!stock) return false;

        int qty = pos.quantity;
        double currentPrice = stock->getCurrentPrice();
        double avgBuyPrice = pos.avgBuyPrice;
        double revenue = currentPrice * qty;

        // 수익률 계산
        double profitRate = (currentPrice - avgBuyPrice) / avgBuyPrice;
        double profitAmount = (currentPrice - avgBuyPrice) * qty;

        Transaction t(stockName, "갭 청산", qty, currentPrice, true);
        t.setProfitInfo(profitRate, profitAmount);
        m.addTransaction(t);
        gapPositions.erase(it);

        cash += revenue;

        return true;
    }

    bool hasOpenGapPositions() const {
        return !gapPositions.empty();
    }

    int getPositionQuantity(const string& stockName) const {
        int qty = 0;
        auto itNormal = portfolio.find(stockName);

        if (itNormal != portfolio.end()) {
            qty += itNormal->second.quantity;
        }

        auto itGap = gapPositions.find(stockName);

        if (itGap != gapPositions.end()) {
            qty += itGap->second.quantity;
        }

        return qty;
    }

    double getTotalAssetValue(Market& m) const {
        double total = cash;

        // 일반 포트폴리오 가치
        for (const auto& kv : portfolio) {
            const string& name = kv.first;
            const Position& pos = kv.second;
            Stock* stock = pos.stock ? pos.stock : m.getStockByName(name);
            if (!stock) continue;
            total += stock->getCurrentPrice() * pos.quantity;
        }

        // 갭 포지션 가치
        for (const auto& kv : gapPositions) {
            const string& name = kv.first;
            const Position& pos = kv.second;
            Stock* stock = pos.stock ? pos.stock : m.getStockByName(name);
            if (!stock) continue;
            total += stock->getCurrentPrice() * pos.quantity;
        }

        return total;
    }

    double getGapPositionProfit(const string& stockName) const {
        auto it = gapPositions.find(stockName);

        if (it == gapPositions.end()) return 0.0;

        const Position& pos = it->second;

        if (pos.quantity <= 0 || pos.avgBuyPrice <= 0.0) return 0.0;

        if (!pos.stock) return 0.0;

        double currentPrice = pos.stock->getCurrentPrice();

        // 수익률 = (현재가 - 평균 매수가) / 평균 매수가
        double profitRate = (currentPrice - pos.avgBuyPrice) / pos.avgBuyPrice;

        return profitRate;
    }
    
    double getCash() const {
        return cash;
    }

    const map<string, Position>& getGapPositions() const {
        return gapPositions;
    }

    int getGapPositionCount() const {
        return static_cast<int>(gapPositions.size());
    }

    void printGapPositionStatus() const {
        if (gapPositions.empty()) return;

        cout << "\n[갭 포지션 현황]" << endl;
        for (const auto& kv : gapPositions) {
            const string& name = kv.first;
            const Position& pos = kv.second;
            if (!pos.stock || pos.quantity <= 0) continue;

            double currentPrice = pos.stock->getCurrentPrice();
            double profitRate = (currentPrice - pos.avgBuyPrice) / pos.avgBuyPrice;
            double profitAmount = (currentPrice - pos.avgBuyPrice) * pos.quantity;

            cout << "  " << name
                 << " | 수량: " << pos.quantity
                 << " | 매수가: " << fixed << setprecision(0) << pos.avgBuyPrice
                 << " | 현재가: " << fixed << setprecision(0) << currentPrice
                 << " | 수익률: " << fixed << setprecision(2) << (profitRate * 100) << "%"
                 << " | 평가손익: " << fixed << setprecision(0) << profitAmount << endl;
        }
    }

    void printPortfolioStatus() const {
        if (portfolio.empty()) return;

        cout << "\n[포트폴리오 현황]" << endl;
        double totalProfit = 0.0;
        for (const auto& kv : portfolio) {
            const string& name = kv.first;
            const Position& pos = kv.second;
            if (!pos.stock || pos.quantity <= 0) continue;

            double currentPrice = pos.stock->getCurrentPrice();
            double profitRate = (currentPrice - pos.avgBuyPrice) / pos.avgBuyPrice;
            double profitAmount = (currentPrice - pos.avgBuyPrice) * pos.quantity;
            totalProfit += profitAmount;

            cout << "  " << name
                 << " | 수량: " << pos.quantity
                 << " | 매수가: " << fixed << setprecision(0) << pos.avgBuyPrice
                 << " | 현재가: " << fixed << setprecision(0) << currentPrice
                 << " | 수익률: " << fixed << setprecision(2) << (profitRate * 100) << "%"
                 << " | 평가손익: " << fixed << setprecision(0) << profitAmount << endl;
        }
        cout << "  [포트폴리오 총 평가손익]: " << fixed << setprecision(0) << totalProfit << endl;
    }
};

// ============================================================
// 팀원 C: MarketState 추상 클래스
// ============================================================
class MarketState {
public:
    virtual void handle(Trader& t, Market& m) = 0;
    virtual string getStateName() const = 0;
    virtual ~MarketState() {}
};

// ============================================================
// 팀원 C: NormalState 클래스
// ============================================================
class NormalState : public MarketState {
private:
    static constexpr double TARGET_PROFIT_RATE = 0.10;   // 목표 수익률 10%
    static constexpr double STOP_LOSS_RATE = -0.20;      // 손절 기준 -20%

public:
    void handle(Trader& t, Market& m) override {
        // 보수적 리밸런싱, 갭 포지션 청산 검사
        checkGapPositionRecovery(t, m);
    }

    string getStateName() const override { return "Normal"; }

    void checkGapPositionRecovery(Trader& t, Market& m) {
        // 갭 포지션 순회 -> 목표 수익률 달성 또는 손절 기준 도달 시 청산
        vector<pair<string, string>> toClose;  // (종목명, 청산사유)

        for (const auto& kv : t.getGapPositions()) {
            const string& name = kv.first;
            double profitRate = t.getGapPositionProfit(name);

            if (profitRate >= TARGET_PROFIT_RATE) {
                toClose.push_back({name, "목표수익"});
            } else if (profitRate <= STOP_LOSS_RATE) {
                toClose.push_back({name, "손절"});
            }
        }

        for (const auto& item : toClose) {
            const string& name = item.first;
            const string& reason = item.second;
            double profitRate = t.getGapPositionProfit(name);

            if (reason == "목표수익") {
                cout << "[NormalState] 갭 포지션 목표 수익 달성: " << name
                     << " (수익률: " << fixed << setprecision(2) << (profitRate * 100) << "%) 청산 수행" << endl;
            } else {
                cout << "[NormalState] 갭 포지션 손절: " << name
                     << " (수익률: " << fixed << setprecision(2) << (profitRate * 100) << "%) 손절 수행" << endl;
            }
            t.closeGapPosition(name, m);
        }
    }
};

// ============================================================
// 팀원 C: DisruptedState 클래스
// ============================================================
class DisruptedState : public MarketState {
private:
    static constexpr double INVESTMENT_RATIO = 0.50;
    static constexpr int MAX_POSITIONS = 3;

public:
    void handle(Trader& t, Market& m) override {
        // 특수 전략 실행
        executeContrarianBuy(t, m);
        executeGapTrading(t, m);
    }

    string getStateName() const override { return "Disrupted"; }

    void executeGapTrading(Trader& t, Market& m) {
        // 급락 주식 매수
        if (t.getGapPositionCount() >= MAX_POSITIONS) return;

        Stock* target = selectBestTarget(m);
        if (!target) return;

        // 이미 보유 중인지 확인
        const auto& gapPositions = t.getGapPositions();
        if (gapPositions.find(target->getName()) != gapPositions.end()) return;

        int targetCount = 1;
        double investAmount = calculateInvestAmount(t, targetCount);

        if (investAmount < 1.0) return;

        double price = target->getCurrentPrice();
        int qty = static_cast<int>(floor(investAmount / price));

        if (qty <= 0) return;

        bool ok = t.buyGapPosition(target->getName(), qty, m);

        if (ok) {
            double fall = target->getFluctuationRate();
            cout << "[DisruptedState] 갭 트레이딩 매수: " << target->getName()
                << " | 수량: " << qty << " | 가격: " << fixed << setprecision(2) << price
                << " | 하락률: " << fixed << setprecision(2) << (fall * 100) << "%" << endl;
        }
    }

    void executeContrarianBuy(Trader& t, Market& m) {
        // 역발상 매수
        if (t.getGapPositionCount() >= MAX_POSITIONS) return;

        Stock* target = selectBestTarget(m);
        if (!target) return;

        // 이미 보유 중인지 확인
        const auto& gapPositions = t.getGapPositions();
        if (gapPositions.find(target->getName()) != gapPositions.end()) return;

        int targetCount = 1;
        double investAmount = calculateInvestAmount(t, targetCount);

        if (investAmount < 1.0) return;

        double price = target->getCurrentPrice();
        int qty = static_cast<int>(floor(investAmount / price));

        if (qty <= 0) return;

        bool ok = t.buyGapPosition(target->getName(), qty, m);

        if (ok) {
            double fall = target->getFluctuationRate();
            cout << "[DisruptedState] 역발상 매수: " << target->getName()
                << " | 수량: " << qty << " | 가격: " << fixed << setprecision(2) << price
                << " | 하락률: " << fixed << setprecision(2) << (fall * 100) << "%" << endl;
        }
    }

    Stock* selectBestTarget(Market& m) {
        // 역발상 매수 대상 선정 조건: 가격 하락률 < -15% & 거래량 비율 > 2.0 (200%)
        // 우선 순위: 하락률 (최대) -> 거래량 비율 (최대)
        Stock* best = nullptr;
        double bestFall = 0.0;
        double bestVolRatio = 0.0;

        for (Stock* s : m.getAllStocks()) {
            if (!s) continue;

            double fall = s->getFluctuationRate();
            double volRatio = s->getVolumeRatio();

            if (fall < -0.15 && volRatio > 2.0) {
                if (best == nullptr || fall < bestFall || (fabs(fall - bestFall) < 1e-9 && volRatio > bestVolRatio)) {
                    best = s;
                    bestFall = fall;
                    bestVolRatio = volRatio;
                }
            }
        }

        return best;
    }

    double calculateInvestAmount(const Trader& t, int targets) {
        if (targets <= 0) return 0.0;

        double cash = t.getCash();

        return (cash * INVESTMENT_RATIO) / static_cast<double>(targets);
    }
};

// ============================================================
// 팀원 C: DisruptionDetector 클래스
// ============================================================
class DisruptionDetector {
private:
    unique_ptr<MarketState> currentState;
    double disruptionScore;
    static constexpr double THRESHOLD = 70.0;
    static constexpr double RECOVERY_THRESHOLD = 50.0;
    double previousScore;

public:
    DisruptionDetector()
        : currentState(nullptr), disruptionScore(0.0), previousScore(0.0) {
        // 초기 상태는 NormalState
        currentState = make_unique<NormalState>();
    }

    void analyzeMarket(Market& m) {
        previousScore = disruptionScore;
        disruptionScore = calculateScore(m);

        // 상태 전환 로직
        if (disruptionScore > THRESHOLD) {
            // disruptionScore > 70 -> DisruptedState로 전환
            if (getCurrentStateName() != "Disrupted") {
                changeState(new DisruptedState());
                cout << "[DisruptionDetector] 상태 전환 -> Disrupted" << endl;
            }
        }

        else if (disruptionScore < RECOVERY_THRESHOLD) {
            // disruptionScore < 50 -> NormalState로 복귀
            if (getCurrentStateName() != "Normal") {
                changeState(new NormalState());
                cout << "[DisruptionDetector] 상태 전환 -> Normal" << endl;
            }
        }

        else {
            // 50 <= disruptionScore <= 70: 현재 상태 유지
        }
    }

    void changeState(MarketState* newState) {
        if (!newState) return;

        currentState.reset(newState);
    }

    void executeStrategy(Trader& t, Market& m) {
        if (currentState) {
            currentState->handle(t, m);
        }
    }

    double calculateScore(Market& m) const {
        // 위험도 계산: disruptionScore = (현재 거래량 / 평균 거래량) x 변동성 x 1000
        // 현재 거래량 / 평균 거래량은 최대 거래량 비율을 사용하여 가장 과도한 종목을 잡음
        const vector<Stock*>& stocks = m.getAllStocks();
        if (stocks.empty()) return 0.0;


        double maxVolRatio = 0.0;

        for (Stock* s : stocks) {
            double vr = s->getVolumeRatio();

            if (vr > maxVolRatio) maxVolRatio = vr;
        }

        double volatility = m.getMarketVolatility();
        double score = maxVolRatio * volatility * 1000.0;

        return score;
    }

    bool checkRecovery() const {
        return disruptionScore < RECOVERY_THRESHOLD;
    }

    double getDisruptionScore() const {
        return disruptionScore;
    }

    bool isDisrupted() const {
        return disruptionScore > THRESHOLD;
    }

    string getCurrentStateName() const {
        if (currentState) return currentState->getStateName();

        return "Unknown";
    }
};

// ============================================================
// Trader::autoTrade() 구현 (DisruptionDetector 정의 후)
// ============================================================
void Trader::autoTrade(Market& m) {
    if (!detector) return;

    // 1) 시장 분석
    detector->analyzeMarket(m);

    // 로그
    cout << "[자동매매] 현재 상태: " << detector->getCurrentStateName()
        << " | DisruptionScore: " << fixed << setprecision(2) << detector->getDisruptionScore() << endl;

    // 2) 현재 상태에 따른 전략 실행
    detector->executeStrategy(*this, m);
}

// ============================================================
// Main (테스트용)
// ============================================================
int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    cout << "==============================" << endl;
    cout << "     [주식 거래 프로그램]     " << endl;
    cout << "==============================\n" << endl;

    // 팀원 A: Market, Stock 테스트
    Market market;
    market.addStock(new Stock("삼성전자", 70000.0, 1000000.0));
    market.addStock(new Stock("SK하이닉스", 130000.0, 500000.0));
    market.addStock(new Stock("네이버", 200000.0, 300000.0));
    market.addStock(new Stock("카카오", 50000.0, 800000.0));
    market.addStock(new Stock("LG에너지솔루션", 400000.0, 200000.0));

    cout << "[초기 상태]" << endl;
    market.printMarketStatus();

    // Trader 생성 (예시)
    Trader trader("F team trader", 10000000.0);  // 초기 현금 1천만 원 (단위 가정)

    cout << "[자동매매 10일]" << endl;

    for (int day = 1; day <= 10; ++day) {
        cout << "--- [Day " << day << "]---" << endl;
        market.simulateFluctuation();
        market.printMarketStatus();

        // 팀원 B, C: Trader 테스트
        // Trader 자동매매 (매일 한 번)
        trader.autoTrade(market);
        trader.printPortfolioStatus();
        trader.printGapPositionStatus();
        cout << "[현금]: " << fixed << setprecision(0) << trader.getCash()
            << " | [전체 자산 평가]: " << fixed << setprecision(0) << trader.getTotalAssetValue(market) << "\n" << endl;
    }

    cout << "\n====== [최종 거래 내역] ======" << endl;
    market.printTransactionHistory();

    return 0;
}