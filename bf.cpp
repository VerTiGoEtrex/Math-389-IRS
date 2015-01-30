#include <iostream>
#include <iomanip>
#include <primesieve.hpp>
#include <algorithm>
#include <deque>
#include <stack>
#include <cassert>

using namespace std;
using primesieve::PrimeSieve;

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define RES "\x1b[0m"

void printStep(const char* op) {
  cout << "\033[0;94m" << op << "\033[0m" << endl;
}

double getWallTime()
{
  return static_cast<double>(std::clock()) / CLOCKS_PER_SEC;
}

bool generateFactors(vector<vector<int>> &levels, vector<int> &primes, int multSoFar, int level, int &boardSize, int oldIndex) {
  if (multSoFar > boardSize)
    return false;
  while (levels.size() <= (size_t)level)
    levels.emplace_back();
  levels[level].push_back(multSoFar);
  //cout << multSoFar << endl;
  for (size_t i = oldIndex; i < primes.size(); ++i) {
    if (!generateFactors(levels, primes, multSoFar*primes[i], level+1, boardSize, i))
      break;
  }
  return true;
}

bool neededComp(const int &l, const int &r) {
  return l*2 <= r;
}

void generateCutoffs(vector<vector<int>> &levels, vector<int> &cutoffs, int boardSize) {
  for(auto &l : levels) {
    auto cutoff = lower_bound(l.begin(), l.end(), boardSize, neededComp);
    cutoffs.push_back(cutoff - l.begin());
  }
}

bool claimNumber(int toClaim, vector<int> &aboveLevel, int aboveCutoff, vector<bool> &taken) {
  assert(!taken[toClaim]);
  bool didTake = false;
  for (int i = 0; i < aboveCutoff; ++i) {
    if (toClaim%aboveLevel[i] == 0) {
      if (!taken[aboveLevel[i]] || !taken[toClaim/aboveLevel[i]]){
        didTake = true;
        taken[toClaim] = true;
      }
      taken[aboveLevel[i]] = true;
      taken[toClaim/aboveLevel[i]] = true;
    }
  }
  //cout << "\t\tCLAIM FOR " << toClaim << " " << didTake << endl;
  return didTake;
}

bool bruteForcePerm(vector<int> &aboveLevel, int aboveCutoff, vector<int> &moves, vector<bool> &taken, vector<int> &soFar, deque<int> &toPermute) {
  if(toPermute.empty()) {
    moves.insert(moves.end(), soFar.begin(), soFar.end());
    return true;
  }
  for (size_t i = 0; i < toPermute.size(); ++i){
    int toTry = toPermute.front();
    toPermute.pop_front();
    soFar.push_back(toTry);
    if (soFar.size() == 1)
      cout << "\ttree 1: " << toTry << endl;
    auto takenCopy = taken;
    if (claimNumber(toTry, aboveLevel, aboveCutoff, takenCopy)) {
      if (bruteForcePerm(aboveLevel, aboveCutoff, moves, takenCopy, soFar, toPermute)){
        taken = takenCopy;
        return true;
      }
    }
    soFar.pop_back();
    toPermute.push_back(toTry);
  }
  return false;
}

void bruteForceLevel(vector<int> &currLevel, vector<int> &aboveLevel, int currCutoff, int aboveCutoff, vector<int> &moves, vector<bool> &taken) {
  deque<int> toPermute(currLevel.begin() + currCutoff, currLevel.end());
  vector<int> soFar;
  if (!bruteForcePerm(aboveLevel, aboveCutoff, moves, taken, soFar, toPermute)){
    cout << RED << "\tWARN: Level wasn't optimal" << RES << endl;
    exit(1);
  }
}

void bruteForce(vector<vector<int>> &levels, vector<int> &cutoffs, int boardSize, vector<int> &moves) {
  // Game starts at index 1
  vector<bool> taken(boardSize+1);
  for (size_t i = 1; i < levels.size(); ++i) {
    cout << "BRUTE FORCING LEVEL " << i << endl;
    bruteForceLevel(levels[i], levels[i-1], cutoffs[i], cutoffs[i-1], moves, taken);
  }
}

int main(int argc, char *argv[])
{
  double startTime = getWallTime();

  // Parse cmdline
  if (argc != 2)
    return 1;
  int boardSize = atoi(argv[1]);
  cout << "Board size = " << boardSize << endl;

  // Get the primes
  printStep("Step 1: Get primes");
  vector<int> primes;
  cout << "Sieve size = " << primesieve::get_sieve_size() << " kilobytes" << endl;
  double t1 = getWallTime();
  primesieve::generate_primes(boardSize, &primes);
  double timeElapsed = getWallTime() - t1;
  cout << "Time elapsed" << " : " << timeElapsed << " sec" << endl;

  // Generate Zamar's table
  printStep("Step 2: Get Zamar's table");
  vector<vector<int>> levels;
  generateFactors(levels, primes, 1, 0, boardSize, 0);
  for (auto &l : levels)
    sort(l.begin(), l.end());
  vector<int> cutoffs;
  generateCutoffs(levels, cutoffs, boardSize);
  cutoffs[0] = 1; // Can't take 1
  cutoffs[1] = levels[1].size() - 1; // Can only take the max prime

  // Print Zamar's table
  for (size_t i = 0; i < levels.size(); ++i) {
    cout << "Level\t" << i << " (sz\t" << levels[i].size() << "\tBF\t " << levels[i].size() - cutoffs[i] << "): \t";
    for (size_t j = 0; j < levels[i].size(); ++j) {
      //if (j == (size_t) cutoffs[i])
        //cout << "| \t";
      if (j < (size_t) cutoffs[i])
        cout << RED << levels[i][j] << RES << "\t";
      else
        cout << GREEN << levels[i][j] << RES << "\t";
    }
    cout << endl;
  }

  // Get our expected score
  printStep("Step 3: Get scores");
  long totalScore = (boardSize * (boardSize + 1)) / 2;
  long ourScore = 0;
  for (size_t i = 0; i < levels.size(); ++i)
    for (size_t j = cutoffs[i]; j < levels[i].size(); ++j)
      ourScore += levels[i][j];
  long computerScore = totalScore - ourScore;
  cout << "Ratio: " << (float) ourScore / totalScore << endl;
  cout << "Our score " << ourScore << endl;
  cout << "Computer score " << computerScore << endl;

  // Try to brute force the top ordering
  printStep("Step 4: Brute force levels");
  vector<int> moves;
  bruteForce(levels, cutoffs, boardSize, moves);
  cout << "Moves: " << endl;
  for (auto m : moves) {
    cout << m << " ";
  }
  cout << endl;

  // Print the time elapsed
  double totalTime = getWallTime() - startTime;
  cout << "Total time elapsed" << " : " << totalTime << " sec" << endl;
  return 0;
}
