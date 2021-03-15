/*** temporary file ***
	Must Be Revised 
*/
#include "GlobalBuffer.h"

extern uint64_t CACHE_LINE_BYTE; 
extern uint64_t CACHE_LINE_COUNT;
extern uint64_t UNIT_BYTE;
extern uint64_t CACHE_WAY_N;

extern uint64_t A_COL_START;  
extern uint64_t A_ROW_START;  
extern uint64_t X_START;  
extern uint64_t WEIGHT_START; 
extern uint64_t XW_START;
extern uint64_t AXW_START;

extern int MAX_DRAM;
extern int MECHA_TYPE;
extern int UNIT_W_READ;

extern uint64_t w_h, w_w, x_h, x_w, a_w, a_h;

extern uint64_t cycle;
extern uint64_t w_fold;

extern uint64_t BF_OVERLAP;
extern vector<int> bfIdx;
extern vector<int> ovlBFIdx;
extern vector<bool> useOvlBF;

extern float BF_RATIO;
uint64_t BF_LEN;

int tot_req;

// 캐시 교체 알고리즘
extern uint64_t CACHE_REPL_ALGO;

vector<uint64_t> set_util;

GlobalBuffer::GlobalBuffer(int id,
						   uint64_t gbsize, 
						   uint64_t tot_w,
						   DRAMInterface *dram) : c_space(0) {
	this->id = id;
	this->gbsize = gbsize;
	this->tot_w = tot_w;
	this->dram = dram;
	isA = false;
	xwflag.w_fill_complete = false;
	xwflag.can_transfer = false;
	axwflag.can_transfer = false;
	axwflag.can_receive = true;
	axwflag.cache_full = false;
	axwflag.q_empty = true;
	w_fold_save = 0;
	pre_repeat = 0;
	
	BF_LEN = (uint64_t)a_w*BF_RATIO;

	// 캐시에서 이용할 단위 계산
	this-> blockN = gbsize / CACHE_LINE_BYTE;
	this-> setN = blockN / CACHE_WAY_N;
	this-> wayN = CACHE_WAY_N;

	// n-way set associative cache
	for(int i = 0; i < setN; i++) {
		this->cache.push_back(vector<int> (CACHE_WAY_N, -1));
		// 각 집합마다 way 수만큼 라인을 가지고 사용중이지 않기 때문에
		// -1로 채움
		this->weight_blk_idx.push_back(vector<int> (CACHE_WAY_N, -1));
		// blk idx (pre_w_fold) 저장하기 위한 배열
		set_util.push_back(0);
	}

	if(CACHE_REPL_ALGO == FIFO_ALGO) {
		FIFO newFifo = FIFO(wayN, setN);
		this->fifo =  &newFifo;
	} else if(CACHE_REPL_ALGO == LRU_ALGO) {
		LRU newLRU = LRU(wayN, setN);
		this->lru = &newLRU;
	} else if(CACHE_REPL_ALGO == BF_ALGO) {
		// 첫 사이클을 위해서 LRU 이용
		LRU newLRU = LRU(wayN, setN);
		this->lru = &newLRU;
		// 이후 두번째 사이클부터는 BF 기록된 정보를 이용한다...
		BloomFilter newBF = BloomFilter(wayN, setN);
		this->bf = &newBF;
		cout << "BF Length is... " << BF_LEN << endl;
	} else if(CACHE_REPL_ALGO == RRIP_ALGO) {
		RRIP newRRIP = RRIP(wayN, setN);
		this->rrip = &newRRIP;
	} else if(CACHE_REPL_ALGO == SHiP_ALGO) {
		SHiP newSHiP = SHiP(wayN, setN);
		this->ship = &newSHiP;
	} else if(CACHE_REPL_ALGO == MOVING_MED_ALGO) {
		// 첫 사이클을 위해서 LRU 이용
		LRU newLRU = LRU(wayN, setN);
		this->lru = &newLRU;
		MovingMedian newMVM = MovingMedian(wayN, setN);
		this->mvm = &newMVM;
	} else {
		cout << "[Warning] unknown algorithm selected!" << endl;
	}
}

GlobalBuffer::~GlobalBuffer() {
	// delete axwflag.requested;
}

// XW의 계산에서 Weight 미리 가져오는 과정에서 이용...
void GlobalBuffer::FillWeight() {
	tot_req = ceil((float)tot_w/CACHE_LINE_COUNT);

	uint64_t address = WEIGHT_START;
	int cnt = tot_req;

	for (int i = 0; i < cnt; i++) {
		dram->DRAMRequest(address, READ);
		dram->AddTransaction();
		dram->UpdateCycle();
		address += CACHE_LINE_BYTE;
		cycle++;
	}

	while (tot_req != 0) {
		dram->AddTransaction();
		dram->UpdateCycle();
		cycle++;
	}

	for (int i = 0; i < MAX_DRAM; i++) {
		if (dram->WReadComplete())
			dram->GetReadData(true);
	}

	//cout<<"FILL WEIGHT COMPLETE"<<endl;
	xwflag.w_fill_complete = true;
}

// X reading....
void GlobalBuffer::ReceiveData(XRData data) {
	//cout<<"GB) RECEIVE DATA: "<<dec<<data.ifvalue<<endl;
	if (xwflag.w_fill_complete) {
		xwflag.can_transfer = true;
	}
	x_data = data;
}

// 캐시 교체 알고리즘과 관련 있음....
void GlobalBuffer::ReceiveData(ERData data) {
	//cout<<"GB) RECEIVE DATA: "<<dec<<data.colindex<<endl;

	uint64_t from = data.rowindex;
	uint64_t to = data.colindex;
	int pre_w_fold = data.pre_w_fold;

	/************MSHR EXTENDED.... *************/
	wq.push(data);
	axwflag.q_empty = false;
	if (wq.size() == LIMIT_WQ_SIZE)
		axwflag.can_receive = false;

	// cout << "from: " << from << ", to: " << to << " , fold: " << pre_w_fold << endl;

	// BloomFilter first Cycle scan 관련 진행
	if(CACHE_REPL_ALGO == BF_ALGO && pre_w_fold == 0) {
		// cout << from << endl;
		bf->firstCycleAccess(from, to);
	}

	if(CACHE_REPL_ALGO == MOVING_MED_ALGO && pre_w_fold == 0) {
		// !!! put logic here
		mvm->firstCycleAccess(from, to);
	}



	bool hit = false;
	uint64_t hash = to;
	uint64_t setIdx = hash%setN;
	bool hasExist = false;

	// for cache util check
	set_util[setIdx]++;

	// 우선 이미 있는 정보인지 체크
	for(int i = 0; i < wayN; i++) {
		// 만약 캐시 해당 set에 이미 정보가 있다면
		if(cache[setIdx][i] == to && weight_blk_idx[setIdx][i] == pre_w_fold) {
			hasExist = true;
			hit = true; // 있었기 때문에 hit!
			// cout << "HIT! SET#: " << setIdx << endl;

			// LRU 알고리즘의 경우 사용 시 반영
			if(CACHE_REPL_ALGO == LRU_ALGO) {
				lru->access(setIdx, to, pre_w_fold);
			} else if((CACHE_REPL_ALGO == BF_ALGO || CACHE_REPL_ALGO == MOVING_MED_ALGO)&& pre_w_fold == 0) {
				// 첫 사이클은 LRU 이용
				lru->access(setIdx, to, pre_w_fold);
			} else if(CACHE_REPL_ALGO == BF_ALGO) {
				// 두번째 사이클부터...
				// 그런데 BF의 경우는 업데이트 필요 없다!
			} else if(CACHE_REPL_ALGO == RRIP_ALGO) {
				// hit이면 setIdx와 wayIdx 넘겨줘서 0으로 설정하도록!
				rrip->access(setIdx, i);
			} else if(CACHE_REPL_ALGO == SHiP_ALGO) {
				ship->access(setIdx, i, data.address);
			} else if(CACHE_REPL_ALGO == MOVING_MED_ALGO) {
				// Moving Median needs no update
			}

			break;
		}
	}
	// 만약 지금 캐시에 로드되어있지 않다면!!!
	if(!hit) {
		// cout << "MISS! SET#: " << setIdx << endl;
		//cout<<"NO HIT! "<<dec<<data.colindex<<", Cycle: "<<cycle<<endl;
		// axwflag.requested[to] = true;
		if (w_req_table.find(data.address) == w_req_table.end() || 
			w_req_table[data.address] == false)
			Request(data);
	} else { // 로드되어있다면!!!
		axwflag.can_transfer = true;
	}
}

// !!!!!!!! 캐시 교체 알고리즘 구성 시 변경 허용 
void GlobalBuffer::ReceiveData(uint64_t address) {
	//cout<<"GB) RECEIVE WEIGHT: "<<hex<<address<<endl;
	/*it maybe will be changed*/
	/* new code .....*/
	uint64_t col = w_map[address].colindex;
	uint64_t from = w_map[address].rowindex;
	uint64_t to = col;
	uint64_t hash = to;
	uint64_t setIdx = hash%setN;
	bool hasExist = false; // 빈 공간이나 이미 정보가 있었는지에 대한 flag
	int pre_w_fold = w_map[address].pre_w_fold;
	// 주의!!! 이미 캐시 위에는 없었기 때문에 들어오는 함수임!!!!!
	// 빈공간이 있다면 채우기
	// !!!! RRIP, SHiP는 hit이 아니면 무조건 replce를 이용하는 알고리즘
	if(CACHE_REPL_ALGO == RRIP_ALGO) {
		pair<int, int> target = rrip->replace(setIdx);
		int targetWayIdx = target.second;
		cache[setIdx][targetWayIdx] = to;
		weight_blk_idx[setIdx][targetWayIdx] = pre_w_fold;
		
	} else if(CACHE_REPL_ALGO == SHiP_ALGO) {
		pair<int, int> target = ship->replace(setIdx, address);
		int targetWayIdx = target.second;
		cache[setIdx][targetWayIdx] = to;
		weight_blk_idx[setIdx][targetWayIdx] = pre_w_fold;
	} else { // !!!! 다른 알고리즘은 scan 후 진행
		for(int i = 0; i < wayN; i++) {
			// 만약 캐시 해당 set에 빈 공간이 있다면
			if(cache[setIdx][i] == -1) {
				cache[setIdx][i] = to;
				weight_blk_idx[setIdx][i] = pre_w_fold;
				// 교체 알고리즘 클래스 관리
				if(CACHE_REPL_ALGO == FIFO_ALGO) {
					fifo->access(setIdx, to, pre_w_fold);
				} else if(CACHE_REPL_ALGO == LRU_ALGO) {
					lru->access(setIdx, to, pre_w_fold);
				} else if((CACHE_REPL_ALGO == BF_ALGO || CACHE_REPL_ALGO == MOVING_MED_ALGO) && pre_w_fold == 0) {
					// 첫번째 사이클은 LRU로 진행
					lru->access(setIdx, to, pre_w_fold);
				} else if(CACHE_REPL_ALGO == BF_ALGO) {
					// 두번째 사이클부터는 BF로 진행
					// 그런데 BF는 수정할 필요가 없음
				} else if(CACHE_REPL_ALGO == MOVING_MED_ALGO) {
					// MVM after 2nd cycle
				}

				hasExist = true;
				break;
			}
		}

		// 만약 공간이 없었다면 replacement algo 적용해서 비우자
		if(!hasExist) {
			if(CACHE_REPL_ALGO == FIFO_ALGO) {
				// 교체할 대상 받아옴
				pair<int, int> replacePair = fifo->replace(setIdx);
				int replaceVal = replacePair.first;
				int replaceBlkIdx = replacePair.second;
				// flag에서 이제 없앨 예정이라고 표시
				// axwflag.requested[replaceVal] = false;
				for(int i = 0; i < wayN; i++) {
					if(cache[setIdx][i] == replaceVal && weight_blk_idx[setIdx][i] == replaceBlkIdx) {
						cache[setIdx][i] = to;
						weight_blk_idx[setIdx][i] = pre_w_fold;
						fifo->access(setIdx, to, pre_w_fold);
						break;
					}
				}

			} else if(CACHE_REPL_ALGO == LRU_ALGO) {
				pair<int, int> replacePair = lru->replace(setIdx);
				int replaceVal = replacePair.first;
				int replaceBlkIdx = replacePair.second;
				// flag에서 이제 없앨 예정이라고 표시
				// axwflag.requested[replaceVal] = false;
				for(int i = 0; i < wayN; i++) {
					if(cache[setIdx][i] == replaceVal && weight_blk_idx[setIdx][i] == replaceBlkIdx) {
						cache[setIdx][i] = to;
						weight_blk_idx[setIdx][i] = pre_w_fold;
						lru->access(setIdx, to, pre_w_fold);
						break;
					}
				}
			} else if(CACHE_REPL_ALGO == BF_ALGO && pre_w_fold == 0) {
				pair<int, int> replacePair = lru->replace(setIdx);
				int replaceVal = replacePair.first;
				int replaceBlkIdx = replacePair.second;
				// flag에서 이제 없앨 예정이라고 표시
				// axwflag.requested[replaceVal] = false;
				for(int i = 0; i < wayN; i++) {
					if(cache[setIdx][i] == replaceVal && weight_blk_idx[setIdx][i] == replaceBlkIdx) {
						cache[setIdx][i] = to;
						weight_blk_idx[setIdx][i] = pre_w_fold;
						lru->access(setIdx, to, pre_w_fold);
						break;
					}
				}
			} else if(CACHE_REPL_ALGO == BF_ALGO) {
				// 두번째 사이클부터는 BF 적용
				// overlap을 사용해야하는지 파악
				bool useOvl = useOvlBF[from];
				int whichBF = bfIdx[(int)from];
				int whichOlBF = ovlBFIdx[(int)from];
				pair<int, int> replacePair;
				if(BF_OVERLAP == 0) {
					replacePair = bf->replace(setIdx, whichBF, false, &cache[setIdx], &weight_blk_idx[setIdx]);
				} else {
					if(!useOvl) { // 일반 BF이용 시
						replacePair = bf->replace(setIdx, whichBF, useOvl, &cache[setIdx], &weight_blk_idx[setIdx]);
					} else { // overlap BF 이용시
						replacePair = bf->replace(setIdx, whichOlBF, useOvl, &cache[setIdx], &weight_blk_idx[setIdx]);
					}
				}

				int replaceVal = replacePair.first;
				int replaceBlkIdx = replacePair.second;
				for(int i = 0; i < wayN; i++) {
					if(cache[setIdx][i] == replaceVal && weight_blk_idx[setIdx][i] == replaceBlkIdx) {
						cache[setIdx][i] = to;
						weight_blk_idx[setIdx][i] = pre_w_fold;
						break;
					}
				}
				
			} else if(CACHE_REPL_ALGO == MOVING_MED_ALGO && pre_w_fold == 0) {
				pair<int, int> replacePair = lru->replace(setIdx);
				int replaceVal = replacePair.first;
				int replaceBlkIdx = replacePair.second;
				// flag에서 이제 없앨 예정이라고 표시
				// axwflag.requested[replaceVal] = false;
				for(int i = 0; i < wayN; i++) {
					if(cache[setIdx][i] == replaceVal && weight_blk_idx[setIdx][i] == replaceBlkIdx) {
						cache[setIdx][i] = to;
						weight_blk_idx[setIdx][i] = pre_w_fold;
						lru->access(setIdx, to, pre_w_fold);
						break;
					}
				}
			} else if(CACHE_REPL_ALGO == MOVING_MED_ALGO) {
				// 두번째 사이클부터는 Moving Median 적용
				pair<int, int> replacePair = mvm->replace(from, setIdx, &cache[setIdx], &weight_blk_idx[setIdx]);
				// !!! put logic here
				int replaceVal = replacePair.first;
				int replaceBlkIdx = replacePair.second;
				for(int i = 0; i < wayN; i++) {
					if(cache[setIdx][i] == replaceVal && weight_blk_idx[setIdx][i] == replaceBlkIdx) {
						cache[setIdx][i] = to;
						weight_blk_idx[setIdx][i] = pre_w_fold;
						break;
					}
				}
				
			} else {
				cout << "[Warning] unknown algorithm selected!" << endl;
			}
		}
	}

	w_req_table[address] = false;
	if (address == (wq.front()).address)
		axwflag.can_transfer = true; 
}

void GlobalBuffer::Request(ERData data) {
	uint64_t address = data.address;
	// 기존 addressing scheme
	w_map[address] = data;
	w_req_table[address] = true;
	dram->DRAMRequest(address, READ, id); 
	//cout<<"GB) WEIGHT REQUEST: "<<hex<<address<<endl;
}

void GlobalBuffer::CanTransfer() {
	if (axwflag.q_empty)
		axwflag.can_transfer = false;
	else {
		ERData data = wq.front();

		uint64_t from = data.rowindex;
		uint64_t to = data.colindex;
		int pre_w_fold = data.pre_w_fold;
		uint64_t hash = to;
		uint64_t setIdx = hash%setN;

		bool can_transfer = false;

		for (int i = 0; i < wayN; i++) {
			if(cache[setIdx][i] == to && weight_blk_idx[setIdx][i] == pre_w_fold)
				can_transfer = true;
		}
		if (!can_transfer) {
			if (!w_req_table[data.address])
				Request(data);
		}
		axwflag.can_transfer = can_transfer;
	}
}

XRData GlobalBuffer::TransferXData() {
	//cout<<"GB) WEIGHT READ COMPLETE"<<endl;
	xwflag.can_transfer = false;
	XRData ret = x_data;
	return ret;
}

ERData GlobalBuffer::TransferAData() {
	//cout<<"GB) WEIGHT READ COMPLETE"<<endl;
	axwflag.can_receive = true;
	ERData ret = wq.front();
	wq.pop();
	if (wq.size() == 0)
		axwflag.q_empty = true;
	return ret;
}