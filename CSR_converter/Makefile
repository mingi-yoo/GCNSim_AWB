all: xwgen csr_translator vt vt_dynamic_0 vt_dynamic_1 vt_dynamic_2 vt_dynamic_0_3 recursive_tiling recursive_tiling_undirected recursive_tiling_resched
xwgen: xwgen.o
	g++ -o xwgen xwgen.o
xwgen.o: xwgen.cpp
	g++ -std=c++11 -c -o xwgen.o xwgen.cpp
csr_translator: csr_translator.o
	g++ -o csr_translator csr_translator.o
csr_translator.o: csr_translator.cpp
	g++ -std=c++11 -c -o csr_translator.o csr_translator.cpp
vt: vt.o
	g++ -o vt vt.o
vt.o: vtmake.cpp
	g++ -std=c++11 -c -o vt.o vtmake.cpp

vt_dynamic_0: vt_dynamic_0.o
	g++ -o vt_dynamic_0 vt_dynamic_0.o
vt_dynamic_0.o: vtmake_dynamic_vt_0.cpp
	g++ -std=c++11 -c -o vt_dynamic_0.o vtmake_dynamic_vt_0.cpp

vt_dynamic_1: vt_dynamic_1.o
	g++ -o vt_dynamic_1 vt_dynamic_1.o
vt_dynamic_1.o: vtmake_dynamic_vt_1.cpp
	g++ -std=c++11 -c -o vt_dynamic_1.o vtmake_dynamic_vt_1.cpp


vt_dynamic_2: vt_dynamic_2.o
	g++ -o vt_dynamic_2 vt_dynamic_2.o
vt_dynamic_2.o: vtmake_dynamic_vt_2.cpp
	g++ -std=c++11 -c -o vt_dynamic_2.o vtmake_dynamic_vt_2.cpp

vt_dynamic_0_3: vt_dynamic_0_3.o
	g++ -o vt_dynamic_0_3 vt_dynamic_0_3.o
vt_dynamic_0_3.o: vtmake_dynamic_vt_0_3.cpp
	g++ -std=c++11 -c -o vt_dynamic_0_3.o vtmake_dynamic_vt_0_3.cpp

recursive_tiling: recursive_tiling.o
	g++ -o recursive_tiling recursive_tiling.o
recursive_tiling.o: recursive_tiling_maker.cpp
	g++ -std=c++11 -c -o recursive_tiling.o recursive_tiling_maker.cpp

recursive_tiling_undirected: recursive_tiling_undirected.o
	g++ -o recursive_tiling_undirected recursive_tiling_undirected.o
recursive_tiling_undirected.o: recursive_tiling_maker_uni_directed.cpp
	g++ -std=c++11 -c -o recursive_tiling_undirected.o recursive_tiling_maker_uni_directed.cpp

recursive_tiling_resched: recursive_tiling_resched.o
	g++ -o recursive_tiling_resched recursive_tiling_resched.o
recursive_tiling_resched.o: recursive_tiling_maker_reschedule.cpp
	g++ -std=c++11 -c -o recursive_tiling_resched.o recursive_tiling_maker_reschedule.cpp

clean:
	rm *.o xwgen csr_translator vt vt_dynamic_0 vt_dynamic_1 vt_dynamic_2 vt_dynamic_0_3 recursive_tiling recursive_tiling_undirected recursive_tiling_resched