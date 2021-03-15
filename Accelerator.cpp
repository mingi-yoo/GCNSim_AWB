/*** temporary file ***
	Must Be Revised 
*/
#include "Accelerator.h"

#define MAC_CYCLE 1

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

extern queue<uint64_t> zero_row;

Accelerator::Accelerator(int id,
						 uint64_t axw_offset, 
						 uint64_t pe, 
						 RowInfo row_info,
						 DRAMInterface *dram) : isA(false) {
	this->id = id;
	this->axw_offset = axw_offset;
	this->pe = pe;
	this->dram = dram;
	this->row_info = row_info;
	axw_address = AXW_START + axw_offset;
	row = row_info.row_start;
	limit = ceil((float)pe/CACHE_LINE_COUNT);
}

Accelerator::~Accelerator() {}

bool Accelerator::CanReceive() {
	if (!isA)
		return (m1_data.size() < limit);
	else
		return (m2_data.size() < limit);
}

void Accelerator::ReceiveData(XRData data) {
	//cout<<"ACC) RECEIVE DATA"<<endl;
	if (CanReceive())
		m1_data.push_back({data, 0});
	else
		x_data.push(data);
	
	
}

void Accelerator::ReceiveData(ERData data) {
	//cout<<"ACC) RECEIVE DATA"<<endl;
	if (CanReceive())
		m2_data.push_back({data, 0});
	else
		a_data.push(data);
	
}

void Accelerator::WriteZeroRow() {
	//cout<<"ROW: "<<zero_row.front()<<"is zero row."<<endl;
	zero_row.pop();
	dram->DRAMRequest(axw_address, WRITE);
	axw_address += CACHE_LINE_BYTE;
}

void Accelerator::MAC() {
	//cout<<"ACC) MAC RUNNING"<<endl;
	int offset = 0;
	int cnt = m2_data.size();
	for (int i = 0; i < cnt; i++) {
		m2_data[offset].pre_cycle++;
		if (m2_data[offset].pre_cycle == MAC_CYCLE) {
			if (m2_data[offset].data.is_end) {
				//cout<<"ROW: "<<dec<<m2_data[offset].data.rowindex<<" complete."<<endl;
				dram->DRAMRequest(axw_address, WRITE);
				axw_address += CACHE_LINE_BYTE;
			}
			m2_data.erase(m2_data.begin() + offset);
			if (!(a_data.empty())) {
				m2_data.push_back({a_data.front(), 0});
				a_data.pop();
			}
		}
		else
			offset++;
	}
}