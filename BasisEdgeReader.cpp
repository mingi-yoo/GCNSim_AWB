
#include "BasisEdgeReader.h"

using namespace std;

extern uint64_t CACHE_LINE_BYTE; 
extern uint64_t CACHE_LINE_COUNT;
extern uint64_t UNIT_BYTE;

extern uint64_t A_COL_START;  
extern uint64_t A_ROW_START;  
extern uint64_t X_START;  
extern uint64_t WEIGHT_START; 
extern uint64_t XW_START;
extern uint64_t AXW_START;

extern uint64_t w_h, w_w, x_h, x_w, a_w, a_h;

extern uint64_t w_fold;
extern uint64_t tot_axw_count;

extern queue<uint64_t> zero_row;

BasisEdgeReader::BasisEdgeReader(int id, 
					   			 uint64_t offset,
					   			 uint64_t a_col_size,
					   			 RowInfo row_info, 
					   			 DRAMInterface *dram) : q_space(0), prev_v(0), cur_v(0), pre_w_fold(0), remain_col_num(0) {
	this->id = id;
	this->offset = offset;
	this->row_info = row_info;
	this->dram = dram;
	req_address = A_COL_START + offset;
	flag.req_need = true;
	flag.q_empty = true;
	req_stat.tot_read_cnt = ceil((double)a_col_size/CACHE_LINE_COUNT);
	req_stat.pre_read_cnt = 0;
	req_stat.read_cnt_acm = 0;
	pre_row = row_info.row_start - 1;
	col_num_archive = 0;
	can_receive = true;
	pre_w_fold_start = 0;
}

BasisEdgeReader::~BasisEdgeReader() {}

ERData BasisEdgeReader::TransferData() {
	ERData ret;

	ret.rowindex = pre_row;
	if (pre_w_fold == 0) {
		ev.push_back(eq.front());
		eq.pop();
	}
	ret.colindex = ev[col_num_archive - remain_col_num];
	ret.pre_w_fold = pre_w_fold;
	ret.pre_w_fold_start = pre_w_fold_start;
	ret.pre_repeat = 0;
	ret.address = XW_START + (ret.colindex * w_fold + pre_w_fold) * CACHE_LINE_BYTE;
	
	if ((req_stat.pre_read_cnt < req_stat.tot_read_cnt) 
		&& (MAX_QUEUE_SIZE - q_space > CACHE_LINE_COUNT))
		flag.req_need = true;

	pre_w_fold++;

	if (remain_col_num == 1) {
		ret.is_end = true;
		if (pre_w_fold == w_fold) {
			q_space -= ev.size();
			ev.clear();
			pre_w_fold = 0;
			remain_col_num--;
			if (!(pre_row == row_info.row_end))
				can_receive = true;
			else
				flag.q_empty = true;
		}
	}
	else {
		if (pre_w_fold == w_fold) {
			pre_w_fold = 0;
			remain_col_num--;
		}	
		ret.is_end = false;
	}

	if (pre_w_fold == 0 && eq.empty())
		flag.q_empty = true;


	return ret;
}

bool BasisEdgeReader::IsEndOperation() {
	return (req_stat.pre_read_cnt == req_stat.tot_read_cnt);
}

bool BasisEdgeReader::IsEndColomn() {
	return (req_stat.read_cnt_acm == req_stat.tot_read_cnt) && flag.q_empty;
}

bool BasisEdgeReader::CanVertexReceive() {
	return can_receive;
}

void BasisEdgeReader::ReceiveData(queue<uint64_t> data) {
	req_stat.read_cnt_acm++;
	int bound = data.size();
	for (int i = 0; i < bound; i++) {
		eq.push(data.front());
		data.pop();
	}
	flag.q_empty = false;
}

void BasisEdgeReader::ReceiveData(uint64_t vertex) {
	prev_v = cur_v;
	cur_v = vertex;
	remain_col_num = cur_v - prev_v;
	col_num_archive = remain_col_num;
	pre_row++;
	can_receive = false;
	if (remain_col_num == 0) {
		for (int i = 0; i < w_fold; i++)
			tot_axw_count--;
		can_receive = true;
	}
}

void BasisEdgeReader::Request() {
	q_space += CACHE_LINE_COUNT;
	req_stat.pre_read_cnt++;
	if ((MAX_QUEUE_SIZE - q_space < CACHE_LINE_COUNT) ||
		(req_stat.pre_read_cnt == req_stat.tot_read_cnt))
		flag.req_need = false;
	dram->DRAMRequest(req_address, READ);
	req_address += CACHE_LINE_BYTE;
}

void BasisEdgeReader::TurnOffFlag() {
	flag.req_need = false;
}