#include "huffman.hpp"

#include <queue>
#include <stack>

#define LEFT 0
#define RIGHT 1
#define PREV 0
#define NEXT 1

using namespace std;

template <typename T>
struct ptr_greater
{
	bool operator()(const T* left, const T* right)
	{
		return *left > *right;
	}
};

namespace Huffman
{

static void BuildCodeTable(HufNode* tree, int size, bool code[], vector<Code>& code_table);

vector<uint32_t> MakeTokenTable(istream& is)
{
	vector<uint32_t> token_table(TOKEN_SIZE);

	while (is.peek() != EOF)
		token_table[(token_size_t)is.get()]++;

	return token_table;
}

HufNode* MakePrefixTree(const vector<uint32_t>& token_table)
{
	priority_queue<HufNode*, vector<HufNode*>, ptr_greater<HufNode>> token_queue;

	// 큐 채우기
	for (int i = 0; i < TOKEN_SIZE; i++) {
		if (token_table[i] > 0) {
			TokenCount token_cnt{ i, token_table[i] };
			token_queue.push(new HufNode{ token_cnt });
		}
	}

	if (token_queue.empty()) return nullptr;

	// 트리 구성
	while (token_queue.size() > 1) {
		HufNode* newNode = new HufNode{};

		HufNode* left = token_queue.top();
		token_queue.pop();
		HufNode* right = token_queue.top();
		token_queue.pop();

		newNode->get().count = left->get().count + right->get().count;
		newNode->set_link(LEFT, left);
		newNode->set_link(RIGHT, right);

		token_queue.push(newNode);
	}
	
	return token_queue.top();
}

vector<Code> MakeCodeTable(HufNode* tree)
{
	bool code[TOKEN_SIZE];
	vector<Code> code_table(TOKEN_SIZE);

	if (tree) BuildCodeTable(tree, 0, code, code_table);

	return code_table;
}

static void BuildCodeTable(HufNode* node, int size, bool code[], vector<Code>& code_table)
{
	// 트리의 자식의 개수는 2개이거나 0개이므로 양쪽 다 검사할 필요 없음
	if (node->link(LEFT)) {
		code[size] = LEFT;
		BuildCodeTable(node->link(LEFT), size + 1, code, code_table);

		code[size] = RIGHT;
		BuildCodeTable(node->link(RIGHT), size + 1, code, code_table);
	}
	else {
		code_table[node->get().token].reserve(size);
		for (int i = 0; i < size; i++)
			code_table[node->get().token].push_back(code[i]);
	}
}

record_size_t BuildTokenRecords(HufNode* node, TokenRecord token_records[])
{
	if (!node) return 0;

	stack<HufNode*> node_stack;
	stack<token_size_t> level_stack;
	record_size_t num_of_records = 0;

	node_stack.push(node);
	level_stack.push(0); // 레벨은 0부터 시작함.

	while (!node_stack.empty()) {
		node = node_stack.top();
		node_stack.pop();
		token_size_t level = level_stack.top();
		level_stack.pop();

		if (node->link(LEFT)) {
			node_stack.push(node->link(RIGHT));
			level_stack.push(level + 1);
			node_stack.push(node->link(LEFT));
			level_stack.push(level + 1);
		}
		else {
			token_records[num_of_records].level = level;
			token_records[num_of_records].token = node->get().token;

			num_of_records++;
		}
	}
	return num_of_records;
}

void Encode(istream& is, ostream& os, const vector<Code>& code_table, HufNode* tree)
{
	// 토큰 리코드 빌드
	TokenRecord token_records[TOKEN_SIZE];
	record_size_t records_size = BuildTokenRecords(tree, token_records);

	// 마지막에 쓰기위한 헤더 공간 확보
	Header header{ 0, records_size };
	os.write((char*)&header, sizeof(Header));

	// 트리 저장
	os.write((char*)token_records, sizeof(TokenRecord) * records_size);

	// 메인 데이터 쓰기
	token_size_t character = 0;
	int current_bit = 0;

	while (is.peek() != EOF) {
		token_size_t idx = is.get();
		for (bool flag : code_table[idx]) {
			character |= flag << current_bit;
			if (++current_bit == 8) {
				os.put(character);
				current_bit = character = 0;
			}
		}
	}
	if (current_bit) {
		os.put(character);
		os.seekp(0);
		header.padding_bits = current_bit;
		os.write((char*)&header, sizeof(Header));
	}
}

HufNode* DecodeTokenRecords(TokenRecord token_records[], record_size_t records_size)
{
	if (!records_size) return nullptr;

	stack<HufNode*> tree_stack;

	for (record_size_t i = 0; i < records_size; i++) {
		HufNode* new_node = new HufNode{ token_records[i].token,
										 token_records[i].level };
		while (!tree_stack.empty() && 
				tree_stack.top()->get().count == new_node->get().count) {
			HufNode* left = tree_stack.top();
			tree_stack.pop();
			HufNode* right = new_node;

			new_node = new HufNode{};
			new_node->get().count = left->get().count - 1;
			new_node->set_link(LEFT, left);
			new_node->set_link(RIGHT, right);
		}
		tree_stack.push(new_node);
	}

	return tree_stack.top();
}

void Decode(std::istream& is, std::ostream& os)
{
	Header header{};
	is.read((char*)&header, sizeof(Header));

	TokenRecord token_records[TOKEN_SIZE];
	is.read((char*)token_records, sizeof(TokenRecord) * header.records_size);
	PODNodeGuard<HufNode> tree{ DecodeTokenRecords(token_records, header.records_size) };

	if (!tree) return;

	HufNode* node = tree.get();

	while (is.peek() != EOF) {
		token_size_t bits = is.get();

		for (uint8_t i = 0; i < 8;) {
			if (node->link(bits & RIGHT)) {
				node = node->link(bits & RIGHT);
				bits >>= 1; i++;
			}
			else {
				os.put(node->get().token);
				node = tree.get();
				if (is.peek() == EOF && i >= header.padding_bits)
					break;
			}
		}
	}
}

void Encoding(istream& is, ostream& os)
{
	// 토큰 테이블
	vector<uint32_t> token_table = MakeTokenTable(is);

	// 트리
	PODNodeGuard<HufNode> tree{ MakePrefixTree(token_table) };

	//코드 테이블
	vector<Code> code_table = MakeCodeTable(tree.get());

	// 파일에 출력
	is.clear();
	is.seekg(0);
	Encode(is, os, code_table, tree.get());
	flush(os);
}

void Decoding(istream& is, ostream& os)
{
	Decode(is, os);
	flush(os);
}

}
