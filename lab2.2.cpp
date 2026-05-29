#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

vector<pair<int, int>> parseWallet(const string& s) {
    vector<pair<int, int>> w;
    size_t p = 0;
    while ((p = s.find('[', p)) != string::npos) {
        size_t e = s.find(']', ++p);
        string item = s.substr(p, e - p);
        size_t c = item.find(',');
        if (c != string::npos)
            w.push_back({ stoi(item.substr(0,c)), stoi(item.substr(c + 1)) });
        p = e + 1;
    }
    return w;
}

int parseAmount(const string& s) {
    size_t p = s.find("\"amount\"");
    if (p == string::npos) return 0;
    p = s.find(':', p) + 1;
    while (p < s.length() && (s[p] == ' ' || s[p] == '\t')) p++;
    return stoi(s.substr(p));
}

string parseStrategy(const string& s) {
    size_t p = s.find("\"strategy\"");
    if (p == string::npos) return "";
    p = s.find(':', p) + 1;
    while (p < s.length() && (s[p] == ' ' || s[p] == '\t')) p++;
    p = s.find('"', p) + 1;
    size_t e = s.find('"', p);
    return s.substr(p, e - p);
}

bool solveMAX(vector<pair<int, int>>& w, int amount, vector<pair<int, int>>& res) {
    sort(w.begin(), w.end(), greater<pair<int, int>>());
    int rem = amount;
    vector<int> used(w.size(), 0);
    for (size_t i = 0; i < w.size(); i++) {
        int take = min(w[i].second, rem / w[i].first);
        used[i] = take;
        rem -= take * w[i].first;
    }
    if (rem != 0) return false;
    for (size_t i = 0; i < w.size(); i++)
        if (used[i] > 0) res.push_back({ w[i].first, used[i] });
    return true;
}

bool solveMIN(vector<pair<int, int>>& w, int amount, vector<pair<int, int>>& res) {
    sort(w.begin(), w.end());
    int n = w.size();
    vector<int> denom(n), avail(n);
    for (int i = 0; i < n; i++) {
        denom[i] = w[i].first;
        avail[i] = w[i].second;
    }

    vector<int> dp(amount + 1, -1);
    dp[0] = 0;

    for (int i = 0; i < n; i++) {
        for (int k = 1; k <= avail[i]; k++) {
            int val = denom[i];
            for (int s = amount; s >= val; s--) {
                if (dp[s - val] != -1 && dp[s] == -1)
                    dp[s] = i;
            }
        }
    }

    if (dp[amount] == -1) return false;

    int rem = amount;
    vector<int> cnt(n, 0);
    while (rem > 0) {
        int idx = dp[rem];
        cnt[idx]++;
        rem -= denom[idx];
    }

    for (int i = 0; i < n; i++)
        if (cnt[i] > 0) res.push_back({ denom[i], cnt[i] });

    sort(res.begin(), res.end(), greater<pair<int, int>>());
    return true;
}

bool solveUNIFORM(vector<pair<int, int>>& w, int amount, vector<pair<int, int>>& res) {
    sort(w.begin(), w.end());
    int n = w.size();
    vector<int> denom(n), avail(n);
    for (int i = 0; i < n; i++) {
        denom[i] = w[i].first;
        avail[i] = w[i].second;
    }

    vector<int> best(n, 0);
    int bestDiff = 1e9;
    bool found = false;

    function<void(int, int, vector<int>&)> dfs = [&](int idx, int rem, vector<int>& used) {
        if (rem == 0) {
            int mx = 0;
            for (int i = 0; i < n; i++) if (used[i] > 0) mx = max(mx, used[i]);
            int diff = mx;
            if (diff < bestDiff) {
                bestDiff = diff;
                best = used;
                found = true;
            }
            return;
        }
        if (idx >= n) return;
        int maxTake = min(avail[idx], rem / denom[idx]);
        for (int t = maxTake; t >= 0; t--) {
            used[idx] = t;
            dfs(idx + 1, rem - t * denom[idx], used);
            used[idx] = 0;
        }
        };

    vector<int> used(n, 0);
    dfs(0, amount, used);

    if (!found) return false;
    for (int i = 0; i < n; i++)
        if (best[i] > 0) res.push_back({ denom[i], best[i] });
    sort(res.begin(), res.end(), greater<pair<int, int>>());
    return true;
}

int main() {
    ifstream f("input.json");
    if (!f.is_open()) return 1;
    string s((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    f.close();

    auto w = parseWallet(s);
    int amount = parseAmount(s);
    string strategy = parseStrategy(s);

    vector<pair<int, int>> res;
    bool ok = false;

    if (strategy == "MAX") ok = solveMAX(w, amount, res);
    else if (strategy == "MIN") ok = solveMIN(w, amount, res);
    else if (strategy == "UNIFORM") ok = solveUNIFORM(w, amount, res);

    if (!ok) res.clear();

    ofstream out("output.json");
    out << "[\n  {\n    \"dispense\": [";
    for (size_t i = 0; i < res.size(); i++) {
        if (i > 0) out << ", ";
        out << "[" << res[i].first << ", " << res[i].second << "]";
    }
    out << "]\n  }\n]\n";
    out.close();

    return 0;
}