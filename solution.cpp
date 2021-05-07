#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <stdint.h>


using namespace std;

struct subsequence
{
    int startIndex;
    int length;
    int64_t sum;
};

struct node
{
    bool isLeaf;
    shared_ptr<node> left;
    shared_ptr<node> right;
    size_t indexFirst; // index of first element from array
    size_t indexLast;  // index of last element from array
    int64_t minValue;
    int64_t maxValue;


    // initialize a leaf node that represents a small subsequence of items
    // in a summation array
    // iFirst: first index in the summation array
    // iLast: last index in the summation array
    // smallest: smallest value in this subsequence
    // largest: largest value in this subsequence
    node(size_t iFirst, size_t iLast, int64_t smallest, int64_t largest) :
        isLeaf(true),
        left(nullptr),
        right(nullptr),
        indexFirst(iFirst),
        indexLast(iLast),
        minValue(smallest),
        maxValue(largest)
    {
    }

    // initialize a parent node from a pair of child nodes
    node(const shared_ptr<node>& l, const shared_ptr<node>& r) :
        isLeaf(false),
        left(l),
        right(r)
    {

        // working assumption is that left and right are both non-null
        // We can trivially allow parent node with one child if we fix the min/max first/last
        // calculations below. But for now, just assume both are non-null
        indexFirst = l->indexFirst;
        indexLast = r->indexLast;
        minValue = (l->minValue < r->minValue) ? l->minValue : r->minValue;
        maxValue = (l->maxValue > r->maxValue) ? l->maxValue : r->maxValue;
    }

    // do a binary search for the largest INDEX in the array that is <= maxSum
    // ignore any indices that are less than "leftEdge"
    // return true on success and assign the discovered index valut to resultIndex
    // otherwise, return false, and leave resultIndex unchanged
    bool search(const vector<int64_t>& summations, int64_t maxSum, size_t leftEdge, size_t& resultIndex)
    {
        if (this->indexLast < leftEdge)
        {
            return false;
        }

        if (maxSum < this->minValue)
        {
            return false;
        }

        if (this->isLeaf)
        {
            // seach the summation array looking for the largestIndex referencing a value <= maxSum

            bool found = false;

            for (size_t i = indexFirst; i <= indexLast; i++)
            {
                if (summations[i] <= maxSum)
                {
                    found = true;
                    resultIndex = i;
                }
            }
            return found;
        }

        if (this->right != nullptr)
        {
            if (this->right->search(summations, maxSum, leftEdge, resultIndex))
            {
                return true;
            }
        }

        if (this->left != nullptr)
        {
            return this->left->search(summations, maxSum, leftEdge, resultIndex);
        }

        // assert - we should never get here
        return false;
    }
};


void buildRunningSumArray(const vector<int>& items, vector<int64_t>& summations)
{
    summations.clear();
    summations.resize(items.size());
    const size_t len = items.size();
    int64_t sum = 0;

    for (size_t i = 0; i < len; i++)
    {
        sum += items[i];
        summations[i] = sum;
    }
}

void buildLeafRow(vector<int64_t>& summations, vector<shared_ptr<node>>& leafrow)
{

    size_t pairCount = summations.size() / 2;  // number of non-leaf nodes to add to leafrow
    bool oddCount = summations.size() % 2;             // if odd number, we'll treat the last as a leaf node
    size_t oddIndex = pairCount * 2;

    for (size_t index = 0; index < pairCount; index++)
    {
        size_t firstIndex = index * 2;
        size_t lastIndex = firstIndex + 1;

        int64_t smallestValue = summations[firstIndex];
        int64_t largestValue = summations[lastIndex];
        if (smallestValue > largestValue)
        {
            swap(smallestValue, largestValue);
        }

        auto spNode = make_shared<node>(firstIndex, lastIndex, smallestValue, largestValue);
        leafrow.push_back(spNode);
    }

    if (oddCount)
    {
        // tack on the last node as a single item leaf node
        auto spNode = make_shared<node>(oddIndex, oddIndex, summations[oddIndex], summations[oddIndex]);
        leafrow.push_back(spNode);
    }

}

bool buildParentRow(const vector<shared_ptr<node>>& childRow, vector<shared_ptr<node>>& parentRow)
{
    if (childRow.size() <= 1)
    {
        return false;
    }

    size_t pairCount = childRow.size() / 2;  // number of non-leaf nodes to add to leafrow
    bool oddCount = childRow.size() % 2;         // if odd number, we'll promote the last node to the parent row
    size_t oddIndex = pairCount * 2;

    for (size_t index = 0; index < pairCount; index++)
    {
        size_t childIndex = index * 2;
        auto spNode = make_shared<node>(childRow[childIndex], childRow[childIndex + 1]);
        parentRow.push_back(spNode);
    }

    if (oddCount)
    {
        // simply promote the last child node to the parent row
        parentRow.push_back(childRow[oddIndex]);
    }

    return true;
}

bool solveWithBinaryTree(const vector<int>& items, int64_t maxSum, subsequence& result)
{
    vector<int64_t> summations;

    size_t esimatedNumberOfRows = 2;
    size_t tmp = items.size();
    while (tmp > 0)
    {
        esimatedNumberOfRows++;
        tmp /= 2;
    }

    // a vector of parent rows just to hold the nodes in memory
    vector<vector<shared_ptr<node>>> rows;
    rows.reserve(esimatedNumberOfRows);
    rows.resize(1);

    buildRunningSumArray(items, summations);
    buildLeafRow(summations, rows[0]);

    // build the tree up
    bool keepGoing = true;
    size_t lastRowIndex = 0;
    while (keepGoing)
    {
        rows.resize(rows.size() + 1);
        auto& previousRow = rows[rows.size() - 2];
        auto& newRow = rows[rows.size() - 1];
        keepGoing = buildParentRow(previousRow, newRow);
        if (keepGoing)
        {
            lastRowIndex = rows.size() - 1;
        }
    }


    auto spRootNodeRow = rows[lastRowIndex];
    auto spRootNode = spRootNodeRow[0];


    // now comes the fun part
    // consider every index to be the starting point of the longest sequence, adjusting maxSum as we go along

    int64_t target = maxSum;
    size_t bestStart = 0;
    size_t bestLength = 0;
    int64_t bestSum = 0;
    int64_t dropSum = 0;
    for (size_t i = 0; i < summations.size(); i++)
    {
        size_t bestPossibleLength = summations.size() - i;
        if (bestLength >= bestPossibleLength)
        {
            break; // no point in continuing
        }

        size_t resultIndex = 0;
        bool resultFound = spRootNode->search(summations, target, i, resultIndex);

        if (resultFound)
        {
            size_t length = resultIndex - i + 1;
            if (length > bestLength)
            {
                bestStart = i;
                bestLength = length;
                bestSum = summations[resultIndex] - dropSum;
            }
        }

        target += items[i]; // add from the items array, not the summations array
        dropSum += items[i];
    }

    result.length = bestLength;
    result.startIndex = bestStart;
    result.sum = bestSum;
    return true;
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

    solveWithBinaryTree(items, S, result);

    cout << result.length << " " << (result.startIndex + 1) << endl;
}
