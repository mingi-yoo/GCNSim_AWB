/*** temporary file ***
	Must Be Revised 
*/
#ifndef __GLOBALBUFFER_H__
#define __GLOBALBUFFER_H__

#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include "DRAMInterface.h"
#include "Common.h"
#include "CacheReplacement.h"

#define LIMIT_WQ_SIZE 32

using namespace std;

struct XWFlag {
	bool w_fill_complete;
	bool can_transfer;
};

struct AXWFlag {
	bool can_transfer;
	bool can_receive;
	bool cache_full;
	bool q_empty;
	// bool *requested; // it maybe changed
};

class GlobalBuffer {
public:
	int w_fold_save;
	int pre_repeat;
	XWFlag xwflag;
	AXWFlag axwflag;
	GlobalBuffer(int id,
				 uint64_t gbsize, 
				 uint64_t tot_w,
				 DRAMInterface *dram);
	~GlobalBuffer();
	void FillWeight();
	void ReceiveData(XRData data);
	void ReceiveData(ERData data);
	void ReceiveData(uint64_t address); // you can change
	void CacheReplacement(); // you can change
	void CanTransfer();
	XRData TransferXData();
	ERData TransferAData();
private:
	int id;
	int c_space;
	uint64_t gbsize;
	uint64_t tot_w; //total size of weight (only XW allowed)
	bool isA;
	XRData x_data;
	ERData a_data;
	DRAMInterface *dram;
	vector<uint64_t> w_data;
	int blockN;
	int setN;
	int wayN;
	vector<vector<int>> cache;
	vector<vector<int>> weight_blk_idx;
	// replacement algo classes
    FIFO* fifo;
    LRU* lru;
    BloomFilter *bf;
	RRIP* rrip;
	SHiP* ship;
	MovingMedian* mvm;
	int before_pre_w_fold;
	void Request(ERData data);
	/*****************************/
	// for mshr
	queue<ERData> wq;
	map<uint64_t, ERData> w_map;
	map<uint64_t, bool> w_req_table;
};
#endif