#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <iomanip>

using namespace std;

// Forward Declarations
class Stock;
class Transaction;
class Market;
class Position;
class Trader;
class DisruptionDetector;
class MarketState;
class NormalState;
class DisruptedState;

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
          volume(initVolume), averageVolume(initVolume) {}

    void updatePrice(double newPrice, double newVolume) {
        previousPrice = currentPrice;
        currentPrice = newPrice;
        setVolume(newVolume);
    }

    double getFluctuationRate() const {
        if (previousPrice == 0) return 0.0;
        return (currentPrice - previousPrice) / previousPrice;
    }

    void setVolume(double newVolume) {
        volume = newVolume;
        averageVolume = averageVolume * 0.95 + newVolume * 0.05;  // EMA
    }

    string getName() const { return name; }
    double getCurrentPrice() const { return currentPrice; }
    double getPreviousPrice() const { return previousPrice; }
    double getVolume() const { return volume; }
    double getAverageVolume() const { return averageVolume; }

    double getVolumeRatio() const {
        if (averageVolume == 0) return 0.0;
        return volume / averageVolume;
    }
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

public:
    Transaction(const string& stockName, const string& type, int quantity,
                double price, bool isGapTrade = false)
        : stockName(stockName), type(type), quantity(quantity),
          price(price), isGapTrade(isGapTrade) {
        totalAmount = price * quantity;
        timestamp = time(nullptr);
    }

    void printLog() const {
        cout << "[" << put_time(localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << "] "
             << (isGapTrade ? "[GAP] " : "")
             << type << " " << stockName
             << " | 수량: " << quantity
             << " | 단가: " << fixed << setprecision(2) << price
             << " | 총액: " << totalAmount << endl;
    }

    string getType() const { return type; }
    string getStockName() const { return stockName; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }
    double getTotalAmount() const { return totalAmount; }
    bool getIsGapTrade() const { return isGapTrade; }
    time_t getTimestamp() const { return timestamp; }

    void setType(const string& t) { type = t; }
    void setGapTradeFlag(bool flag) { isGapTrade = flag; }
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
            double randomValue = (double)rand() / RAND_MAX;
            double changeRate;
            double volumeMultiplier;

            if (randomValue < 0.95) {
                changeRate = ((double)rand() / RAND_MAX) * 0.04 - 0.02;
                volumeMultiplier = 0.8 + ((double)rand() / RAND_MAX) * 0.4;
            } else {
                changeRate = ((double)rand() / RAND_MAX) * (-0.20) - 0.10;
                volumeMultiplier = 2.0 + ((double)rand() / RAND_MAX) * 2.0;
            }

            double newPrice = stock->getCurrentPrice() * (1.0 + changeRate);
            if (newPrice < 0.01) newPrice = 0.01;

            double newVolume = stock->getAverageVolume() * volumeMultiplier;
            stock->updatePrice(newPrice, newVolume);
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

        return sqrt(variance);
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

    void printTransactionHistory() const {
        cout << "\n===== 거래 내역 =====" << endl;
        for (const Transaction& t : transactionHistory) {
            t.printLog();
        }
        cout << "=====================\n" << endl;
    }

    void printMarketStatus() const {
        cout << "\n===== 시장 현황 =====" << endl;
        cout << "시장 변동성: " << fixed << setprecision(4)
             << (getMarketVolatility() * 100) << "%" << endl;
        cout << "평균 거래량: " << fixed << setprecision(0)
             << getAverageVolume() << endl;
        cout << "\n[종목별 현황]" << endl;
        for (Stock* stock : allStocks) {
            cout << stock->getName()
                 << " | 현재가: " << fixed << setprecision(2) << stock->getCurrentPrice()
                 << " | 변동률: " << fixed << setprecision(2) << (stock->getFluctuationRate() * 100) << "%"
                 << " | 거래량 비율: " << fixed << setprecision(2) << (stock->getVolumeRatio() * 100) << "%"
                 << endl;
        }
        cout << "=====================\n" << endl;
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

    // 팀원 B: 포지션 생성자
    Position(Stock* s = nullptr, int qty = 0, double price = 0.0)
        : stock(s), quantity(qty), avgBuyPrice(price) {
        buyTimestamp = time(nullptr);
    }
};

// ============================================================
// 팀원 C: MarketState 추상 클래스
// ============================================================
class MarketState {
public:
    virtual ~MarketState() {}
    virtual void handle(Trader& t, Market& m) = 0;
    virtual string getStateName() const = 0;
};

// ============================================================
// 팀원 C: NormalState 클래스
// ============================================================
class NormalState : public MarketState {
private:
    static constexpr double TARGET_PROFIT_RATE = 0.10;

public:
    void handle(Trader& t, Market& m) override;
    string getStateName() const override { return "Normal"; }
    void checkGapPositionRecovery(Trader& t, Market& m);
};

// ============================================================
// 팀원 C: DisruptedState 클래스
// ============================================================
class DisruptedState : public MarketState {
private:
    static constexpr double INVESTMENT_RATIO = 0.50;
    static constexpr int MAX_POSITIONS = 3;

public:
    void handle(Trader& t, Market& m) override;
    string getStateName() const override { return "Disrupted"; }
    void executeGapTrading(Trader& t, Market& m);
    void executeContrarianBuy(Trader& t, Market& m);
    Stock* selectBestTarget(Market& m, const map<string, Position>& currentGapPositions);
    double calculateInvestAmount(double cash, int targets);
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
    DisruptionDetector();
    void analyzeMarket(Market& m);
    void changeState(MarketState* newState);
    void executeStrategy(Trader& t, Market& m);
    double calculateScore(Market& m) const;
    bool checkRecovery() const;
    double getDisruptionScore() const;
    bool isDisrupted() const;
    string getCurrentStateName() const;
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
    Trader(const string& name, double initialCash);
    bool buy(const string& stockName, int qty, Market& m);
    bool sell(const string& stockName, int qty, Market& m);
    void autoTrade(Market& m);
    bool buyGapPosition(const string& stockName, int qty, Market& m);
    bool closeGapPosition(const string& stockName, Market& m);
    void closeAllGapPositions(Market& m);
    bool hasOpenGapPositions() const;
    int getPositionQuantity(const string& stockName) const;
    double getTotalAssetValue(Market& m) const;
    double getGapPositionProfit(const string& stockName) const;
    double getCash() const;
    const map<string, Position>& getGapPositions() const;
    int getGapPositionCount() const;
};

// ============================================================
// 팀원 B: Trader 구현
// ============================================================
Trader::Trader(const string& name, double initialCash)
    : username(name), cash(initialCash) {
    // detector는 Phase 3에서 DisruptionDetector 구현 시 생성 예정
}

bool Trader::buy(const string& stockName, int qty, Market& m) {
    if (qty <= 0) return false;

    Stock* stock = m.getStockByName(stockName);
    if (!stock) return false;

    double price = stock->getCurrentPrice();
    double cost = price * qty;

    if (cost > cash) return false;

    // 현금 차감
    cash -= cost;

    // 포트폴리오 업데이트 (가중 평균 단가)
    auto it = portfolio.find(stockName);
    if (it == portfolio.end()) {
        portfolio[stockName] = Position(stock, qty, price);
    } else {
        Position& pos = it->second;
        int newQty = pos.quantity + qty;
        double newAvg =
            (pos.avgBuyPrice * pos.quantity + price * qty) / newQty;
        pos.quantity = newQty;
        pos.avgBuyPrice = newAvg;
        pos.buyTimestamp = time(nullptr);
    }

    // 거래 내역 기록
    Transaction t(stockName, "BUY", qty, price, false);
    m.addTransaction(t);

    return true;
}

bool Trader::sell(const string& stockName, int qty, Market& m) {
    if (qty <= 0) return false;

    auto it = portfolio.find(stockName);
    if (it == portfolio.end()) return false;

    Position& pos = it->second;
    if (qty > pos.quantity) return false;

    Stock* stock = pos.stock ? pos.stock : m.getStockByName(stockName);
    if (!stock) return false;

    double price = stock->getCurrentPrice();
    double revenue = price * qty;

    // 포지션 및 현금 업데이트
    pos.quantity -= qty;
    cash += revenue;

    if (pos.quantity == 0) {
        portfolio.erase(it);
    }

    // 거래 내역 기록
    Transaction t(stockName, "SELL", qty, price, false);
    m.addTransaction(t);

    return true;
}

void Trader::autoTrade(Market& m) {
    // Phase 3에서 DisruptionDetector와 연동 예정
    // 현재 단계에서는 자동 매매 로직 미구현
    (void)m; // 미사용 매개변수 경고 방지
}

bool Trader::buyGapPosition(const string& stockName, int qty, Market& m) {
    if (qty <= 0) return false;

    Stock* stock = m.getStockByName(stockName);
    if (!stock) return false;

    double price = stock->getCurrentPrice();
    double cost = price * qty;

    if (cost > cash) return false;

    cash -= cost;

    auto it = gapPositions.find(stockName);
    if (it == gapPositions.end()) {
        gapPositions[stockName] = Position(stock, qty, price);
    } else {
        Position& pos = it->second;
        int newQty = pos.quantity + qty;
        double newAvg =
            (pos.avgBuyPrice * pos.quantity + price * qty) / newQty;
        pos.quantity = newQty;
        pos.avgBuyPrice = newAvg;
        pos.buyTimestamp = time(nullptr);
    }

    Transaction t(stockName, "GAP_BUY", qty, price, true);
    m.addTransaction(t);

    return true;
}

bool Trader::closeGapPosition(const string& stockName, Market& m) {
    auto it = gapPositions.find(stockName);
    if (it == gapPositions.end()) return false;

    Position& pos = it->second;
    if (pos.quantity <= 0) return false;

    Stock* stock = pos.stock ? pos.stock : m.getStockByName(stockName);
    if (!stock) return false;

    int qty = pos.quantity;
    double price = stock->getCurrentPrice();
    double revenue = price * qty;

    cash += revenue;

    Transaction t(stockName, "GAP_SELL", qty, price, true);
    m.addTransaction(t);

    gapPositions.erase(it);

    return true;
}

void Trader::closeAllGapPositions(Market& m) {
    // 키를 복사해두고 순회 중 map을 수정
    vector<string> keys;
    keys.reserve(gapPositions.size());
    for (const auto& kv : gapPositions) {
        keys.push_back(kv.first);
    }

    for (const string& name : keys) {
        closeGapPosition(name, m);
    }
}

bool Trader::hasOpenGapPositions() const {
    return !gapPositions.empty();
}

int Trader::getPositionQuantity(const string& stockName) const {
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

double Trader::getTotalAssetValue(Market& m) const {
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

double Trader::getGapPositionProfit(const string& stockName) const {
    auto it = gapPositions.find(stockName);
    if (it == gapPositions.end()) return 0.0;

    const Position& pos = it->second;
    if (pos.quantity <= 0 || pos.avgBuyPrice <= 0.0) return 0.0;

    // 현재가는 Market 객체가 필요하므로, 수익률 계산은
    // DisruptedState/NormalState에서 필요 시 별도 로직에서 처리 가능
    // 여기서는 매수가 기준의 누적 수익률만 반환
    // (현재 단계에서는 Market 의존성을 줄이기 위해 0.0 반환)
    return 0.0;
}

double Trader::getCash() const {
    return cash;
}

const map<string, Position>& Trader::getGapPositions() const {
    return gapPositions;
}

int Trader::getGapPositionCount() const {
    return static_cast<int>(gapPositions.size());
}

// ============================================================
// 팀원 C: DisruptionDetector 구현
// ============================================================
// TODO: 팀원 C 구현

// ============================================================
// 팀원 C: NormalState 구현
// ============================================================
// TODO: 팀원 C 구현

// ============================================================
// 팀원 C: DisruptedState 구현
// ============================================================
// TODO: 팀원 C 구현

// ============================================================
// Main (테스트용 - 팀원 B, C 구현 후 활성화)
// ============================================================
int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    cout << "========================================" << endl;
    cout << "      주식 거래 시뮬레이션 프로그램" << endl;
    cout << "========================================\n" << endl;

    // 팀원 A: Market, Stock 테스트
    Market market;
    market.addStock(new Stock("삼성전자", 70000.0, 1000000.0));
    market.addStock(new Stock("SK하이닉스", 130000.0, 500000.0));
    market.addStock(new Stock("네이버", 200000.0, 300000.0));
    market.addStock(new Stock("카카오", 50000.0, 800000.0));
    market.addStock(new Stock("LG에너지솔루션", 400000.0, 200000.0));

    cout << "[초기 상태]" << endl;
    market.printMarketStatus();

    cout << "[시뮬레이션 5일간]" << endl;
    for (int day = 1; day <= 5; ++day) {
        cout << "--- Day " << day << " ---" << endl;
        market.simulateFluctuation();
        market.printMarketStatus();
    }

    // TODO: 팀원 B, C 구현 후 Trader 테스트 추가

    return 0;
}
