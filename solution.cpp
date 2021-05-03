#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <stdint.h>

using namespace std;

struct subsequence
{
    int startIndex;
    int length;
    int64_t sum;
};


bool g_debugOn = false;

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
    unordered_map<std::pair<int, int>, subsequence, pair_hash> _cache;

public:
    void insert(int start, int end, const subsequence& seq)
    {
        _cache[{start, end}] = seq;
    }

    bool lookup(int start, int end, subsequence& seq)
    {
        auto itor = _cache.find({ start,end });
        if (itor != _cache.end())
        {
            seq = itor->second;
            return true;
        }
        return false;
    }
};


bool recursiveReduction(const vector<int>& items, int N, int64_t maxSum, int start, int end, int currentSum, subsequence& result, Cache& cache)
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

    if (end < start)
    {
        // THIS SHOULD NEVER HAPPEN
        result = {};
        result.sum = maxSum + 1;
        //ASSERT(FALSE);
        if (g_debugOn)
        {
            cout << "ERROR" << endl;
        }
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


    if (left == right)
    {
        result.length = 1;
        result.sum = currentSum;
        result.startIndex = start;
        // ASSERT(currentSum == items[start])

        return(currentSum <= maxSum);
    }

    subsequence rightResult = {};
    subsequence leftResult = {};
    bool canReduceLeft, canReduceRight;

    canReduceLeft = recursiveReduction(items, N, maxSum, start + 1, end, currentSum - items[start], leftResult, cache);
    canReduceRight = recursiveReduction(items, N, maxSum, start, end - 1, currentSum - items[end], rightResult, cache);


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

    return false;
}

bool useRecursiveReduction(const vector<int>& items, int N, int maxSum, subsequence& result)
{
    Cache cache;
    int fullSum = 0;
    for_each(items.begin(), items.end(), [&fullSum](int x) {fullSum += x; });

    return recursiveReduction(items, N, maxSum, 0, N - 1, fullSum, result, cache);
}


bool bruteForce(const vector<int>& items, int N, int maxSum, subsequence& result)
{
    subsequence best = {};

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
        bruteForce(items, N, S, result);
    }
    else
    {
        useRecursiveReduction(items, N, S, result);
    }
    cout << result.length << " " << (result.startIndex + 1) << endl;
}

