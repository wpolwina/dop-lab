#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <climits>
#include <functional>
using namespace std;

bool MAXMIN(vector<int>& denominations, vector<int>& available, vector<int>& taken,
    int amount, int index, const string& strategy) {
    if (amount == 0) return true;
    if (index == denominations.size()) return false;

    int maxTake = min(available[index], amount / denominations[index]);

    if (strategy == "MAX") {
        for (int take = maxTake; take >= 0; take--) {
            taken[index] = take;
            if (MAXMIN(denominations, available, taken,
                amount - take * denominations[index], index + 1, strategy)) {
                return true;
            }
        }
    }
    else {
        for (int take = 0; take <= maxTake; take++) {
            taken[index] = take;
            if (MAXMIN(denominations, available, taken,
                amount - take * denominations[index], index + 1, strategy)) {
                return true;
            }
        }
    }

    taken[index] = 0;
    return false;
}

bool UNIFORM(vector<int>& denominations, vector<int>& available, vector<int>& taken, int amount) {
    int n = denominations.size();
    vector<int> best;
    int bestDiff = INT_MAX;
    vector<int> tempTaken(n, 0);

    vector<int> indices(n);
    for (int i = 0; i < n; i++) indices[i] = i;
    sort(indices.begin(), indices.end(), [&](int a, int b) {
        return denominations[a] < denominations[b];
        });

    vector<int> sortedDenom(n), sortedAvail(n);
    for (int i = 0; i < n; i++) {
        sortedDenom[i] = denominations[indices[i]];
        sortedAvail[i] = available[indices[i]];
    }

    function<void(int, int)> dfs = [&](int idx, int rem) {
        if (rem == 0) {
            int minC = INT_MAX, maxC = 0;
            for (int i = 0; i < n; i++) {
                minC = min(minC, tempTaken[i]);
                maxC = max(maxC, tempTaken[i]);
            }
            int diff = maxC - minC;
            if (diff < bestDiff) {
                bestDiff = diff;
                best = tempTaken;
            }
            return;
        }
        if (idx == n) return;

        int maxTake = min(sortedAvail[idx], rem / sortedDenom[idx]);
        if (maxTake > 20) maxTake = 20;

        for (int take = 0; take <= maxTake; take++) {
            tempTaken[idx] = take;
            dfs(idx + 1, rem - take * sortedDenom[idx]);
        }
        tempTaken[idx] = 0;
        };

    dfs(0, amount);

    if (!best.empty()) {
        taken.assign(n, 0);
        for (int i = 0; i < n; i++) {
            taken[indices[i]] = best[i];
        }
        return true;
    }

    return false;
}

bool parse(const string& filename, vector<pair<int, int>>& wallet, int& amount, string& strategy) {
    ifstream file(filename);
    if (!file.is_open()) return false;

    string content;
    char ch;
    while (file.get(ch)) content += ch;
    file.close();

    size_t pos = content.find("\"wallet\"");
    if (pos == string::npos) return false;

    pos = content.find("[", pos);
    if (pos == string::npos) return false;

    while (true) {
        pos = content.find("[", pos + 1);
        if (pos == string::npos) break;

        int value = 0, count = 0;
        size_t start = pos + 1;
        size_t end = content.find(",", start);
        if (end != string::npos) {
            value = stoi(content.substr(start, end - start));
        }

        start = end + 1;
        end = content.find("]", start);
        if (end != string::npos) {
            count = stoi(content.substr(start, end - start));
        }

        wallet.push_back({ value, count });
    }

    pos = content.find("\"amount\"");
    if (pos != string::npos) {
        pos = content.find(":", pos);
        if (pos != string::npos) {
            pos++;
            while (pos < content.length() && (content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\n')) pos++;

            string numStr;
            while (pos < content.length() && isdigit(content[pos])) {
                numStr += content[pos];
                pos++;
            }
            if (!numStr.empty()) {
                amount = stoi(numStr);
            }
        }
    }

    pos = content.find("\"strategy\"");
    if (pos != string::npos) {
        pos = content.find("\"", pos + 10);
        if (pos != string::npos) {
            size_t endQuote = content.find("\"", pos + 1);
            if (endQuote != string::npos) {
                strategy = content.substr(pos + 1, endQuote - pos - 1);
            }
        }
    }

    return true;
}

int main() {
    setlocale(LC_ALL, "Russian");

    vector<pair<int, int>> wallet;
    int amount = 0;
    string strategy;

    if (!parse("input.json", wallet, amount, strategy)) {
        cerr << "Ошибка: не прочтен input.json" << endl;
        return 1;
    }

    vector<int> denominations, available;
    for (auto& p : wallet) {
        denominations.push_back(p.first);
        available.push_back(p.second);
    }

    vector<int> taken(denominations.size(), 0);
    bool solved = false;

    if (strategy == "UNIFORM") {
        solved = UNIFORM(denominations, available, taken, amount);
    }
    else {
        vector<int> indices(denominations.size());
        for (size_t i = 0; i < indices.size(); i++) indices[i] = i;

        if (strategy == "MAX") {
            sort(indices.begin(), indices.end(), [&](int a, int b) {
                return denominations[a] > denominations[b];
                });
        }
        else {
            sort(indices.begin(), indices.end(), [&](int a, int b) {
                return denominations[a] < denominations[b];
                });
        }

        vector<int> sortedDenom, sortedAvail;
        for (int idx : indices) {
            sortedDenom.push_back(denominations[idx]);
            sortedAvail.push_back(available[idx]);
        }

        vector<int> tempTaken(sortedDenom.size(), 0);
        solved = MAXMIN(sortedDenom, sortedAvail, tempTaken, amount, 0, strategy);

        if (solved) {
            for (size_t i = 0; i < indices.size(); i++) {
                taken[indices[i]] = tempTaken[i];
            }
        }
    }

    string output = "[\n{\n\"dispense\": [";
    if (solved) {
        bool first = true;
        for (size_t i = 0; i < taken.size(); i++) {
            if (taken[i] > 0) {
                if (!first) output += ", ";
                output += "[" + to_string(denominations[i]) + ", " + to_string(taken[i]) + "]";
                first = false;
            }
        }
    }
    output += "]\n}\n]";

    ofstream outFile("output.json");
    if (outFile.is_open()) {
        outFile << output;
        outFile.close();
    }

    cout << output << endl;

    return 0;
}