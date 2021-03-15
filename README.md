# GCNSim

A GCN Accelerator Simulator with DRAMsim3.

## 1. 기본 구조

### RunSimulator.cpp (메인 함수)

Argument 처리 및 기본적인 모듈 로드, 시뮬레이션 수행

1. IniParser에서 ini 설정 불러옴.
2. DataReader에서 data 불러옴.
3. DRAMInterface를 이용해서 DRAM Simulation 객체 생성
4. 추후 추가할 것!!

###  DRAMInterface.cpp (DRAMsim3 호출 및 활용 인터페이스)

1. 생성자에서 설정파일을 기준으로 DRAMsim3를 호출... 생성자에서 dramsim에 callback function 넣는 것도 이곳에서 이루어짐

2. void updateCycle() {};

3. void DRAMRequest(주소, is_write) {}; => DRAM에 읽기 쓰기 요청

4. bool IsReadComplete() {};

5. unit64_t GetReadData() {}; => 읽어들인 주소 반환

6. ReadCompleteCallback() {}; => dramsim read callback

7. WriteCompleteCallback() {}; => dramsim write callback

   참고) DRAMSIMInterface 내부에는 read_complete 값이 존재해서 이것을 통해 접근 가능

### GlobalBuffer.cpp (전체 버퍼)

```c++
private:
	uint64_t gbsize;
	DRAMInterface *dram;
```

### Accelerator.cpp (accelerator)

```c++
private:
	uint64_t tot_acc;
	DRAMInterface *dram;
```

### VertexReader (Vertex 읽어들임)

VertexReader.h

```c++
class VertexReader {
public:
	VertexReader(int id, DRAMInterface *dram);
	~VertexReader();
	void ReceiveData(queue<uint64_t> data);
	void Request();
private:
	int id;
	int q_space;
	bool req_need;
	DRAMInterface *dram;
	queue<uint64_t> vq; //save vertex data from DRAM
};
```

### EdgeReader (Edge 읽어들임)

EdgeReader.h

```c++
class EdgeReader {
public:
	EdgeReader(int id, DRAMInterface *dram);
	~EdgeReader();
	void ReceiveData(queue<uint64_t> data);
	void ReceiveData(uint64_t vertex);
private:
	int id;
	uint64_t cur_v;
	uint64_t prev_v;
	DRAMInterface *dram;
	queue<uint64_t> eq; //save edge data from DRAM
};
```

### XWReader (계산된 XW를 읽어들임)

```c++
class XWReader {
public:
	XWReader(int id, GlobalBuffer *gb, Accelerator *acc);
	~XWReader();
private:
	GlobalBuffer *gb;
	Accelerator *acc;
};
```

### XController

x와 w를 읽어들이고 이를 acceleration을 통해서 계산하여 XW를 계산

```c++
class XController {
public:
	XController(int id,
				DRAMInterface *dram,
				DataController *data, 
				GlobalBuffer *gb, 
				XBuffer *xb, 
				Accelerator *acc);
	~XController();
private:
	int id = id;
	DRAMInterface *dram;
	DataController *data;
	GlobalBuffer *gb;
	XBuffer *xb;
	Accelerator *acc;
	XReader *xr;
	WReader *wr;
};
```

### XBuffer

x 크기가 크기 때문에 나누어서 buffer를 통해 이용... dram 연결 필요

```c++
class XBuffer {
public:
	XBuffer(uint64_t xbsize, DRAMInterface *dram);
	~XBuffer();
private:
	int id;
	uint64_t xbsize;
	DRAMInterface *dram;
};
```

### XReader

```c++
class XReader {
public:
	XReader(int id, DRAMInterface *dram);
	~XReader();
private:
	int id;
	DRAMInterface *dram;
	queue<uint64_t> xq; //save x data from DRAM
};
```

### WReader

```c++
class WReader {
public:
	WReader(GlobalBuffer *gb, Accelerator *acc);
	~WReader();
private:
	GlobalBuffer *gb;
	Accelerator *acc;
};
```



## 2. Class Diagram 및 Flow

추후 진행해가면서 추가할 것!



## #. 수정 및 개발 현황

### (7/17)

* .cpp 파일 추가
* 메인함수 (RunSimulator.cpp) 추가
* DRAMInterface.* 약간 수정

### (7/20)

* DataController.h, DataController.cpp 추가
  + 데이터를 컨트롤러에게 분배하는 역할
* Common.h 추가
* Acontroller.cpp 내 RunController 함수 작성중

### (7/22)

* multiprocessing(simulator상에서)을 위한 id 추가
* RunController 계속 작성중
* VertexReader, EdgeReader 데이터 passing 처리중

### (7/25)

* debugging 함수 추가 (main.cpp - PrintState())
* multiprocessing(simulator상에서)을 위한 offset 추가

### (7/27)
* Distributer.cpp 완성 (데이터 분할)
* 이제 본격적으로 모듈별 작업 함수 시작해야함
* 복잡할 수 있으니 다시 한번 코드 정리 및 코드 구조 정리할 것

### (7/28)
* AController, XController 삭제
* XWReader, WReader에서 accelerator 제외
* EdgeReader, VertexReader 구현 (Maybe)
* Common.h에 여러 struct 구현되었으니 참고할 것 

### (7/29)
* XReader 구현 (Maybe)
* accelerator, buffer 구현해야함
* RunSimulator에서 현재까지 만든 거 잘 돌아가나 실험중

### (7/30)
* XReader 전면 수정 (XBuffer 고려)
* XBuffer, GlobalBuffer 일부 구현
* GlobalBuffer의 경우 total weight size가 버퍼의 크기보다 작을 경우 처음 시작때 DRAM에 웨이트 전부 요청

### (8/3)
* XW Operation debugging 중

### (8/4)
* XW Operation 완료 (Maybe)
* AXW Operation 작업해야함

### (8/5)
* XW Operation 잘 되나 여러가지 실험 해봄
* 여러가지 수정을 거쳐서, 아마도 잘 되는 것 같
* AXW Operation 진행중

### (8/6)
* XBuffer 삭제, XReader로 통합
* EdgeReader, VertexReader 클래스 구조 작성
* GlobalBuffer, Accelerator, XWReader 마저 해야함

### (8/8)
* XWReader 작성완료 (Maybe)
* GlobalBuffer 작업

### (8/10)
* XWReader 수정
* Accelerator, GlobalBuffer 작성완료 (Maybe)
* RunSimulator 작성중

### (8/11)
* RunSimulator 작성완료
* debugging 필요할

### (8/12)
* 일단 돌아감
* 여러가지 실험해보면서 잘 작동하는게 맞는지 테스트 필요

### (8/14)
* 데이터 읽는 방식 변경
* 기존의 데이터 포멧을 두개로 나눔 (x data와 a data)
* 따라서 각각 따로 텍스트 파일을 받아서 읽어옴
* accelerator에 acc 계산 사이클 조절 추가

### (8/16)
* DRAM 개수 조절 추가
* But, Weight 구분을 위해 GlobalBuffer와 DRAMInterface사이 함수 수정 필요
* 그 외 몇 가지 더 수정 필요

### (8/17)
* 일단 완성 (제발)
* distributer가 데이터를 나누는 과정에서 약간의 문제 (정말 특정 예외상황에서)
* 어차피 수정할 거, 일단 나둔다.