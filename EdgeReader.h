#ifndef __EDGEREADER_H__
#define __EDGEREADER_H__

#include <iostream>
#include <queue>
#include <cmath>
#include "DRAMInterface.h"
#include "Common.h"

using namespace std;

class EdgeReader {
public:
	int pre_w_fold;
	bool can_receive;
	ReaderFlag flag;
	EdgeReader(int id, 
			   uint64_t offset,
			   uint64_t a_col_size,
			   RowInfo row_info, 
			   DRAMInterface *dram);
	~EdgeReader();
	ERData TransferData();
	bool IsEndRequest();
	bool IsEndOperation();
	bool CanVertexReceive();
	void ReceiveData(queue<uint64_t> data);
	void ReceiveData(uint64_t vertex);
	void Request();
	void TurnOffFlag();
	// for our mechanism
	void ResetRequestStat();
	void OuterProduct();
	void Write();

private:
	int x_repeat;
	uint64_t e_cache_idx;
	uint64_t pre_row_archive;
	int write_count;
	bool do_write;
	int id;
	int transfer_cnt;
	int pre_w_fold_start; // save start w_fold
	uint64_t a_col_size;
	uint64_t q_space;
	uint64_t offset;
	uint64_t req_address;
	uint64_t write_address;
	int pre_row;
	int tot_repeat, tot_req_repeat;
	int pre_repeat, pre_req_repeat;
	// for index check
	uint64_t cur_v; // remain number of column check
	uint64_t prev_v;
	// check remain num of column
	uint64_t remain_col_num;
	uint64_t col_num_archive;
	// other
	RCNT req_stat;
	RowInfo row_info;
	DRAMInterface *dram;
	queue<uint64_t> eq; //save edge data from DRAM
	vector<uint64_t> ev;
};

#endif