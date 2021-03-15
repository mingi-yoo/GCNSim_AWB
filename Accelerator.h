/*** temporary file ***
	Must Be Revised 
*/
#ifndef __ACCELERATOR_H__
#define __ACCELERATOR_H__

#include <iostream>
#include <queue>
#include <vector>
#include <cmath>
#include "DRAMInterface.h"
#include "Common.h"

using namespace std;

struct MAC1Data {
	XRData data;
	int pre_cycle;
};

struct MAC2Data {
	ERData data;
	int pre_cycle;
};

class Accelerator {
public:
	bool isA;
	Accelerator(int id,
				uint64_t axw_offset, 
				uint64_t pe,
				RowInfo row_info, 
				DRAMInterface *dram);
	~Accelerator();
	bool CanReceive();
	void ReceiveData(XRData data);
	void ReceiveData(ERData data);
	void WriteZeroRow();
	void MAC();
private:
	int id; // for multiprocessing
	int limit;
	/*
	 * offset: DRAM에서 계산 쉽도록 offset 계산
	 * 참조: start.ini
	*/
	//
	uint64_t axw_address;
	uint64_t axw_offset; // output

	uint64_t pe; // # of process element
				// 한번에 계산할 수 있는 양
	uint64_t row;
	RowInfo row_info;
	DRAMInterface *dram;
	queue<XRData> x_data;
	queue<ERData> a_data;
	vector<MAC1Data> m1_data;
	vector<MAC2Data> m2_data;
};

#endif