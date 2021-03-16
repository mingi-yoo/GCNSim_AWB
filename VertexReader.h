/*** temporary file ***
	Must Be Revised 
*/
#ifndef __VERTEXREADER_H__
#define __VERTEXREADER_H__

#include <iostream>
#include <queue>
#include <cmath>
#include "DRAMInterface.h"
#include "Common.h"

using namespace std;

class VertexReader {
public:
	int pre_w_fold;
	ReaderFlag flag;
	VertexReader(int id, 
				 uint64_t offset, 
				 uint64_t a_row_size,
				 RowInfo row_info,
				 DRAMInterface *dram);
	~VertexReader();
	uint64_t TransferData();
	bool IsEndRequest(); // all of request done (1 cycle)
	bool IsEndOperation(); // all of request done for AXW 
	bool BasisEndOperation();
	void ReceiveData(queue<uint64_t> data);
	void Request();
	void RequestF();
	void ResetRequestStat();
	void TurnOffFlag();
private:
	int x_repeat;
	uint64_t pre_row_archive;
	uint64_t v_cache_idx;
	int id;
	int pop_count;
	uint64_t q_space;
	uint64_t offset;
	uint64_t req_address;
	uint64_t req_f_address;
	int pre_row; // for row index check
	int tot_repeat;
	int pre_repeat;
	RCNT req_stat;
	RowInfo row_info;
	DRAMInterface *dram;
	queue<uint64_t> vq; //save vertex data from DRAM
};

#endif