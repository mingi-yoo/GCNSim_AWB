#ifndef __CACHEREPLACEMENT_H__
#define __CACHEREPLACEMENT_H__

#include <fstream>
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include "Common.h"

// Replacement 알고리즘
#define FIFO_ALGO 0
#define LRU_ALGO 1
#define RRIP_ALGO 2
#define BF_ALGO 3
#define SHiP_ALGO 4
#define OPTIMAL_ALGO 5
#define MOVING_MED_ALGO 6

using namespace std;

class FIFO {
    private:
    int wayN;
    int setN;

    public:
    FIFO(int wayN, int setN);
    void access(int setIdx, int data, int weightBlkIdx);
    pair<int, int> replace(int setIdx);
	void clear();
};

// LRU replacemnt 보조 클래스
class LRU {
    private:
    int wayN;
    int setN;
    public:
    LRU(int wayN, int setN);
    void access(int setIdx, int data, int weightBlkIdx);
    pair<int, int> replace(int setIdx);
	void clear();
};

class RRIP {
    private:
    int wayN;
    int setN;
    public:
    RRIP(int wayN, int setN);
    void access(int setIdx, int wayIdx);
    pair<int, int> replace(int setIdx);
};

class SHiP {
    private:
    int wayN;
    int setN;
    // vector<bool> lineOutcome;
    // vector<uint64_t> lineSignature;
    public:
    SHiP(int wayN, int setN); 
    void access(int setIdx, int wayIdx, uint64_t address);
    pair<int, int> replace(int setIdx, uint64_t address);
    int getSHCTSize(); // 마지막에 테이블 사이즈 출력
};

// BF replacement 보조 클래스
class BloomFilter {
    private:
    int wayN;
    int setN;
	// 각 블록 별로 몇 인덱스씩 가져야하는지
	vector<int> blkIdxVector;
	// Overlap 블록에 대해서도 계산
	vector<int> ovlBlkIdxVector;
	// Overlap Block StartIdx
	int ovlStartIdx;
	int curRowIdx;
    public:
    BloomFilter(int wayN, int setN);
	int firstCycleAccess(uint64_t from, uint64_t to);
    pair<int, int> replace(int setIdx, int blkIdx, bool useOvl, vector<int>* setVector, vector<int>* weightVector);
};

// Moving Median Algorithm
class MovingMedian {
    private:
    int wayN;
    int setN;
    public:
    MovingMedian(int wayN, int setN);
    int firstCycleAccess(uint64_t from, uint64_t to);
    pair<int, int> replace(int from, int setIdx, vector<int>* setVector, vector<int>* weightVector);
};
#endif