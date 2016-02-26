#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <queue>
#include <ctime>
#include "omp.h"
using namespace std;

using Lines = std::vector<std::string>;

struct Result
{
    Result(int lineNumber_, int firstChar_, int length_)
            : lineNumber(lineNumber_), firstChar(firstChar_), length(length_)
    {}

    // This allows you to compare results with the < (less then) operator, i.e. r1 < r2
    bool
    operator<(Result const& o)
    {
        // Line number can't be equal
        return length < o.length ||
               (length == o.length && lineNumber <  o.lineNumber) ||
               (length == o.length && lineNumber == o.lineNumber  && firstChar < o.firstChar);
    }

    int lineNumber, firstChar, length;
};

// Removes all non letter characters from the input file and stores the result in a Lines container
Lines
strip(std::ifstream& file)
{
    Lines result;
    result.reserve(50000); // If reading is too slow try increasing this value

    std::string workString;

    while(std::getline(file,workString))
    {
        //Strip non alpha characters
        workString.erase(std::remove_if(workString.begin(), workString.end(),
                                        [] (char c) { return !std::isalpha(c); }
        ), workString.end());
        result.push_back(workString);
        workString.clear();
    }
    return result;
}

// CHANGE This Code (you can add more functions)-----------------------------------------------------------------------------
Result SearchFromCentre(std::string line, int lineNum){
    int bestLen = 0;
    int bestStart = 0;
    int lineNumber = lineNum;
        for(int centre= 0 ; centre < line.length(); centre++)
        {//pick a centre to expand from
            int i = centre -1;
            int j = centre +1;
            bool stillPal = true;
            while ((i>=0&&j<line.length())&&stillPal)
            {
                if (line[i]==line[j])
                {
                    i--;
                    j++;
                }
                else if (j-i>bestLen)
                {
                    bestStart = i;
                    bestLen = j-i;
                    stillPal = false;
                }
                else stillPal = false;
            }
            if (stillPal)
            {
                int x = (j<line.length()) ? j-i: line.length()-i;
                int y = (i>0) ? i : 0;
                if (x-y>bestLen) {
                    bestStart = (i > 0) ? i : 0;
                    bestLen = (j < line.length()) ? j - i : line.length() - i;
                    stillPal = false;
                }
            }
            //Searchs for an even length palindrome
            if (centre< line.length()-1){
                if (line[centre]==line[centre+1]) {
                    i = centre - 1;
                    j = centre + 2;
                    stillPal = true;
                    while (i >= 0 && j < line.length() && stillPal) {
                        if (line[i] == line[j]) {
                            i--;
                            j++;
                        }
                        else if (j - i > bestLen) {
                            bestStart = i;
                            bestLen = j - i;
                            stillPal = false;
                        }
                        else stillPal = false;

                    }
                    if (stillPal) {
                        int x = (j < line.length()) ? j - i : line.length() - i;
                        int y = (i > 0) ? i : 0;
                        if (x - y > bestLen) {
                            bestStart = (i > 0) ? i : 0;
                            bestLen = x - y;
                            stillPal = false;
                        }
                    }
                }
            }
        }
    Result res = {0,0,0};
    res.lineNumber = lineNumber;
    res.firstChar = bestStart+1;//+1
    res.length = bestLen-1;//-1
    //std::cout << "\nres in func len, lineNum, start: "<< res.length<< " "<< res.lineNumber<< "  "<< res.firstChar<<std::endl;
    return res;
}



Result FindPalindromeStatic(Lines const& lines, int numThreads){
    vector<Result> results;
    for (int i = 0; i< numThreads; i++){
        results.push_back({0,0,0});
    }
    #pragma omp for schedule(static) nowait
    for (int i = 0; i<lines.size(); i++){
        Result palindrome = SearchFromCentre(lines[i], i);
        if (palindrome>results[omp_get_thread_num()])
        {
            results[omp_get_thread_num()]  = palindrome;
        }
        cout<<"thread running: "<<omp_get_thread_num<<endl;
    }
    cout<<"does no wait keep going!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
    Result largestPal = {0,0,0};
    for (auto &var : results)
    {
        if (largestPal<var) largestPal = var;
    }
    return largestPal;
}




// PART B
Result
FindPalindromeDynamic(Lines const& lines, int numThreads, int chunkSize)
{
    vector<Result> results;
    for (int i = 0; i< numThreads; i++){
        results.push_back({0,0,0});
    }
    #pragma omp for schedule(static, chunkSize) nowait
    for (int i = 0; i<lines.size(); i++){
        Result palindrome = SearchFromCentre(lines[i], i);
        if (results[omp_get_thread_num()]<palindrome){
            results[omp_get_thread_num()]  = palindrome;
        }
    cout<<"thread running: "<<omp_get_thread_num<<endl;
    }
    cout<<"does no wait keep going!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
    Result largestPal = {0,0,0};
    for (auto &var : results)
    {
        if (largestPal<var) largestPal = var;
    }
    return largestPal;
}

// DONT CHANGE THIS -----------------------------------------------------------------------------------------------------------------

int
main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "ERROR: Incorrect number of arguments. Format is: <filename> <numThreads> <chunkSize>" <<
        std::endl;
        return 0;
    }

    std::ifstream theFile(argv[1]);
    if (!theFile.is_open()) {
        std::cout << "ERROR: Could not open file " << argv[1] << std::endl;
        return 0;
    }
    int numThreads = std::atoi(argv[2]);
    int chunkSize = std::atoi(argv[3]);

    // std::cout << "Process " << argv[1] << " with " << numThreads << " threads using a chunkSize of " << chunkSize << " for dynamic scheduling\n" << std::endl;

    Lines lines = strip(theFile);

    //Part A
    Result aResult = FindPalindromeStatic(lines, numThreads);
    std::clock_t start;
    std::clock_t pb;
    double durb;
    double duration;
    start = std::clock();
    std::cout << "PartA: " << aResult.lineNumber << " " << aResult.firstChar << " " << aResult.length << ":\t" <<
    lines.at(aResult.lineNumber).substr(aResult.firstChar, aResult.length) << std::endl;
    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << std::fixed << "Part A time: " << duration << '\n';
    //Part B
    pb = std::clock();
    Result bResult = FindPalindromeDynamic(lines, numThreads, chunkSize);
    std::cout << "PartB: " << bResult.lineNumber << " " << bResult.firstChar << " " << bResult.length << ":\t" <<
    lines.at(bResult.lineNumber).substr(bResult.firstChar, bResult.length) << std::endl;
    durb = (std::clock() - pb) / (double) CLOCKS_PER_SEC;
    std::cout << "Part B time: " << durb << '\n';
    return 0;
}