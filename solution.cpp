#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdint.h>


using namespace std;

bool g_debugOn = false;
bool g_assertOn = true;
void AssertImpl(bool valid, int linenumber)
{
    if ((g_debugOn||g_assertOn) && !valid)
    {
        cout << "ASSERT AT LINE: " << linenumber << endl;
    }
}

#define ASSERT(expr) {AssertImpl(!!(expr), __LINE__);}


struct subsequence
{
    int startIndex;
    int length;
    int64_t sum;
};


struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

class Cache
{
    unordered_map<pair<int, int>, subsequence, pair_hash> _cache;
    unordered_set<pair<int, int>, pair_hash> _deadPaths;

public:
    void insert(int start, int end, const subsequence& seq)
    {
        _cache[{start, end}] = seq;
    }

    bool lookup(int start, int end, subsequence& result)
    {
        auto itor = _cache.find({ start,end });
        if (itor != _cache.end())
        {
            result = itor->second;
            return true;
        }
        return false;
    }

    void addDeadPath(int start, int end)
    {
        _deadPaths.insert({ start,end });
    }

    bool isDeadPath(int start, int end)
    {
        return (_deadPaths.find({ start,end }) != _deadPaths.end());
    }
};

bool recursiveReduction(const vector<int>& items, int64_t maxSum, int start, int end, int64_t currentSum, subsequence& result, Cache& cache)
{
    if (g_debugOn)
    {
        cout << "start=" << start << "   end=" << end << endl;
    }

    // lookup in the cache so we don't recurse needlessly
    if (cache.lookup(start, end, result))
    {
        return true;
    }

    if (cache.isDeadPath(start, end))
    {
        return false;
    }

    if (end < start)
    {
        // THIS SHOULD NEVER HAPPEN
        ASSERT(false);
        return false;
    }

    if (currentSum <= maxSum)
    {
        // stop - we've found a solution that isn't going to get longer
        result.length = end - start + 1;
        result.sum = currentSum;
        result.startIndex = start;

        cache.insert(start, end, result);
        return true;
    }


    if (start == end)
    {
        ASSERT(currentSum == items[start]);
        cache.addDeadPath(start, end);
        return false;
    }

    subsequence rightResult = {};
    subsequence leftResult = {};
    bool canReduceLeft, canReduceRight;

    canReduceLeft = recursiveReduction(items, maxSum, start + 1, end, currentSum - items[start], leftResult, cache);
    canReduceRight = recursiveReduction(items,maxSum, start, end - 1, currentSum - items[end], rightResult, cache);


    if (canReduceLeft && canReduceRight)
    {
        if (leftResult.length == rightResult.length)
        {
            if (leftResult.startIndex <= rightResult.startIndex)
            {
                result = leftResult;
            }
            else
            {
                result = rightResult;
            }
        }
        else if (leftResult.length >= rightResult.length)
        {
            result = leftResult;
        }
        else
        {
            result = rightResult;
        }
        cache.insert(start, end, result);
        return true;
    }
    else if (canReduceLeft)
    {
        result = leftResult;
        cache.insert(start, end, result);
        return true;
    }
    else if (canReduceRight)
    {
        result = rightResult;
        cache.insert(start, end, result);
        return true;
    }
    else
    {
        cache.addDeadPath(start, end);
        return false;
    }
}

bool useRecursiveReduction(const vector<int>& items, int maxSum, subsequence& result)
{
    Cache cache;
    int64_t fullSum = 0;
    for_each(items.begin(), items.end(), [&fullSum](int x) {fullSum += x; });

    return recursiveReduction(items, maxSum, 0, items.size()-1, fullSum, result, cache);
}


bool bruteForce(const vector<int>& items, int maxSum, subsequence& result)
{
    subsequence best = {};

    int N = items.size();

    for (int i = 0; i < N; i++)
    {
        if ((N - i) <= best.length)
        {
            // no point in continuing if we can't do any better
            break;
        }

        subsequence current;
        current.length = 0;
        current.startIndex = i;
        current.sum = 0;

        for (int j = i; j < N; j++)
        {
            current.sum += items[j];
            current.length += 1;
            if ((current.sum <= maxSum) && (current.length > best.length))
            {
                best = current;
            }
        }
    }

    result = best;

    return (result.length > 0);
}


int runtestcase(int seed)
{
    srand(seed);
   

    vector<int> items(10000);
    int sum = 0;

    for (int i = 0; i < 10000; i++)
    {
        items[i] = (rand() % 75) - 25;
        sum += items[i];

        if ((sum < 100) && (items[i] > 0))
        {
            items[i] = -items[i];
        }
        else if ((sum > 100) && (items[i] < 0))
        {
            items[i] = -items[0];
        }
    }

    subsequence result;
    useRecursiveReduction(items, 50, result);

    cout << result.length << " " << (result.startIndex + 1) << endl;
    return 0;

}


int main()
{
    int N, S;

    bool useBruteForce = false; // for correctness validation

    cin >> N >> S;
    vector<int> items(N);
    subsequence result;

    for (int i = 0; i < N; i++)
    {
        cin >> items[i];
    }

    if (useBruteForce)
    {
        bruteForce(items, S, result);
    }
    else
    {
        useRecursiveReduction(items, S, result);
    }
    cout << result.length << " " << (result.startIndex + 1) << endl;
}





