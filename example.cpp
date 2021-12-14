//#include <iostream>
//#include <fstream>
//#include <cstdio>
//#include "huffman.hpp"
//
//using namespace std;
//using namespace Huffman;
//
//#define LEFT 0
//#define RIGHT 1
//
//void PreOrderPrintNodes(HufNode* node, int depth)
//{
//	if (!node) return;
//
//	for (int i = 0; i < depth; i++)
//		cout << ". ";
//	//cout << (int) << ':' <<  << endl;
//	printf("%d : %5i\n", (int)node->data.token, node->data.count);
//
//	PreOrderPrintNodes(node->link(LEFT), depth + 1);
//	PreOrderPrintNodes(node->link(RIGHT), depth + 1);
//}
//
//void PrintTreeByPreOrder(HufNode* tree)
//{
//	cout << "-----------token tree------------\n";
//	PreOrderPrintNodes(tree, 0);
//}
//
//void PrintTokenTable(const vector<uint32_t>& token_table)
//{
//	cout << "-----------token table-----------\n";
//	for (int i = 0; i < token_table.size(); i++) {
//		printf("%03d : %5i||", i, token_table[i]);
//		if (!(i % 5)) cout << endl;
//	}
//	cout << endl;
//}
//
//void PrintCodeTable(const vector<Code>& token_table)
//{
//	cout << "-----------code table------------\n";
//	for (int i = 0; i < token_table.size(); i++) {
//		if (token_table[i].empty()) continue;
//
//		printf("%03d : %c : ", i, i);
//
//		for (bool flag : token_table[i])
//			cout << flag;
//		
//		cout << endl;
//	}
//}
//
//void encoding()
//{
//	ifstream is{ "test.txt" };
//	ofstream os{ "test.bin", ios_base::binary };
//
//	// 토큰 테이블
//	vector<uint32_t> token_table = MakeTokenTable(is);
//	PrintTokenTable(token_table);
//
//	// 트리
//	PODNodeGuard<HufNode> tree{ MakePrefixTree(token_table) };
//	PrintTreeByPreOrder(tree.get());
//	
//	//코드 테이블
//	vector<Code> code_table = MakeCodeTable(tree.get());
//	PrintCodeTable(code_table);
//
//	// 파일에 출력
//	is.clear();
//	is.seekg(0);
//	Encode(is, os, code_table, tree.get());
//	flush(os);
//}
//
//void decoding()
//{
//	//// 디코딩의 부분 루틴------------------------------------------------
//	//PODNodeGuard<HufNode> tree;
//	//TokenRecord token_records[TOKEN_SIZE];
//	//uint8_t records_size = BuildTokenRecords(tree.get(), token_records);
//	//tree = DecodeTokenRecords(token_records, records_size);
//	//PrintTreeByPreOrder(tree.get());
//	//// ------------------------------------------------------------------
//
//	ifstream is{ "test.bin", ios_base::binary };
//	ofstream os{ "result.txt" };
//	Decode(is, os);
//	flush(os);
//}
//
//int main()
//{
//	encoding();
//	decoding();
//}