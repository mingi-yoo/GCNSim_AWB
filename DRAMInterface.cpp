/*
 * DRAMsim3와 인터페이스
*/
#include "DRAMInterface.h"

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

extern int MAX_DRAM;

extern uint64_t w_h, w_w, x_h, x_w, a_w, a_h;

// defined in Distributer.cpp
extern uint64_t tot_xw_count;
extern uint64_t tot_axw_count;

extern int tot_req;

// defined in RunSimulator.cpp
extern uint64_t cycle;

uint64_t a_util;
uint64_t w_util;

DRAMInterface::DRAMInterface(const string& config_file, const string& output_dir) {
	dram = new dramsim3::MemorySystem* [MAX_DRAM];
	dram_idx = 0;
	cmp_idx = 0;
	for (int i = 0; i < MAX_DRAM; i++) {
		dram[i] = dramsim3::GetMemorySystem(config_file, 
									 		output_dir, 
									 		std::bind(&DRAMInterface::ReadCompleteCallback, this, std::placeholders::_1), 
									 		std::bind(&DRAMInterface::WriteCompleteCallback, this, std::placeholders::_1));
		
	}
	f_read_complete = false;
	w_read_complete = false;
}

DRAMInterface::~DRAMInterface() {
	for (int i = 0; i < MAX_DRAM; i++)
		delete dram[i];
	delete dram;
}

void DRAMInterface::UpdateCycle() {
	for (int i = 0; i < MAX_DRAM; i++)
		dram[i]->ClockTick();
}

void DRAMInterface::DRAMRequest(uint64_t address, bool is_write) {
	Transaction t = {address, is_write};
	t_queue.push(t);
}

void DRAMInterface::DRAMRequest(uint64_t address, bool is_write, int id) {
	Transaction t = {address, is_write};
	t_queue.push(t);
	info.push_back({id, address});
}

void DRAMInterface::AddTransaction () {
	int check = dram_idx;
	while (IsExistTransaction()) {
		if (CanRequest(dram_idx)) {
			Transaction t = t_queue.front();
			dram[dram_idx]->AddTransaction(t.address, t.is_write);
			t_queue.pop();
			dram_idx++;
			if (dram_idx == MAX_DRAM)
				dram_idx = 0;
		}
		else {
			dram_idx++;
			if (dram_idx == MAX_DRAM)
				dram_idx = 0;
			if (dram_idx == check)
				break;
		}
	}
}

bool DRAMInterface::FReadComplete() {
	return f_read_complete;
}

bool DRAMInterface::WReadComplete() {
	return w_read_complete;
}

bool DRAMInterface::CanRequest(int idx) {
	Transaction t = t_queue.front();
	if(dram[idx]->WillAcceptTransaction(t.address, t.is_write))
		return true;
	else
		return false;
	
}

bool DRAMInterface::IsExistTransaction() {
	return !(t_queue.empty());
}

int DRAMInterface::ReturnID(uint64_t address) {
	int i, ret;
	for (i = 0; i < info.size(); i++) {
		if (info[i].address == address)
			break;
	}
	ret = info[i].id;
	info.erase(info.begin() + i);
	return ret;
}

uint64_t DRAMInterface::GetReadData(bool isW) {
	uint64_t ret;
	if (!isW) {
		ret = f_address.front();
		f_address.pop();
		if (f_address.empty())
			f_read_complete = false;
	}
	else {
		ret = w_address.front();
		w_address.pop();
		if (w_address.empty())
			w_read_complete = false;
	}
	return ret;
}

void DRAMInterface::PrintStats() {
	for(int i = 0; i < MAX_DRAM; i++) {
		dram[i] -> PrintStats();
	}
}

// 방법을 바꿀 필요 있어보임.....
void DRAMInterface::ReadCompleteCallback(uint64_t address) {
	if (address < WEIGHT_START) {
		f_read_complete = true;
		f_address.push(address);
		a_util++;
	}
	else {
		w_read_complete = true;
		w_address.push(address);
		w_util++;
	}
	tot_req--;
	//cout<<"Read Complete. Address: "<<hex<<address;
	//cout<<", Cycle: "<<dec<<cycle<<endl;
}

void DRAMInterface::WriteCompleteCallback(uint64_t address) {
	if (address >= AXW_START)
		tot_axw_count--;
	else
		tot_xw_count--;
	//cout<<"Write Complete. Address: "<<hex<<address;
	//cout<<", Cycle: "<<dec<<cycle<<endl;
}
