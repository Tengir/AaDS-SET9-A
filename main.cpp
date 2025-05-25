#include <bits/stdc++.h>
using namespace std;

/* глобальный счётчик сравнений */
static uint64_t charCmpCnt = 0;

/* вспом. функция сравнения двух строк */
inline bool lexLessCount(const string &a, const string &b) {
    size_t i = 0, na = a.size(), nb = b.size();
    while (i < na && i < nb) {
        ++charCmpCnt;
        if (a[i] != b[i]) return a[i] < b[i];
        ++i;
    }
    ++charCmpCnt; // сравнение 'кончилась ли строка'
    return na < nb;
}

/* ========================================================= */
class StringGenerator {
    static constexpr char alphabet[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#%:;^&*()-.";
    static constexpr int ALPHA = 74;
    mt19937_64 rng;
    uniform_int_distribution<int> lenDist{10, 200};
    uniform_int_distribution<int> chDist{0, ALPHA - 1};

    string randStr() {
        int L = lenDist(rng);
        string s;
        s.reserve(L);
        for (int i = 0; i < L; ++i) s += alphabet[chDist(rng)];
        return s;
    }

public:
    explicit StringGenerator(uint64_t seed = 0)
        : rng(seed ? seed : chrono::steady_clock::now().time_since_epoch().count()) {
    }

    vector<string> baseRandom(int n) {
        vector<string> v(n);
        generate(v.begin(), v.end(), [&] { return randStr(); });
        return v;
    }

    vector<string> baseReversed(int n) {
        auto v = baseRandom(n);
        sort(v.begin(), v.end(), lexLessCount);
        reverse(v.begin(), v.end());
        return v;
    }

    vector<string> baseAlmost(int n) {
        auto v = baseRandom(n);
        sort(v.begin(), v.end(), lexLessCount);
        int swapCnt = n / 50;
        uniform_int_distribution<int> idx(0, n - 1);
        for (int i = 0; i < swapCnt; ++i) swap(v[idx(rng)], v[idx(rng)]);
        return v;
    }
};

/* ========================================================= */
void stdQuick(vector<string> &a) {
    sort(a.begin(), a.end(), [](const string &x, const string &y) { return lexLessCount(x, y); });
}

/* простой рекурсивный MergeSort */
void mergeSimple(vector<string> &a, vector<string> &buf, int l, int r) {
    if (r - l < 2) return;
    int m = (l + r) >> 1;
    mergeSimple(a, buf, l, m);
    mergeSimple(a, buf, m, r);
    int i = l, j = m, k = l;
    while (i < m || j < r) {
        if (j == r || (i < m && lexLessCount(a[i], a[j]))) buf[k++] = a[i++];
        else buf[k++] = a[j++];
    }
    for (int t = l; t < r; ++t) a[t] = move(buf[t]);
}

void stdMerge(vector<string> &a) {
    vector<string> buf(a.size());
    mergeSimple(a, buf, 0, a.size());
}

/* ========================================================= */
/* ---- 2.1  Тернарный STRING QUICKSORT ---- */
int charAtQS(const string &s, int d) { return d < (int) s.size() ? (unsigned char) s[d] : -1; }

void qsRec(vector<string> &a, int l, int r, int d) {
    if (r <= l) return;
    int lt = l, gt = r, v = charAtQS(a[l], d), i = l + 1;
    while (i <= gt) {
        int t = charAtQS(a[i], d);
        charCmpCnt++; // сравнение t<v  или t>v (первое)
        if (t < v) {
            swap(a[lt++], a[i++]);
            continue;
        }
        charCmpCnt++; // второе сравнение t>v
        if (t > v) { swap(a[i], a[gt--]); } else ++i;
    }
    qsRec(a, l, lt - 1, d);
    if (v >= 0) qsRec(a, lt, gt, d + 1);
    qsRec(a, gt + 1, r, d);
}

void ternarySQS(vector<string> &a) { if (!a.empty()) qsRec(a, 0, a.size() - 1, 0); }

/* ---- 2.2  STRING MERGESORT with LCP ---- */
int lcp(const string &a, const string &b, int start = 0) {
    int i = start, n = a.size(), m = b.size();
    while (i < n && i < m) {
        ++charCmpCnt;
        if (a[i] != b[i]) break;
        ++i;
    }
    return i;
}

void mergeLcp(vector<string> &a, vector<string> &buf, int l, int m, int r) {
    int i = l, j = m, k = l, pi = 0, pj = 0;
    while (i < m || j < r) {
        if (i == m) {
            buf[k++] = a[j++];
            pj = 0;
            continue;
        }
        if (j == r) {
            buf[k++] = a[i++];
            pi = 0;
            continue;
        }
        int skip = min(pi, pj);
        int p = lcp(a[i], a[j], skip);
        bool takeLeft;
        if (p == a[i].size() || p == a[j].size()) {
            ++charCmpCnt;
            takeLeft = a[i].size() < a[j].size();
        } else {
            ++charCmpCnt;
            takeLeft = a[i][p] < a[j][p];
        }
        if (takeLeft) {
            buf[k++] = a[i++];
            pi = p;
            pj = 0;
        } else {
            buf[k++] = a[j++];
            pj = p;
            pi = 0;
        }
    }
    for (int t = l; t < r; ++t) a[t] = move(buf[t]);
}

void msLcp(vector<string> &a, vector<string> &buf, int l, int r) {
    if (r - l < 2) return;
    int m = (l + r) >> 1;
    msLcp(a, buf, l, m);
    msLcp(a, buf, m, r);
    mergeLcp(a, buf, l, m, r);
}

void stringMergeLCP(vector<string> &a) {
    vector<string> buf(a.size());
    msLcp(a, buf, 0, a.size());
}

/* ---- 2.3  MSD RADIX SORT ---- */
int keyMSD(const string &s, int d) { return d < (int) s.size() ? (unsigned char) s[d] + 1 : 0; }

void msdRec(vector<string> &a, vector<string> &buf, int l, int r, int d) {
    if (r - l <= 1) return;
    const int R = 256;
    int cnt[R + 2] = {0};
    for (int i = l; i < r; ++i) ++cnt[keyMSD(a[i], d) + 1];
    for (int i = 1; i < R + 2; ++i) cnt[i] += cnt[i - 1];
    for (int i = l; i < r; ++i) buf[cnt[keyMSD(a[i], d)]++] = move(a[i]);
    for (int i = l; i < r; ++i) a[i] = move(buf[i - l]);
    for (int i = 0; i < R + 1; ++i) msdRec(a, buf, l + cnt[i], l + cnt[i + 1], d + 1);
}

void msdRadix(vector<string> &a) {
    vector<string> buf(a.size());
    msdRec(a, buf, 0, a.size(), 0);
}

/* ---- 2.4  MSD + cut-off → SQS ---- */
const int CUTOFF = 74;

void msdCut(vector<string> &a, vector<string> &buf, int l, int r, int d) {
    if (r - l <= 1) return;
    if (r - l < CUTOFF) {
        qsRec(a, l, r - 1, d);
        return;
    }
    const int R = 256;
    int cnt[R + 2] = {0};
    for (int i = l; i < r; ++i) ++cnt[keyMSD(a[i], d) + 1];
    for (int i = 1; i < R + 2; ++i) cnt[i] += cnt[i - 1];
    for (int i = l; i < r; ++i) buf[cnt[keyMSD(a[i], d)]++] = move(a[i]);
    for (int i = l; i < r; ++i) a[i] = move(buf[i - l]);
    for (int i = 0; i < R + 1; ++i) msdCut(a, buf, l + cnt[i], l + cnt[i + 1], d + 1);
}

void msdRadixCut(vector<string> &a) {
    vector<string> buf(a.size());
    msdCut(a, buf, 0, a.size(), 0);
}

/* ========================================================= */
class StringSortTester {
    struct Algo {
        string name;
        function<void(vector<string> &)> fn;
    };

    vector<Algo> algos = {
        {"StdQuick", stdQuick},
        {"StdMerge", stdMerge},
        {"SQS", ternarySQS},
        {"MergeLCP", stringMergeLCP},
        {"MSD", msdRadix},
        {"MSD+cut", msdRadixCut}
    };
    static constexpr int RUNS = 5;
    static constexpr int MAXN = 3000;
    static constexpr int STEP = 100;

public:
    void run() {
        StringGenerator gen;
        auto baseRand = gen.baseRandom(MAXN);
        auto baseRev = gen.baseReversed(MAXN);
        auto baseAlm = gen.baseAlmost(MAXN);

        vector<pair<string, const vector<string> &> > types = {
            {"Random", baseRand},
            {"Reversed", baseRev},
            {"Almost", baseAlm}
        };

        cout << fixed << setprecision(2);
        for (auto &T: types) {
            cout << "\n=== " << T.first << " ===\n";
            cout << "size";
            for (auto &a: algos) cout << setw(12) << a.name;
            cout << setw(12) << "cmp"; // итог сравнений StdQuick (как эталон)
            cout << "\n";

            for (int n = STEP; n <= MAXN; n += STEP) {
                cout << setw(4) << n;
                uint64_t cmpEtalon = 0;
                for (auto &A: algos) {
                    long long total = 0;
                    for (int r = 0; r < RUNS; ++r) {
                        vector<string> v(T.second.begin(), T.second.begin() + n);
                        charCmpCnt = 0;
                        auto t0 = chrono::steady_clock::now();
                        A.fn(v);
                        auto t1 = chrono::steady_clock::now();
                        total += chrono::duration_cast<chrono::microseconds>(t1 - t0).count();
                        if (A.name == "StdQuick") cmpEtalon += charCmpCnt;
                    }
                    cout << setw(12) << double(total) / RUNS;
                }
                cout << setw(12) << double(cmpEtalon) / RUNS << "\n";
            }
        }
    }
};

/* ========================================================= */
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    StringSortTester().run();
    return 0;
}
