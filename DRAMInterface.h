/*** temporary file ***
	Must Be Revised 
*/
#ifndef __DRAMINTERFACE_H__
#define __DRAMINTERFACE_H__

#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <src/dramsim3.h>
#include "Common.h"

using namespace std;

struct Transaction {
	uint64_t address;
	bool is_write;
};

struct GBInfo {
	int id;
	uint64_t address;
};

class DRAMInterface {
public:
	DRAMInterface(const string& config_file, const string& output_dir);
	~DRAMInterface();
	void UpdateCycle(); //update 1 cycle
	void DRAMRequest(uint64_t address, bool is_write); //request read/write to dram
	void DRAMRequest(uint64_t address, bool is_write, int id);
	void AddTransaction();
	bool FReadComplete(); //get imformation whether dram read is complete or not
	bool WReadComplete();
	bool CanRequest(int idx);
	bool IsExistTransaction();
	int ReturnID(uint64_t address);
	uint64_t GetReadData(bool isW); //get read address that read complete
	void PrintStats();
private:
	dramsim3::MemorySystem **dram;
	int dram_idx;
	int cmp_idx;
	bool f_read_complete;
	bool w_read_complete;
	queue<uint64_t> f_address;
	queue<uint64_t> w_address;
	queue<Transaction> t_queue;
	vector<GBInfo> info;
	void ReadCompleteCallback(uint64_t address); 
	void WriteCompleteCallback(uint64_t address); 
};

#endif