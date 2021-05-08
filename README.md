# ULS1902
Solution for: https://www.spoj.com/CSMS/problems/ULS1902/  Was accepted as a valid solution by the site.


From a given table with N members, find the longest sub-sequence whose sum does not exceed S. A sub-sequence is a set of consecutive numbers in a base table.

Restrictions:

N≤500,000

S≤1,000,000,000

−1,000,000≤Ai≤1,000,000 (Value of Sequence Members)

Input
In the first line of the test, the numbers N and S are separated by a space. The next line contains N numbers representing the members of the sequence.

Output
On one line, print the length of the longest sub-sequence whose sum does not exceed S and its starting number. If there are multiple answers, print the appropriate number first. (Numbered starting from Table 1)

Note: The longest sub-sequence whose sum does not exceed 666 is a sequence of 10 lengths starting from the 2nd and 3rd members of the table, but the answer is 10 2 because the number at the beginning is printed.


===========================================

**General technique:**

**Step 1: initialize from input**

Taking the sample values that the OP gave as the input set

    15 666
    101 42 -132 17 404 -13 55 222 89 11 -66 91 -9 21 4

It's easy to initialize our values for N, S, and A.  That's the easy part:

    int N, S;  // N=count of items to be read, S is the max sum constraint of the longest subsequence
    vector<int> A;  // array of integers, vector is convenient in C++
    
    cin >> N >> S;
    
    A.resize(N);
    
    for (int i = 0; i < N; i++)
    {
        cin >> A[i];
    }

**Step 2: Create a summation array from the original input array.**

A summation array is an array of equal length to `A`.  Each element `SUM[i]` is assigned the following value:

     SUM[0] = A[0]
     SUM[1] = SUM[0] + A[1]
     SUM[2] = SUM[1] + A[2]
     SUM[3] = SUM[2] + A[3]
     ...

It's essentially an array that represents the rolling summations of values in `A`.

    vector<int64_t> summations;
    int64_t sum = 0;
    for (int i = 0; i < N; i++)
    {
       sum += A[i];
       summations.push_back(sum);   
    }

Example, the original items array:

             A[] = 101  42 -132 17 404 -13  55 222  89  11 -66  91  -9  21   4

Becomes:

    summations[] = 101 143   11 28 432 419 474 696 785 796 730 821 812 833 837 


Now to find the longest subsequence (with sum <= `S`) that starts at `A[0]`, the code only has to scan from the right of the `summations` array until the first value `<=S` is found. If not such value is found after evaluating `summations[0]`, there is no viable sequence starting at index 0.  In this case, the first value from the right of the summations array `<= 666` is at `summations[6]`, which is `474`.  Hence, the sequence from `A[0] - A[6]` (length: `7`) has a sum of `474` and is a candidate for longest subsequence with sum less than `S` (666).

Then to test if `A[1]` has a better candidate sequence, no modifications to the arrays are needed. Simply add `A[0]` to `S` and repeat the scan from the right again until a value `<=S` is encountered or until index 1 is hit.  In this case, scanning for a value less than `767` is at `summations[11]`, which is `730`.  So the `10` item sequence from `A[1] to A[11]` is now the best sequence found.

We can repeat this algorithm for the entire array of items. Again, adding `A[1]` to `S` and repeating the scan from `summations[N-1]` and go as far as `summations[2]` to to determine the longest sequence starting at `A[2]`.... 

However, for large input arrays, **this won't scale**. The problem states that the input array may have over 500000 items in it. That will take a long time to scan, even with some optimizations.  And it still has a `O(N²)` running time. The algorithm will repeatedly be re-evaluating the same numbers linearly from the end of the sequence array.  There's got to be a better way to find the value in the summations array closest to the end that is <= to the value imposed by S. So let's explore that....

**Step 3: Build a binary tree from the summations array**

Now build a binary tree.  Each node contains 4 values: the low/high index values of the items in the summations array as well as the smallest and largest elements within that sequence range.  Let's see if we can draw out what this might look like in ascii

```
                                                                                            {[0-14],11,837}
                                                         /------------------------------------             --------------------------------\
                                           {[0-7],11,696}                                                                                   {[8-14],730,837}
                              /------------             -----------\                                                             /-----------              ---------\
                {[0-3],11,143}                                      {[4-7],419,696}                               {[8-11],730,821}                                   {[12-14],812,837}  
               /             \                                      /             \                               /              \                                   /               \
{[0-1],101,143}               {[2-3],11,28}          {[4-5],419,432}               {[6-7],474,696}  {[8-9],785,796}               {[10-11,730,821}  {[12-13],812,833}                 {[14-14],837,837}
    101 143                        11 28                  432 419                       474 696          785 796                        30 821           812 833                             837 
```



**Step 4 - use the binary tree to search for the last occurrence of a value <= `S` in summations array**

Now with the above binary tree, we can search for the last value in the `summations` array that is less than or equal to `S` (or any value). Simply start at the root node and do a depth-fist traversal by visiting the **right node first** stopping when you hit a leaf node that references an item in `summations` that qualifies.

So to start, we can easily traverse down the tree at the root node looking for the largest index <= `666`.  We look at the first right child, `{[8-14],730,837}` and see that the target value, 666 won't be found, so we visit `{[0-7],11,696}` next, then `{[4-7],419,696}`, then `{[6-7],474,696}` and then we can easily find `474` at index 6.

Subsequent iterations are similar to the array traversal method in step 2.  To evaluate A[1] as the starting point, we need to add `A[0]` back to `S` and traverse the tree again. One notable exception this time is that we have to designate a "left edge" of `1` so we don't consider values before the start index. Any traversal to a value less than the left edge should not be done.  As additional iterations are done, the left edge increments as well.

Code for the search would look something like this:

```

struct node
{
    bool isLeaf;
    shared_ptr<node> left;
    shared_ptr<node> right;
    size_t indexFirst; // index of first element from array
    size_t indexLast;  // index of last element from array
    int64_t minValue;
    int64_t maxValue;

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
            // seach the summation array looking for the largest index referencing a value <= maxSum

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

        // visit the right node first to find values that are less than or equal to maxSum, but at higher index numbers
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
        ASSERT(false);
        return false;
    }
}
```

And then the code to repeat the search for subsequent starting index points:

    auto spRootNode = previousRow[0];

    // now comes the fun part
    // consider every index to be the starting point of the longest sequence, adjusting maxSum as we go along

    int64_t target = S; // MAX SUM target
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

        target += A[i]; // add from the items array, not the summations array
        dropSum += A[i];
    }



The above runs in approximately **O(N lg N)** time.  **O(N)** to build the summation array.  Approximately **O(N)** to build the binary tree.  Then in the worse case scenario, **N** searches on the binary tree, each taking **lg N** time.  Hence **O(N)+O(N)+O(N lg N) ==> O(N lg N)**



