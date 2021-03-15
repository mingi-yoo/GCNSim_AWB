#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>

// 최대 vt 8까지를 고려해서 작성함

// 만약 왼쪽 상단을 한번 at진행하고 그 다음 또 왼쪽 상단을 at 진행한 상태라면
// (논문에서 제시된 figure와 같이...)
// <at_setting.config>
// 1 -1 -1 -1
// (0번)
// 1 -1 -1 -1 (이 depth에서 종료... 마지막 단위는 무조건 vt2로 쪼갬)

// 왼쪽 상단(0번 인덱스)은 다 쪼개고, 1번 2번 인덱스는 1번 쪼개고 4번은 안쪼갬
// <at_setting.config>
// 1 1 1 -1
// (0번)
// 1 1 1 1 (이 depth에서 종료되도록 코딩됨...)
// (1번)
// -1 -1 -1 -1
// (2번)
// -1 -1 -1 -1

// Represents a node of an n-ary tree
struct Node {
    uint64_t fromStart;
	uint64_t fromEnd;
	uint64_t toStart;
	uint64_t toEnd;
    std::vector<Node *>child;
    std::vector<std::vector<uint64_t>> new_csr;
};

std::vector<Node*> inorderLeaves;

// Utility function to create a new tree node
Node *newNode(uint64_t fromStart, uint64_t fromEnd, uint64_t toStart, uint64_t toEnd) {
    Node *newNd = new Node;
    newNd->fromStart = fromStart;
	newNd->fromEnd = fromEnd;
	newNd->toStart = toStart;
	newNd->toEnd = toEnd;
    for(uint64_t i = 0; i < (fromEnd-fromStart+1); i++) {
        newNd->new_csr.push_back(std::vector<uint64_t> ());
    }
    return newNd;
}

bool inThisNode(Node* nd, uint64_t from, uint64_t to) {
    if(nd->fromStart <= from && from <= nd->fromEnd) {
        if(nd->toStart <= to && to <= nd->toEnd) {
            return true;
        }
    }
    return false;
}

void getLeavesInorder(Node* root) {
    int total = root->child.size();
    if(total == 0) {
        inorderLeaves.push_back(root);
        return;
    }
    for(int i = 0; i < total-1; i++) {
        getLeavesInorder(root->child.at(i));
    }
    getLeavesInorder(root->child.at(total-1));
}
// Prints the n-ary tree level wise
std::vector<Node*> getLeaves(Node * root)
{   
    std::vector<Node *> leaves;
    if (root==NULL)
        return leaves;
  
    // Standard level order traversal code
    // using queue
    std::queue<Node *> q;  // Create a queue
    q.push(root); // Enqueue root 
    while (!q.empty())
    {
        int n = q.size();
        std::vector<Node *> levelLeaves;
        // If this node has children
        while (n > 0)
        {
            // Dequeue an item from queue and print it
            Node * p = q.front();
            q.pop();
            if(p->child.size() == 0) {
                levelLeaves.push_back(p);
            }
            std::cout << "(" << p->fromStart << "," << p->fromEnd << ")/";
		    std::cout << "(" << p->toStart << "," << p->toEnd << ") \n";
            // Enqueue all children of the dequeued item
            for (int i=0; i<p->child.size(); i++)
                q.push(p->child[i]);
            n--;
        }
  
        std::cout << std::endl; // Print new line between two levels
        if(levelLeaves.size() != 0) {
            for(int i = levelLeaves.size()-1; i >= 0; i--) {
                leaves.insert(leaves.begin(), levelLeaves.at(i));
            }
        }

    }
    return leaves;
}


int main(int argc, char** argv) {
	// 명령어: ./recursive_tiling v file output_path
	
	// vertex 숫자 저장
	uint64_t vertex_num = std::stoull(argv[1]); // vertex 숫자
	// config reader와 csr reader
	std::ifstream configReader;
	std::ifstream csrReader;
	configReader.open("at_setting.config");
	csrReader.open(argv[2]);

	// 우선, 루트 노드는 전체를 담당함
	Node *root = newNode(0, vertex_num, 0, vertex_num);

	// 첫번째 depth에 대해서 생각 ...
	// 전부 -1이면 루트노드만 생각하면 됨
	// 만약 하나라도 1이 존재하면 4분할 해주어야함
    std::vector<uint64_t> firstIdentifier;
    bool splitFirstDepth = false; // 첫번째 depth를 나눌지 말지 판단
    for(int read = 0; read < 4; read++) {
        uint64_t temp;
        configReader >> temp;
        if(temp == 1) {
            splitFirstDepth = true;
        }
        firstIdentifier.push_back(temp);
    } 

    std::vector<Node *> firstDepthNds;
    if(splitFirstDepth) {
        uint64_t q2 = ceil(vertex_num/2);
        Node* newNd;
        newNd = newNode(0, q2, 0, q2);
        (root->child).push_back(newNd); // 0번 순서
        firstDepthNds.push_back(newNd);
        newNd = newNode(q2+1, vertex_num, 0, q2);
        (root->child).push_back(newNd);
        firstDepthNds.push_back(newNd);
        newNd = newNode(0, q2, q2+1, vertex_num);
        (root->child).push_back(newNd);
        firstDepthNds.push_back(newNd);
        newNd = newNode(q2+1, vertex_num, q2+1, vertex_num);
        (root->child).push_back(newNd);
        firstDepthNds.push_back(newNd);
    }

    // 만약 firstIdentifier에서 1이었으면 이 블럭은 depth를 더 들어가야함
    for(int firstDepth = 0; firstDepth < 4; firstDepth++) {
        if(firstIdentifier.at(firstDepth) == 1) {
            std::vector<Node *> secondDepthNds;

            Node* parentNd;
            parentNd = firstDepthNds.at(firstDepth);
            uint64_t from_start = parentNd->fromStart;
            uint64_t from_end = parentNd->fromEnd;
            uint64_t from_q2 = ceil((parentNd->fromStart+parentNd->fromEnd)/2);
            uint64_t to_start = parentNd->toStart;
            uint64_t to_end = parentNd->toEnd;
            uint64_t to_q2 = ceil((to_start+to_end)/2);

            Node* newNd;
            newNd = newNode(from_start, from_q2, to_start, to_q2);
            (parentNd->child).push_back(newNd); // 0번 순서
            secondDepthNds.push_back(newNd);
            newNd = newNode(from_q2+1, from_end, to_start, to_q2);
            (parentNd->child).push_back(newNd);
            secondDepthNds.push_back(newNd);
            newNd = newNode(from_start, from_q2, to_q2+1, to_end);
            (parentNd->child).push_back(newNd);
            secondDepthNds.push_back(newNd);
            newNd = newNode(from_q2+1, from_end, to_q2+1, to_end);
            (parentNd->child).push_back(newNd);
            secondDepthNds.push_back(newNd);
            // 다음 줄들을 읽어들여라
            for(int read = 0; read < 4; read++) {
                uint64_t temp;
                configReader >> temp;
                if(temp == 1) {
                    Node* parentNd;
                    parentNd = secondDepthNds.at(firstDepth);
                    uint64_t from_start = parentNd->fromStart;
                    uint64_t from_end = parentNd->fromEnd;
                    uint64_t from_q2 = ceil((parentNd->fromStart+parentNd->fromEnd)/2);
                    uint64_t to_start = parentNd->toStart;
                    uint64_t to_end = parentNd->toEnd;
                    uint64_t to_q2 = ceil((to_start+to_end)/2);

                    Node* newNd;
                    newNd = newNode(from_start, from_q2, to_start, to_q2);
                    (parentNd->child).push_back(newNd); // 0번 순서
                    newNd = newNode(from_q2+1, from_end, to_start, to_q2);
                    (parentNd->child).push_back(newNd);
                    newNd = newNode(from_start, from_q2, to_q2+1, to_end);
                    (parentNd->child).push_back(newNd);
                    newNd = newNode(from_q2+1, from_end, to_q2+1, to_end);
                    (parentNd->child).push_back(newNd);
                }
            }
        }
    }

    std::cout << "TREE GENERATED" << std::endl;

    // 리프노드 받기
    std::vector<Node *> leaves = getLeaves(root);
    getLeavesInorder(root);
    // for(int i = 0; i < leaves.size(); i++) {
    //     Node* p = leaves.at(i);
    //     std::cout << "# " << i << std::endl;
    //     std::cout << "(" << p->fromStart << "," << p->fromEnd << ")/";
	// 	std::cout << "(" << p->toStart << "," << p->toEnd << ") \n";
    // }
    std::cout << "Inorder Result" << std::endl;
    for(int i = 0; i < inorderLeaves.size(); i++) {
        Node* p = inorderLeaves.at(i);
        std::cout << "# " << i << std::endl;
        std::cout << "(" << p->fromStart << "," << p->fromEnd << ")/";
		std::cout << "(" << p->toStart << "," << p->toEnd << ") \n";
    }
    configReader.close();

    // csr 읽어들이기
    std::vector<uint64_t> row;
    if(csrReader.is_open()) {
        std::string line, readTmp;
        getline(csrReader, line);
        std::stringstream ss(line);

        while(getline(ss, readTmp, ' ')) {
            if(readTmp == "\n") {
                break;
            }
            row.push_back(std::stoull(readTmp));
        }

        getline(csrReader, line);
        std::stringstream tt(line);

        std::cout << "CSR read fininshed" << std::endl;

        // 노드들에 배분해서 저장
        for(uint64_t from = 0; from < vertex_num; from++) {
            uint64_t v_cnt = row[from+1] - row[from];
            for(uint64_t j = 0; j < v_cnt; j++) {
                getline(tt, readTmp, ' ');
                uint64_t to = std::stoull(readTmp);
                Node* targetNd;
                for(int nd = 0; nd < inorderLeaves.size(); nd++) {
                    targetNd = inorderLeaves.at(nd);
                    if(inThisNode(targetNd, from, to)) {
                        break;
                    }
                }
                uint64_t newIdx = from - targetNd->fromStart;
                targetNd->new_csr[newIdx].push_back(to); 
            }
        }

        std::cout << "CSR distribution finished" << std::endl;
    }
    csrReader.close();
    
    std::vector<std::vector<uint64_t>> new_csr;
    for(int leafIdx = 0; leafIdx < inorderLeaves.size(); leafIdx++) {
        std::vector<std::vector<uint64_t>> tmp = inorderLeaves.at(leafIdx)->new_csr;
        for(uint64_t i = 0; i < tmp.size(); i++) {
            new_csr.push_back(tmp.at(i));
        }
    }

    // for (int i = 0; i < new_csr.size(); i++) {
    //     for (int j = 0; j <new_csr[i].size(); j++) {
    //         std::cout<<new_csr[i][j]<<" ";
    //     }
    //     std::cout<<std::endl;
    // }

    std::string output_arg = argv[3];

    std::string output_path = output_arg + "_rs.txt";
    std::cout << "saves to ..." << output_path << std::endl;
    std::ofstream output(output_path);

    output<<"0 ";
    uint64_t v_acm = 0;
    for (uint64_t i = 0; i < new_csr.size(); i++) {
        v_acm += new_csr[i].size();
        output << v_acm;
        if (i != new_csr.size() - 1)
            output << " ";
        else
            output << std::endl;
    }
    std::cout << "CSR first line printed" << std::endl;
    for (uint64_t i = 0; i < new_csr.size(); i++) {
        uint64_t v_cnt = new_csr[i].size();
        for (uint64_t j = 0; j < v_cnt; j++) {
            output << new_csr[i][j];
            if (i == new_csr.size() -1 && j == v_cnt -1 )
                output << std::endl;
            else
                output << " ";
        }
    }
    output.close();

    std::cout << "you should use xw input of 256 256 " << new_csr.size() << std::endl;

    std::string xw_output_path = output_arg + "_rs_xw.txt";
    std::cout << "xw saves to ..." << xw_output_path << std::endl;
    std::ofstream xw_output(xw_output_path);
    xw_output << "256\n256\n";
    xw_output << new_csr.size() << std::endl;

    xw_output.close();

	return 0;
}

/* 파라미터 순서 ---- "./vt v file_path output name" "*/