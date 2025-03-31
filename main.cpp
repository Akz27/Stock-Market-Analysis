#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <cstdlib>
#include <numeric>
#include <algorithm>

using namespace std;

// Struct to hold stock data, including the chart link
struct StockData {
    string ticker;
    float open;
    float dayHigh;
    float dayLow;
    float lastPrice;
    string chartTodayPath; // Field to store today's chart link
    string chart30DaysPath; // Field to store 30-day chart link
    string chart365DaysPath; // Field to store 365-day chart link
};

// AVL Tree Node
class AVLTreeNode {
public:
    StockData stock;
    AVLTreeNode* left;
    AVLTreeNode* right;
    int height;

    AVLTreeNode(const StockData& stockData)
        : stock(stockData), left(nullptr), right(nullptr), height(1) {}
};

// AVL Tree for managing stock data
class AVLTree {
private:
    AVLTreeNode* root;

    int height(AVLTreeNode* node) {
        return node ? node->height : 0;
    }

    int balanceFactor(AVLTreeNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }

    AVLTreeNode* rotateRight(AVLTreeNode* y) {
        AVLTreeNode* x = y->left;
        AVLTreeNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;
        return x;
    }

    AVLTreeNode* rotateLeft(AVLTreeNode* x) {
        AVLTreeNode* y = x->right;
        AVLTreeNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;
        return y;
    }

   AVLTreeNode* insert(AVLTreeNode* node, const StockData& stockData) {
        if (!node) return new AVLTreeNode(stockData);

        if (stockData.ticker < node->stock.ticker)
            node->left = insert(node->left, stockData);
        else if (stockData.ticker > node->stock.ticker)
            node->right = insert(node->right, stockData);
        else
            return node;

        node->height = 1 + max(height(node->left), height(node->right));
        int balance = balanceFactor(node);

        if (balance > 1 && stockData.ticker < node->left->stock.ticker)
            return rotateRight(node);
        if (balance < -1 && stockData.ticker > node->right->stock.ticker)
            return rotateLeft(node);
        if (balance > 1 && stockData.ticker > node->left->stock.ticker) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balance < -1 && stockData.ticker < node->right->stock.ticker) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    AVLTreeNode* update(AVLTreeNode* node, const StockData& stockData) {
        if (!node) return nullptr;

        if (stockData.ticker < node->stock.ticker) {
            node->left = update(node->left, stockData);
        } else if (stockData.ticker > node->stock.ticker) {
            node->right = update(node->right, stockData);
        } else {
            node->stock = stockData;
        }

        return node;
    }


    AVLTreeNode* minNode(AVLTreeNode* node) {
        AVLTreeNode* current = node;
        while (current && current->left) {
            current = current->left;
        }
        return current;
    }

    AVLTreeNode* deleteNode(AVLTreeNode* node, const string& ticker) {
        if (!node) return node;

        if (ticker < node->stock.ticker)
            node->left = deleteNode(node->left, ticker);
        else if (ticker > node->stock.ticker)
            node->right = deleteNode(node->right, ticker);
        else {
            if (!node->left) {
                AVLTreeNode* temp = node->right;
                delete node;
                return temp;
            } else if (!node->right) {
                AVLTreeNode* temp = node->left;
                delete node;
                return temp;
            }

            AVLTreeNode* temp = minNode(node->right);
            node->stock = temp->stock;
            node->right = deleteNode(node->right, temp->stock.ticker);
        }

        node->height = 1 + max(height(node->left), height(node->right));
        int balance = balanceFactor(node);

        if (balance > 1 && balanceFactor(node->left) >= 0)
            return rotateRight(node);
        if (balance > 1 && balanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (balance < -1 && balanceFactor(node->right) <= 0)
            return rotateLeft(node);
        if (balance < -1 && balanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    void printInOrder(AVLTreeNode* node) {
        if (!node) return;
        printInOrder(node->left);
        cout << "Ticker: " << node->stock.ticker
             << ", Open: " << node->stock.open
             << ", Day High: " << node->stock.dayHigh
             << ", Day Low: " << node->stock.dayLow
             << ", Last Price: " << node->stock.lastPrice << endl;
        printInOrder(node->right);
    }

public:
    AVLTree() : root(nullptr) {}

    void insert(const StockData& stockData) {
        root = insert(root, stockData);
    }

    void update(const StockData& stockData) {
        root = update(root, stockData);
    }

    void deleteStock(const string& ticker) {
        root = deleteNode(root, ticker);
    }

    void printTree() {
        printInOrder(root);
    }

    AVLTreeNode* getRoot() {
        return root;
    }
};

// Function to load stock data and chart links from a CSV file
void loadStockData(const string& filePath, vector<StockData>& stockList) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error: Could not open the file " << filePath << endl;
        return;
    }

    string line;
    bool isHeader = true;
    while (getline(file, line)) {
        if (isHeader) {
            isHeader = false;
            continue;
        }

        stringstream ss(line);
        string token;
        string ticker, chartLinkToday, chartLink30Days, chartLink365Days;
        float open = 0.0f, dayHigh = 0.0f, dayLow = 0.0f, lastPrice = 0.0f;

        int column = 0;
        while (getline(ss, token, ',')) {
            switch (column) {
                case 1:
                    ticker = token;
                    break;
                case 3:
                    open = stof(token);
                    break;
                case 4:
                    dayHigh = stof(token);
                    break;
                case 5:
                    dayLow = stof(token);
                    break;
                case 6:
                    lastPrice = stof(token);
                    break;
                case 23:
                    chartLink30Days = token; // 30-day chart link
                    break;
                case 24:
                    chartLinkToday = token;  // Today's chart link
                    break;
                case 20:
                    chartLink365Days = token; // 365-day chart link
                    break;
            }
            column++;
        }

        if (!ticker.empty()) {
            stockList.push_back({ticker, open, dayHigh, dayLow, lastPrice, chartLinkToday, chartLink30Days, chartLink365Days});
        } else {
            cerr << "Invalid data in line: " << line << endl;
        }
    }

    file.close();
}

// Function to open URL in default browser
void openURL(const string& url) {
  system(("start " + url).c_str());
}

// Function to visualize stock chart by ticker
void visualize(const string& filePath, const string& ticker) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error: Could not open the file " << filePath << endl;
        return;
    }

    string line;
    bool isHeader = true;
    while (getline(file, line)) {
        if (isHeader) {
            isHeader = false;
            continue;
        }

        stringstream ss(line);
        string token;
        string stockTicker, chartLinkToday, chartLink30Days, chartLink365Days;
        int column = 0;

        while (getline(ss, token, ',')) {
            if (column == 1) stockTicker = token;
            else if (column == 25) chartLinkToday = token;   // Column 24 - Today's Chart
            else if (column == 24) chartLink30Days = token;  // Column 23 - 30 Days Chart
            else if (column == 21) chartLink365Days = token; // Column 20 - 365 Days Chart
            column++;
        }

        if (stockTicker == ticker) {
            int chartChoice;
            cout << "\nWhich chart do you want to visualize?\n";
            cout << "1. Today's Chart\n";
            cout << "2. 30-Day Chart\n";
            cout << "3. 365-Day Chart\n";
            cout << "Enter your choice: ";
            cin >> chartChoice;

            string chartLink;
            switch (chartChoice) {
                case 1:
                    chartLink = chartLinkToday;
                    break;
                case 2:
                    chartLink = chartLink30Days;
                    break;
                case 3:
                    chartLink = chartLink365Days;
                    break;
                default:
                    cout << "Invalid choice. No chart displayed.\n";
                    file.close();
                    return;
            }

            if (!chartLink.empty()) {
                cout << "Opening chart for " << ticker << " at " << chartLink << endl;
                openURL(chartLink);
            } else {
                cout << "No chart available for the selected period.\n";
            }
            file.close();
            return;
        }
    }

    cerr << "Error: Stock ticker " << ticker << " not found.\n";
    file.close();
}
void findHighestPrice(AVLTreeNode* node, StockData& highest) {
    if (!node) return;

    if (node->stock.lastPrice > highest.lastPrice) {
        highest = node->stock;
    }

    findHighestPrice(node->left, highest);
    findHighestPrice(node->right, highest);
}

void findLowestPrice(AVLTreeNode* node, StockData& lowest) {
    if (!node) return;

    if (node->stock.lastPrice < lowest.lastPrice) {
        lowest = node->stock;
    }

    findLowestPrice(node->left, lowest);
    findLowestPrice(node->right, lowest);
}

void fetchByRange(AVLTreeNode* node, float minPrice, float maxPrice) {
    if (!node) return;

    if (node->stock.lastPrice >= minPrice) {
        fetchByRange(node->left, minPrice, maxPrice);
    }

    if (node->stock.lastPrice >= minPrice && node->stock.lastPrice <= maxPrice) {
        cout << "Ticker: " << node->stock.ticker
             << ", Open: " << node->stock.open
             << ", Day High: " << node->stock.dayHigh
             << ", Day Low: " << node->stock.dayLow
             << ", Last Price: " << node->stock.lastPrice << endl;
    }

    if (node->stock.lastPrice <= maxPrice) {
        fetchByRange(node->right, minPrice, maxPrice);
    }
}

void fetchByName(AVLTreeNode* node, const string& ticker) {
    if (!node) {
        cout << "Stock with ticker " << ticker << " not found." << endl;
        return;
    }

    if (node->stock.ticker < ticker) {
        fetchByName(node->right, ticker);
    } else if (node->stock.ticker > ticker) {
        fetchByName(node->left, ticker);
    } else {
        cout << "Ticker: " << node->stock.ticker
             << ", Open: " << node->stock.open
             << ", Day High: " << node->stock.dayHigh
             << ", Day Low: " << node->stock.dayLow
             << ", Last Price: " << node->stock.lastPrice << endl;
    }
}

double mean(const vector<double>& data) {
    double sum = accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

// Function to calculate linear regression coefficients (slope and intercept)
void linearRegression(const vector<double>& x, const vector<double>& y, double& slope, double& intercept) {
    double x_mean = mean(x);
    double y_mean = mean(y);

    double numerator = 0.0;
    double denominator = 0.0;

    for (size_t i = 0; i < x.size(); ++i) {
        numerator += (x[i] - x_mean) * (y[i] - y_mean);
        denominator += (x[i] - x_mean) * (x[i] - x_mean);
    }

    if (denominator == 0) {
        cerr << "Error: Denominator for slope calculation is zero." << endl;
        return;
    }

    slope = numerator / denominator;
    intercept = y_mean - slope * x_mean;
}

// Function to predict a future stock price
double predictPrice(int dayIndex, double slope, double intercept) {
    return slope * dayIndex + intercept;
}

// Function to read stock prices from a CSV file
vector<double> readPricesFromCSV(const string& filename, int priceColumnIndex) {
    vector<double> prices;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return prices;
    }

    string line;
    bool isFirstLine = true;

    while (getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }

        stringstream ss(line);
        string value;
        vector<string> row;

        while (getline(ss, value, ',')) {
            row.push_back(value);
        }

        if (row.size() > priceColumnIndex) {
            try {
                prices.push_back(stod(row[priceColumnIndex])); // Convert the specific column to double
            } catch (const invalid_argument& e) {
                cerr << "Warning: Skipping non-numeric data: " << row[priceColumnIndex] << endl;
                continue;
            }
        }
    }

    file.close();
    return prices;
}

int main() {
    AVLTree stockTree;
    vector<StockData> stockList;
    loadStockData("C:\\Users\\ASUS\\Desktop\\ads_sem5\\livestock.csv", stockList);

    for (const auto& stock : stockList) {
        stockTree.insert(stock);
    }

    int choice;
    string ticker;
    StockData stockData;
    float minPrice, maxPrice;
    StockData highestStock, lowestStock;

    do {
        cout << "\nMenu:\n";
        cout << "1. Add Stock\n";
        cout << "2. Update Stock\n";
        cout << "3. Delete Stock\n";
        cout << "4. Display All Stocks\n";
        cout << "5. Visualize Stock Chart\n";
        cout << "6. Find Highest Stock Price\n";
        cout << "7. Find Lowest Stock Price\n";
        cout << "8. Fetch Stocks (By Range or By Name)\n";
        cout << "9. Predict Stock Price\n";
        cout << "10. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Enter ticker, open, day high, day low, last price, and chart link: ";
                cin >> stockData.ticker >> stockData.open >> stockData.dayHigh >> stockData.dayLow >> stockData.lastPrice;
                cin.ignore();
                stockTree.insert(stockData);
                break;

            case 2:
                cout << "Enter stock ticker to update: ";
                cin >> stockData.ticker;
                cout << "Enter updated data for ticker: (open, high, day low, last price): ";
                cin >> stockData.open >> stockData.dayHigh >> stockData.dayLow >> stockData.lastPrice;
                cin.ignore();
                stockTree.update(stockData);
                break;

            case 3:
                cout << "Enter ticker to delete: ";
                cin >> ticker;
                stockTree.deleteStock(ticker);
                break;

            case 4:
                cout << "Displaying all stocks:\n";
                stockTree.printTree();
                break;

            case 5:
                cout << "Enter ticker to visualize chart: ";
                cin >> ticker;
                visualize("C:\\Users\\ASUS\\Desktop\\ads_sem5\\livestock.csv", ticker);
                break;

            case 6:
                highestStock = stockList[0];  // Initialize with the first stock as highest
                findHighestPrice(stockTree.getRoot(), highestStock);
                cout << "Highest Stock Price: " << highestStock.ticker << " - " << highestStock.lastPrice << endl;
                break;

            case 7:
                lowestStock = stockList[0];  // Initialize with the first stock as lowest
                findLowestPrice(stockTree.getRoot(), lowestStock);
                cout << "Lowest Stock Price: " << lowestStock.ticker << " - " << lowestStock.lastPrice << endl;
                break;

            case 8:
                int fetchChoice;
                cout << "Fetch Stocks By:\n";
                cout << "1. By Price Range\n";
                cout << "2. By Ticker Name\n";
                cout << "Enter your choice: ";
                cin >> fetchChoice;

                if (fetchChoice == 1) {
                    cout << "Enter minimum price: ";
                    cin >> minPrice;
                    cout << "Enter maximum price: ";
                    cin >> maxPrice;
                    cout << "Stocks within the price range (" << minPrice << ", " << maxPrice << "):\n";
                    fetchByRange(stockTree.getRoot(), minPrice, maxPrice);
                } else if (fetchChoice == 2) {
                    cout << "Enter ticker to fetch: ";
                    cin >> ticker;
                    fetchByName(stockTree.getRoot(), ticker);
                } else {
                    cout << "Invalid choice. Returning to the main menu.\n";
                }
                break;

            case 9: {  // Predict stock price
                string filename = "C:\\Users\\ASUS\\Desktop\\ads_sem5\\Tesla.csv";
                int priceColumnIndex = 4;  // Assuming the 'close' price column index is 4

                // Read stock prices from CSV
                vector<double> stockPrices = readPricesFromCSV(filename, priceColumnIndex);
                int n = stockPrices.size();

                if (n == 0) {
                    cerr << "Error: No valid data found in CSV file." << endl;
                    break;
                }

                // Generate days vector for regression
                vector<double> days(n);
                for (int i = 0; i < n; ++i) {
                    days[i] = i;
                }

                // Calculate regression coefficients
                double slope = 0.0, intercept = 0.0;
                linearRegression(days, stockPrices, slope, intercept);

                // Check if slope calculation succeeded
                if (slope == 0 && intercept == 0) {
                    cerr << "Error: Regression calculation failed." << endl;
                    break;
                }

                // Predict the next day's price
                double predictedPrice = predictPrice(n, slope, intercept);
                cout << "Predicted price for the next day (day " << n << ") is: " << predictedPrice << endl;
                break;
            }

            case 10:
                cout << "Exiting.\n";
                break;

            default:
                cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 10);

    return 0;
}

