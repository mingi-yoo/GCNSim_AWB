/*** temporary file ***
	Must Be Revised 
*/

//MAX QUEUE SIZE is defined in Common.h
#include "VertexReader.h"

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
extern int UNIT_W_READ;
extern bool f_flag;

VertexReader::VertexReader(int id, 
						   uint64_t offset,
						   uint64_t a_row_size,
						   RowInfo row_info, 
						   DRAMInterface *dram) : q_space(0), pre_w_fold(0) {
	this->id = id;
	this->offset = offset;
	this->row_info = row_info;
	this->dram = dram;
	req_address = A_ROW_START + offset;
	flag.req_need = true;
	flag.q_empty = true;
	req_stat.tot_read_cnt = ceil((double)a_row_size/CACHE_LINE_COUNT);
	req_stat.pre_read_cnt = 0;
	req_stat.read_cnt_acm = 0;
	pre_row = row_info.row_start;
	tot_repeat = w_fold; 
	pre_repeat = 0;
	req_f_address = XW_START;
	pop_count = 0;
}

VertexReader::~VertexReader() {}

uint64_t VertexReader::TransferData() {
	//cout<<"VR) COL "<<pre_row<<" is passed"<<endl;
	pre_row++;
	if (pre_row == a_h)
		pre_row = 0;
	uint64_t ret = vq.front();

	vq.pop();
	q_space--;
	pop_count++;
	//cout<<pop_count<<endl;
	if (pop_count == CACHE_LINE_COUNT - 1 || (vq.size() == 1 && req_stat.read_cnt_acm == req_stat.tot_read_cnt))
		RequestF();

	if (pop_count == CACHE_LINE_COUNT)
		pop_count = 0;

	if (vq.empty())
		flag.q_empty = true;
	if ((req_stat.pre_read_cnt < req_stat.tot_read_cnt) 
		&& (MAX_QUEUE_SIZE - q_space > CACHE_LINE_COUNT))
		flag.req_need = true;

	return ret;
}

bool VertexReader::IsEndRequest() {
	return (req_stat.pre_read_cnt == req_stat.tot_read_cnt) && (pre_repeat < tot_repeat - 1);
}

bool VertexReader::IsEndOperation() {
	return (req_stat.pre_read_cnt == req_stat.tot_read_cnt) && (pre_repeat == tot_repeat - 1);
}

bool VertexReader::BasisEndOperation() {
	return (req_stat.pre_read_cnt == req_stat.tot_read_cnt);
}

void VertexReader::ReceiveData(queue<uint64_t> data) {
	//cout<<"VR) RECEIVE"<<endl;
	if (data.front() == 0)
		data.pop();
	int bound = data.size();
	req_stat.read_cnt_acm++;
	for (int i = 0; i < bound; i++) {
		vq.push(data.front());
		data.pop();
	}
	flag.q_empty = false;
} 

void VertexReader::Request() {
	q_space += CACHE_LINE_COUNT;
	req_stat.pre_read_cnt++;
	if (MAX_QUEUE_SIZE - q_space < CACHE_LINE_COUNT
		|| req_stat.pre_read_cnt == req_stat.tot_read_cnt)
		flag.req_need = false;
	dram->DRAMRequest(req_address, READ);
	//cout<<"VR) REQUEST. ADDRESS: "<<hex<<req_address<<endl;
	req_address += CACHE_LINE_BYTE;
}

void VertexReader::RequestF() {
	dram->DRAMRequest(req_f_address, READ);
	req_f_address += CACHE_LINE_BYTE;
	f_flag = false;
	//cout<<"Request Feature!"<<endl;
}

void VertexReader::ResetRequestStat() {
	req_stat.pre_read_cnt = 0;
	req_address = A_ROW_START + offset;
	pre_repeat++;
}

void VertexReader::TurnOffFlag() {
	flag.req_need = false;
}
