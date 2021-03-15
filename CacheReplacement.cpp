#include "CacheReplacement.h"

// Bloom Filter Num
extern uint64_t BLOOM_FILTER_NUM;
extern uint64_t BF_LEN; // Globalbuffer에서 a_w로 계산
extern uint64_t BF_OVERLAP;

extern uint64_t w_h, w_w, x_h, x_w, a_w, a_h;

extern uint64_t MOV_MED_BLOCK_NUM;

// todo !!!!! multi unit 관련해서 상의해볼 필요 있음

vector<vector<int>> fifoList; // fifo를 위한 tmp
vector<vector<int>> fifoWeightBlkIdx;

vector<vector<int>> lruList;  // lru를 위한 tmp
vector<vector<int>> lruWeightBlkIdx; // lru에서 weight blk idx 저장

vector<vector<int>> bfList; // BF를 위한 tmp
vector<vector<int>> olBfList; // overlap BF를 위한 tmp
vector<int> bfIdx;
vector<int> ovlBFIdx;
vector<bool> useOvlBF;

// RRIP에서 2bit RRIP 구현...
// SHiP에서도 이용됨....
vector<vector<int>> rrpvList;

// SHiP
vector<bool> lineOutcome;
vector<int> lineSignature;
map<int, int> SHCT;

vector<int> mvmTmp;
vector<double> medians;
vector<int> mvmBlkIdxVector;
int mvmCurIdx;

/*
 * >>>>>> FIFO Cache Replacement <<<<<
*/

FIFO::FIFO(int wayN, int setN) {
	this->wayN = wayN;
	this->setN = setN;
	for(int i = 0; i < setN; i++) {
		fifoList.push_back(vector<int> ());
		fifoWeightBlkIdx.push_back(vector<int> ());
	}
}

// 이미 cache 내의 꽉찬지 여부는 cache에서 관리중이므로
// 여기서는 fifo만 단순하게 관리해주면 됨...

// hit이 아닌 miss의 경우에만 들어오게 됨
// 따라서 그냥 넣어주기만 하면 됨
// 넘치는 경우에 대해서는 replace에서 제거함
void FIFO::access(int setIdx, int data, int weightBlkIdx) {
	fifoList[setIdx].push_back(data);
	fifoWeightBlkIdx[setIdx].push_back(weightBlkIdx);
}

// 해당 set에서 없애야할(교체해야할) edgeTo를 알려줌
pair<int, int> FIFO::replace(int setIdx) {
	vector<int>::iterator iter;
	vector<int>::iterator weightIter;
	iter = fifoList[setIdx].begin();
	weightIter = fifoWeightBlkIdx[setIdx].begin(); 
	// 첫번째 (먼저 들어온) 요소 취하고 삭제
	int firstIn = fifoList[setIdx][0];
	int firstInBlkIdx = fifoWeightBlkIdx[setIdx][0];
	fifoList[setIdx].erase(iter);
	fifoWeightBlkIdx[setIdx].erase(weightIter);
	return make_pair(firstIn, firstInBlkIdx);
}

// fold 변경 시 clear
void FIFO::clear() {
	fifoList.clear();
	for(int i = 0; i < setN; i++) {
		fifoList.push_back(vector<int> ());
	}
}

/* >>>>> (end) <<<<< */


/*
 * >>>>>> LRU Cache Replacement <<<<<
*/

LRU::LRU(int wayN, int setN) {
	this->wayN = wayN;
	this->setN = setN;
	for(int i = 0; i < setN; i++) {
		lruList.push_back(vector<int> ());
		lruWeightBlkIdx.push_back(vector<int> ());
	}
}

void LRU::access(int setIdx, int data, int weightBlkIdx) {
	bool isExist = false;
	for(int i = 0; i < lruList[setIdx].size(); i++) {
		// 이미 존재하는 경우
		// 우선 삭제!
		if(lruList[setIdx][i] == data && lruWeightBlkIdx[setIdx][i] == weightBlkIdx) {
			vector<int>::iterator iter = lruList[setIdx].begin();
			vector<int>::iterator iterW = lruWeightBlkIdx[setIdx].begin();
			iter += i;
			iterW += i;
			// !!!!!! 이 밑이 LRU 오류 지점!!!!
			lruList[setIdx].erase(iter);
			lruWeightBlkIdx[setIdx].erase(iterW);
			isExist = true;
		}
	}
	// 사용한 값은 무조건 앞으로 추가
	lruList[setIdx].insert(lruList[setIdx].begin(), data);
	lruWeightBlkIdx[setIdx].insert(lruWeightBlkIdx[setIdx].begin(), weightBlkIdx);
}

pair<int, int> LRU::replace(int setIdx) {
	// 무조건 miss 인 경우에만 호출되기 때문에
	// 루틴에만 집중하면 됨

	// 가장 덜 사용한 요소를 반환함...
	// lruList[setIdx]의 마지막 요소
	int target = lruList[setIdx][lruList[setIdx].size()-1];
	int targetWeightBlkIdx = lruWeightBlkIdx[setIdx][lruWeightBlkIdx[setIdx].size()-1];
	lruList[setIdx].pop_back();
	lruWeightBlkIdx[setIdx].pop_back();
	return make_pair(target, targetWeightBlkIdx);
}

void LRU::clear() {
	lruList.clear();
	for(int i = 0; i < setN; i++) {
		lruList.push_back(vector<int> ());
	}
}

/* >>>>> (end) <<<<< */

/*
 * >>>>>> RRIP Cache Replacement <<<<<
*/

RRIP::RRIP(int wayN, int setN) {
	this->wayN = wayN;
	this->setN = setN;

	/* RRIP 기본 logic 설명
	 * 0-1. 처음에 모든 RRPV는 3으로 초기화된다.
	 * 0-2. hit해서 access 되는 경우 RRPV는 0이 된다. 
	 * 1. set에서 첫 3을 찾는다.
	 * 2. 3이 찾아지면 '5'로 이동
	 * 3. (찾아지지 않으면) 모든 RRPV++
	 *    (0->1, 1->2, 2->3)
	 * 4. '1'로 이동
	 * 5. 블록 교체대상으로 반환하고, 해당 블럭 RRPV는 2로 설정
	 */

	// 0. 처음에 모든 RRPV는 3으로 초기화된다.
	for(int i = 0; i < setN; i++) {
		rrpvList.push_back(vector<int> (wayN, 3));
	}
}

// Hit의 경우임!!!
void RRIP::access(int setIdx, int wayIdx) {
	// 0-2. hit해서 access 되는 경우 RRPV는 0이 된다.
	rrpvList[setIdx][wayIdx] = 0;
}

// Miss의 경우임!!!
// setIdx와 wayIdx 반환
pair<int, int> RRIP::replace(int setIdx) {
	bool isFound = false;
	int wayIdx;
	/*
	 * 1. set에서 첫 3을 찾는다.
	 * 2. 3이 찾아지면 '5'로 이동
	 * 3. (찾아지지 않으면) 모든 RRPV++
	 *    (0->1, 1->2, 2->3)
	 * 4. '1'로 이동
	 * 5. 블록 교체대상으로 반환하고, 해당 블럭 RRPV는 2로 설정
	 */

	// 교체할 것을 찾을때까지 돌아라
	while(!isFound) {
		// 1. set에서 첫 3을 찾는다.
		for(int i = 0; i < rrpvList[setIdx].size(); i++) {
			if(rrpvList[setIdx][i] == 3) {
				// 2. 3이 찾아지면 '5'로 이동
				wayIdx = i;
				isFound = true;
				break;
			}
		}
		if(!isFound) {
			// 3. (찾아지지 않으면) 모든 RRPV++
			for(int i = 0; i < rrpvList[setIdx].size(); i++) {
				int val = rrpvList[setIdx][i];
				// (0->1, 1->2, 2->3)
				// 그냥 ++ 해도 되지만... 혹시 모를 오류를 위해서
				// if 문으로 구성함...
				if(val == 0 || val == 1 || val == 2) {
					rrpvList[setIdx][i]++; 
				} else {
					cout << "RRPV error... 3 cannot be ++ but it happened... check logic" << endl;
				}
			}
		}
		// 4. 1로 이동 (isFound == false)
	}
	// 5. 블록 교체대상으로 반환하고, 해당 블럭 RRPV는 2로 설정
	rrpvList[setIdx][wayIdx] = 2;
	return make_pair(setIdx, wayIdx);
}

/* >>>>> (end) <<<<< */

/*
 * >>>>>> SHiP Cache Replacement <<<<<
*/

SHiP::SHiP(int wayN, int setN) {
	this->wayN = wayN;
	this->setN = setN;

	/* SHiP 기본 logic 설명
	 * 우선 기본적인 replacement는 RRIP를 이용한다...
	 * 
	 * RRIP의 기본적인 logic은 아래와 같다...
	 * 0-1. 처음에 모든 RRPV는 3으로 초기화된다.
	 * 0-2. hit해서 access 되는 경우 RRPV는 0이 된다. 
	 * 1. set에서 첫 3을 찾는다.
	 * 2. 3이 찾아지면 '5'로 이동
	 * 3. (찾아지지 않으면) 모든 RRPV++
	 *    (0->1, 1->2, 2->3)
	 * 4. '1'로 이동
	 * 5. 블록 교체대상으로 반환하고, 해당 블럭 RRPV는 2또는 3으로 설정
	 * 
	 * SHiP에서 추가로 이용하는 것은 signature를 이용한
	 * SHCT(Signature History Counter Table)이다...
	 * 
	 * 또한 cache line별로 두개의 요소를 추가로 이용한다...
	 * (A: line_outcome과 B: line_signature)
	 * line_outcome은 initially set to zero(false)
	 * 
	 * 이 때 각 signature를 계산하는 것이 중요한데...
	 * SHiP 논문에서는 MSB 14bit을 활용해서 memory region을 393(약 400개)
	 * 정도로 분할되었다고 하였다...
	 * 이 구현에서는 area를 나눈다고 단순히 생각하여
	 * right shift를 해서 적당하게 나오도록 하였는데...
	 * !!!!! todo - logic에 문제가 있으면 수정해야함 !!!!!
	 * 
	 * RRIP를 활용한 pseudo code
	 * if hit then:
	 *     cache_line.outcome = true;
	 *     SHCT[signature]++;
	 * else:
	 *     if replaced_cache_line.outcome != true;
	 *         SHCT[signature]--;
	 *     cache_line.outcome = true;
	 *     cache_line.signature = signature(newly calculated)
	 *     if SHCT[signature] == 0:
	 *         RRPV setting 3;
	 *     else:
	 *         RRPV setting 2;
	 */

	
	// 0. 처음에 모든 RRPV는 3으로 초기화된다.
	for(int i = 0; i < setN; i++) {
		rrpvList.push_back(vector<int> (wayN, 3));
	}
	
	// A: line_outcome, B: line_signature 설정
	for(int i = 0; i < wayN; i++) {
		lineOutcome.push_back(false);
		lineSignature.push_back(-1);
	}
	
	// SHCT 초기화
	
	// SHiP Signature를 위한 하위 몇비트?
	// 일단 16bit를 이용 (2^16)
}

// Hit의 경우!!!
void SHiP::access(int setIdx, int wayIdx, uint64_t address) {
	// 0-2. hit해서 access 되는 경우 RRPV는 0이 된다.
	rrpvList[setIdx][wayIdx] = 0;
	/*
	 * if hit then:
	 *     cache_line.outcome = true;
	 *     SHCT[signature]++;
	 */
	lineOutcome[wayIdx] = true;
	uint64_t sig = address >> 16;
	if(SHCT.find(sig) != SHCT.end()) {
		// 만약 table에 이미 존재하는 요소이면...
		SHCT[sig] += 1;
	} else {
		cout << sig << endl;
		// 그런데 무조건 반드시 이미 존재해야한다!!
		cout << "SHCT should already have such element but not... check error" << endl;
	}
}

// Miss의 경우!!!
// setIdx와 wayIdx 반환
pair<int, int> SHiP::replace(int setIdx, uint64_t address) {
	bool isFound = false;
	int wayIdx;

	/*
	 * 1. set에서 첫 3을 찾는다.
	 * 2. 3이 찾아지면 '5'로 이동
	 * 3. (찾아지지 않으면) 모든 RRPV++
	 *    (0->1, 1->2, 2->3)
	 * 4. '1'로 이동
	 * 5. 블록 교체대상으로 반환하고, 해당 블럭 RRPV는 2로 설정
	 */

	// 교체할 것을 찾을때까지 돌아라
	while(!isFound) {
		// 1. set에서 첫 3을 찾는다.
		for(int i = 0; i < rrpvList[setIdx].size(); i++) {
			if(rrpvList[setIdx][i] == 3) {
				// 2. 3이 찾아지면 '5'로 이동
				wayIdx = i;
				isFound = true;
				break;
			}
		}
		if(!isFound) {
			// 3. (찾아지지 않으면) 모든 RRPV++
			for(int i = 0; i < rrpvList[setIdx].size(); i++) {
				int val = rrpvList[setIdx][i];
				// (0->1, 1->2, 2->3)
				// 그냥 ++ 해도 되지만... 혹시 모를 오류를 위해서
				// if 문으로 구성함...
				if(val == 0 || val == 1 || val == 2) {
					rrpvList[setIdx][i]++; 
				} else {
					cout << "RRPV error... 3 cannot be ++ but it happened... check logic" << endl;
				}
			}
		}
		// 4. 1로 이동 (isFound == false)
	}

	/*
	 * else:
	 *     if replaced_cache_line.outcome != true;
	 *         SHCT[signature]--;
	 *     cache_line.outcome = true;
	 *     cache_line.signature = signature(newly calculated)
	 *     if SHCT[signature] == 0:
	 *         RRPV setting 3;
	 *     else:
	 *         RRPV setting 2;
	 */

	// cout << "came here " << wayIdx << endl;
	// cout << lineOutcome[wayIdx] << endl;

	// 오류 발견.... 처음으로 채우기 시작할 때는?? (사실상 교체라기 보다는 채우기)
	if(lineOutcome[wayIdx] != true) {
		if(lineSignature[wayIdx] != -1) { // 실제 교체인 경우
			if(SHCT.find(lineSignature[wayIdx]) != SHCT.end()) {
				SHCT[lineSignature[wayIdx]] -= 1;
				// for error detection...
				if(SHCT[lineSignature[wayIdx]] < 0) {
					cout << "SHCT cnt cannot be negative!!! find error!" << endl;
				}
			} else {
				// 절대 일어나서는 안되는 경우임...
				cout << "SHiP has fatal error.... should find it" << endl;
			}
		} else { // 그냥 채우기인 경우
		}
	}
	lineOutcome[wayIdx] = true;
	uint64_t sig = address >> 16; // right shift 16bit
	if(lineSignature[wayIdx] == -1) {
		// 처음으로 채워지기 시작하기 대문에 SHCT에 요소를 만들어준다....
		lineSignature[wayIdx] = sig;
		SHCT.insert(make_pair(sig, 0));
	} else {
		lineSignature[wayIdx] = sig;
	}
	// 5. 블록 교체대상으로 반환하고, 해당 블럭 RRPV는 2 또는 3으로 설정
	if(SHCT[sig] == 0) {
		rrpvList[setIdx][wayIdx] = 3;
	} else {
		rrpvList[setIdx][wayIdx] = 2;
	}
	return make_pair(setIdx, wayIdx);
}

int SHiP::getSHCTSize() {
	return SHCT.size();
}

/* >>>>> (end) <<<<< */


/*
 * >>>>>> BF Cache Replacement <<<<<
*/

BloomFilter::BloomFilter(int wayN, int setN) {
	this->wayN = wayN;
	this->setN = setN;

	/* Bloom Filter logic 관련 설명
	 * 우선 기본적으로 기본 BF, overlapped BF로 나눌 수 있다...
	 * 잘 생각해보면 기본 BF와 overlapped BF의 갯수는 동일하다...
	 * 그런데 추후에... 각 vertex가 어떤 BF를 쓸지가 굉장히 까다로울 수 있다...
	 * 따라서 미리 각 vertex가 어떤 BF를 선택하는지 여부를 미리 결정한다.
	 * vector<int> bfIdx, vector<int> ovlBFIdx, vector<bool> useOvlBF
	 * 
	*/

	// BF 초기화 (같은 수의 기본 BF 및 overlapped BF)
	for(int i = 0; i < BLOOM_FILTER_NUM; i++) {
		bfList.push_back(vector<int> (BF_LEN, 0));
		olBfList.push_back(vector<int> (BF_LEN, 0));
	}

	// 여기서 블록별 idx 결정하고 넘어가자
	// overlap 블록에 대한 계산도 미리하고 넘어갈 것!

	// 한 Block당 일반적인 vertex 갯수
	int blockVertexN = a_h / BLOOM_FILTER_NUM;
	
	// BF의 overlapping 계수(%)에 따라서 overlapped BF가 담당하기 시작할
	// vertex의 시작점이 달라진다...
	this->ovlStartIdx = blockVertexN*(100 - BF_OVERLAP)/100 + 1;
	
	// BF에 위해서 구한 blockVertexN으로 초기화해주면 된다...
	for(int i = 0; i < BLOOM_FILTER_NUM; i++) {
		// 각각의 BF와 overlappedBF가 담당할 vetex의 수 저장
		this->blkIdxVector.push_back(blockVertexN);
	}
	// 나머지는 균등하게 나눠주면 됨
	for(int i = 0; i < a_h - BLOOM_FILTER_NUM*blockVertexN; i++) {
		blkIdxVector[i] += 1;
	}
	
	cout << "Covered Vertices by BF: " << endl;
	for(int i = 0; i < blkIdxVector.size(); i++) {
		cout << blkIdxVector[i] << " ";
	}
	cout << endl;

	// 여기서 overlapped BF 초기화
	for(int i = 0; i < (BLOOM_FILTER_NUM-1); i++) {
		this->ovlBlkIdxVector.push_back(blockVertexN);
	}
	// 마지막 overlapped BF는 상대적으로 작은 양의 vertex를 담당하게 됨
	this->ovlBlkIdxVector.push_back(a_h - (BLOOM_FILTER_NUM-1)*blockVertexN - (ovlStartIdx-1));

	cout << "Covered Vertices by ovlBF: " << endl;
	for(int i = 0; i < ovlBlkIdxVector.size(); i++) {
		cout << ovlBlkIdxVector[i] << " ";
	}
	cout << endl;


	// 우선 각 vertex가 해당하는 BF의 index를 미리 저장해두자!! (편의를 위해서)
	for(int i = 0; i < BLOOM_FILTER_NUM; i++) {
		for(int j = 0; j < blkIdxVector[i]; j++) {
			bfIdx.push_back(i);
		}
	}

	// 우선 초반 p%의 vertex는 overlapped BF에 해당될수가 없다!
	for(int i = 0; i < (ovlStartIdx-1); i++) {
		ovlBFIdx.push_back(-1);
	}

	// 그 이후에 각 vertex에 해당하는 overlapped BF의 index를 저장해두자
	for(int i = 0; i < BLOOM_FILTER_NUM; i++) {
		for(int j = 0; j < ovlBlkIdxVector[i]; j++) {
			ovlBFIdx.push_back(i);
		}
	}

	// 그 뒤에 각 vertex가 overlapped BF를 이용해야하는지 여부를 저장
	for(int i = 0; i < BLOOM_FILTER_NUM; i++) {
		int remaining = blkIdxVector[i];
		for(int j = 0; j < (this->ovlStartIdx-1); j++) {
			useOvlBF.push_back(false);
			remaining -= 1;
		}
		while(remaining > 0) {
			useOvlBF.push_back(true);
			remaining -= 1;
		}
	}
	cout << "Bloom Filter Made... " << useOvlBF.size() << " vertices are covered!" << endl;
	cout << "BF: " << bfIdx.size() << ", ovlBF: " << ovlBFIdx.size() << endl;
	cout << "If tot cnts are same and same as vetex tot index, BF processing was correct." << endl;
	this->curRowIdx = 0;
}

// 첫 사이클 스캔 관련 코드
int BloomFilter::firstCycleAccess(uint64_t from, uint64_t to) {
	int whichBF = bfIdx[from];
	int whichOvlBF = ovlBFIdx[from];
	// 우선 일반 BF에 대해서 Cnt
	bfList[whichBF][to%BF_LEN] += 1;
	// overlap BF의 범위이면 Cnt
	// cout << "from: " << from << ", whichBF: " << whichBF << ", whichOvlBF: " << whichOvlBF << endl;
	if(whichOvlBF != -1) {
		olBfList[whichOvlBF][to%BF_LEN] += 1;
	}
}

// 교체할 대상에 대해서 알려줌
// 이미 set이 찬 상태에서 호출되므로
// 기본 루틴에만 집중할 것
pair<int, int> BloomFilter::replace(int setIdx, int blkIdx, bool useOvl, vector<int>* setVector, vector<int>* weightVector) {
	// 해당 set에서 교체 대상을 선택해야함
	// 이 때 blkIdx(BF idx)를 이용해서 해당 bloomFilter에서
	// set 안의 각 요소(n-way이면 n개) 중 cnt가 가장 낮은 것을
	// 교체 대상으로 선정한다...
	vector<int> cntVector;
	for(int i = 0; i < setVector->size(); i++) {
		cntVector.push_back(0);
	}

	for(int i = 0; i < setVector->size(); i++) {
		int bfIdx = setVector->at(i) % BF_LEN;
		// bf에서 cnt 가져와서 저장
		if(!useOvl) {
			cntVector[i] = bfList[blkIdx][bfIdx];
		} else {
			cntVector[i] = olBfList[blkIdx][bfIdx];
		}
	}

	// cnt가 가장 적은 교체 대상을 반환함...
	int minIdx = min_element(cntVector.begin(), cntVector.end()) - cntVector.begin();
	int target = setVector->at(minIdx);
	int weightIdx = weightVector->at(minIdx);
	return make_pair(target, weightIdx);
}

/* >>>>> (end) <<<<< */

MovingMedian::MovingMedian(int wayN, int setN) {
	this->wayN = wayN;
	this->setN = setN;
	mvmCurIdx = 0;
	int blockVertexN = a_h/MOV_MED_BLOCK_NUM;
	// BF에 위해서 구한 blockVertexN으로 초기화해주면 된다...
	for(int i = 0; i < MOV_MED_BLOCK_NUM; i++) {
		// 각각의 BF와 overlappedBF가 담당할 vetex의 수 저장
		mvmBlkIdxVector.push_back(blockVertexN);
	}
	// 나머지는 균등하게 나눠주면 됨
	for(int i = 0; i < a_h - MOV_MED_BLOCK_NUM*blockVertexN; i++) {
		mvmBlkIdxVector[i] += 1;
	}
}

int MovingMedian::firstCycleAccess(uint64_t from, uint64_t to) {
	int sum = 0;
	int idx = 0;
	for(int i = 0; i < MOV_MED_BLOCK_NUM; i++) {
		sum += mvmBlkIdxVector[i];
		if(sum >= from) {
			idx = i;
			break;
		}
	}
	// we should update median
	if(mvmCurIdx != idx) {
		// for avg calc
		uint64_t tmpSum = 0;
		for(int i = 0; i < mvmTmp.size(); i++) {
			tmpSum += mvmTmp[i];
			// cout << mvmTmp[i] << endl;
		}
		// ofstream output;
		// output.open("avgs.txt", ios::app);
		// output << (double)tmpSum/mvmTmp.size()<< endl;
		// output.close();
		medians.push_back((double)tmpSum/mvmTmp.size());

		// for median calc
		// nth_element(mvmTmp.begin(), mvmTmp.begin() + mvmTmp.size()/2, mvmTmp.end());
		// // ofstream output;
		// // output.open("medians.txt", ios::app);
		// // output << (double)mvmTmp[mvmTmp.size()/2] << endl;
		// // output.close();
		// medians.push_back((double)mvmTmp[mvmTmp.size()/2]);
		mvmTmp.clear();
		mvmCurIdx = idx;
	} else { // we keep tracking elements
		mvmTmp.push_back(to);
	}
}

pair<int, int> MovingMedian::replace(int from, int setIdx, vector<int>* setVector, vector<int>* weightVector) {
	vector<double> distanceVector;
	for(int i = 0; i < setVector->size(); i++) {
		distanceVector.push_back(0.0);
	}
	int median = medians[from];
	for(int i = 0; i < setVector->size(); i++) {
		int to = setVector->at(i);
		int distance = abs(median - to);
		distanceVector[i] = distance;
	}

	int maxIdx = max_element(distanceVector.begin(), distanceVector.end()) - distanceVector.begin();
	int target = setVector->at(maxIdx);
	int weightIdx = weightVector->at(maxIdx);
	return make_pair(target, weightIdx);
}