#include "huffman.hpp"

#include <queue>
#include <stack>
//#include <fstream>
#include <memory>
//#include <cstring>

#define LEFT 0
#define RIGHT 1
//#define PREV 0
//#define NEXT 1

using namespace std;
namespace fs = std::filesystem;

//template <typename T>
//struct greater<T*>
//{
//	bool operator()(const T* left, const T* right)
//	{
//		return *left > *right;
//	}
//};

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

size_t WritePath(ostream& os, const NameType* str)
{
	auto cur = str;
	while (*cur) cur++;
	os.write((char*)str, (cur - str) * sizeof(NameType));
	return (cur - str);
}

static void BuildCodeTable(const HufNode* tree, size_t size, bool code[], vector<Code>& code_table);
inline void FillTokenTable(istream& is, vector<size_t>& token_table);

// preprocessing for encoding-----------------------------------------------
inline void FillTokenTable(istream& is, vector<size_t>& token_table)
{
	while (is.peek() != EOF)
		token_table[(token_t)is.get()]++;
}

vector<size_t> MakeTokenTable(istream& is)
{
	vector<size_t> token_table(TOKEN_MAX);

	FillTokenTable(is, token_table);

	return token_table;
}

// 메모리 누수 고려 없음
HufNode* MakePrefixTree(const vector<size_t>& token_table)
{
	priority_queue<HufNode*, vector<HufNode*>, ptr_greater<HufNode>> token_queue;

	// 큐 채우기
	for (int i = 0; i < TOKEN_MAX; i++) {
		if (token_table[i] > 0) {
			TokenCount token_cnt{ i, token_table[i] };
			token_queue.push(new HufNode{ token_cnt });
		}
	}

	if (token_queue.empty()) return nullptr;

	// 트리 구성
	while (token_queue.size() > 1) {
		HufNode* left = token_queue.top();
		token_queue.pop();
		HufNode* right = token_queue.top();
		token_queue.pop();

		HufNode* newNode = new HufNode{};

		newNode->get().count = left->get().count + right->get().count;
		newNode->set_link(LEFT, left);
		newNode->set_link(RIGHT, right);

		token_queue.push(newNode);
	}
	return token_queue.top();
}

vector<Code> MakeCodeTable(const HufNode* tree)
{
	bool code[TOKEN_MAX];
	vector<Code> code_table(TOKEN_MAX);

	if (tree) BuildCodeTable(tree, 0, code, code_table);

	return code_table;
}

static void BuildCodeTable(const HufNode* node, size_t size, bool code[], vector<Code>& code_table)
{
	// 트리의 자식의 개수는 2개이거나 0개이므로 양쪽 다 검사할 필요 없음
	if (node->link(LEFT)) {
		code[size] = LEFT;
		BuildCodeTable(node->link(LEFT), size + 1, code, code_table);

		code[size] = RIGHT;
		BuildCodeTable(node->link(RIGHT), size + 1, code, code_table);
	}
	else {
		for (int i = 0; i < size; i++)
			code_table[node->get().token].code.set(i, code[i]);
		code_table[node->get().token].size = size;
	}
}

uint16_t BuildTokenRecords(const HufNode* node, TokenRecord token_records[])
{
	if (!node) return 0;

	stack<const HufNode*> node_stack;
	stack<token_t> level_stack;
	uint16_t num_of_records = 0;

	node_stack.push(node);
	level_stack.push(0); // 레벨은 0부터 시작함.

	while (!node_stack.empty()) {
		node = node_stack.top();
		node_stack.pop();
		token_t level = level_stack.top();
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

// 메모리 누수 고려 없음
HufNode* DecodeTokenRecords(const TokenRecord token_records[], uint16_t records_size)
{
	stack<HufNode*> tree_stack;

	for (uint16_t i = 0; i < records_size; i++) {
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

	if (tree_stack.size() == 1)
		return tree_stack.top();
	else {
		while (!tree_stack.empty()) {
			DestroyPODNodes(tree_stack.top());
			tree_stack.pop();
		}
		return nullptr;
	}
}

// encoding process-------------------------------------------------------------

int ConvertToHufCode(istream& is, ostream& os, const vector<Code>& code_table)
{
	token_t character = 0;
	int current_bit = 0;

	while (is.peek() != EOF) {
		token_t idx = is.get();
		for (int i = 0; i < code_table[idx].size; i++) {
			character |= code_table[idx].code.test(i) << current_bit;
			if (++current_bit == TOKEN_BITS) { // 하나씩 하지말고 한번에 하게 바꿀 것
				os.put(character);
				current_bit = character = 0;
			}
		}
	}

	if (current_bit)
		os.put(character);

	return (TOKEN_BITS - current_bit) % TOKEN_BITS;
}

void Encode(istream& is, ostream& os, const vector<Code>& code_table, const HufNode* tree)
{
	// 토큰 리코드 빌드
	TokenRecord token_records[TOKEN_MAX];
	uint16_t records_size = BuildTokenRecords(tree, token_records);

	// 마지막에 쓰기위한 헤더 공간 확보
	HufHeader header{ 0, records_size };
	auto header_pos = os.tellp();
	os.write((char*)&header, sizeof(HufHeader));
	
	// 트리 저장
	os.write((char*)token_records, sizeof(TokenRecord) * records_size);

	// 허프만 코드로 변환
	auto temp_os_pos = os.tellp();
	header.padding_bits = ConvertToHufCode(is, os, code_table);

	auto last_pos = os.tellp();
	header.data_size = last_pos - temp_os_pos;

	os.seekp(header_pos);
	os.write((char*)&header, sizeof(HufHeader));
	os.seekp(last_pos);
}

void Encoding(istream& is, ostream& os)
{
	auto first_pos = is.tellg();

	// 토큰 테이블
	vector<size_t> token_table = MakeTokenTable(is);

	// 트리
	//PODNodeGuard<HufNode> tree{ MakePrefixTree(token_table) };
	unique_ptr<HufNode, void(*)(HufNode*)> tree{ MakePrefixTree(token_table), &DestroyPODNodes };

	/*if (!tree)
		throw exception{ "Cannot build Huffman tree" };*/

	//코드 테이블
	vector<Code> code_table = MakeCodeTable(tree.get());

	// 파일에 출력
	is.clear();
	is.seekg(first_pos);
	Encode(is, os, code_table, tree.get());
}

void EncodeFile(const fs::path& file_path, ostream& os)
{
	ifstream is{ file_path, ios_base::binary };
	if (!is.good()) {
		auto ec = make_error_code(huf_errc::invalid_fstream);
		throw fs::filesystem_error{ "EncodeFile", file_path, ec };
	}

	FileHeader header{ TYPE_REGULAR_FILE };
	auto header_pos = os.tellp();
	os.write((char*)&header, sizeof(FileHeader));

	header.name_size = WritePath(os, file_path.filename().c_str());

	if (header.name_size >= FILENAME_MAX)
		throw out_of_range{ "Invalid file name length: " + to_string(header.name_size) };

	Encoding(is, os);

	auto current_pos = os.tellp();
	os.seekp(header_pos);
	os.write((char*)&header, sizeof(FileHeader));
	os.seekp(current_pos);
}

void EncodeDirectory(const fs::path& dir_path, ostream& os)
{
	auto directory_iter = fs::directory_iterator(dir_path);

	DirectoryHeader header{ TYPE_DIRECTORY };
	auto header_pos = os.tellp();
	os.write((char*)&header, sizeof(DirectoryHeader));

	header.name_size = WritePath(os, dir_path.filename().c_str());

	if (header.name_size >= FILENAME_MAX)
		throw out_of_range{ "Invalid file name length: " + to_string(header.name_size) };

	for (const auto& path : directory_iter) {
		Compress(path, os);
		header.data_size++;
	}

	auto current_pos = os.tellp();
	os.seekp(header_pos);
	os.write((char*)&header, sizeof(DirectoryHeader));
	os.seekp(current_pos);
}

void Compress(const fs::path& path, ostream& os)
{
	if (fs::is_directory(path)) {
		EncodeDirectory(path, os);
	}
	else if (fs::is_regular_file(path)) {
		EncodeFile(path, os);
	}
	else {
		error_code ec = make_error_code(huf_errc::invalid_file_type);
		throw fs::filesystem_error{ "Compress", path, ec };
	}
}

// decoding process-------------------------------------------------------------

void ConvertToToken(std::istream& is, std::ostream& os, const HufNode* tree, int padding_bits, size_t max_len)
{
	if (!tree) return;
	const HufNode* node = tree;

	token_t bits = 0;
	while (is.peek() != EOF && max_len--) {
		bits = is.get();

		for (uint8_t i = 0; i < TOKEN_BITS;) {
			if (node->link(bits & RIGHT)) {
				node = node->link(bits & RIGHT);
				bits >>= 1;
				i++;
			}
			else {
				os.put(node->get().token);
				node = tree;
				if (is.peek() == EOF && (i + padding_bits) >= TOKEN_BITS)
					break;
			}
		}
	}
	if (!node->link(bits & RIGHT))
		os.put(node->get().token);
}

void Decode(istream& is, ostream& os)
{
	HufHeader header{};
	is.read((char*)&header, sizeof(Header));

	if (header.records_size > TOKEN_MAX)
		throw out_of_range{ "Invalid file header: Invalid token records size: " + to_string(header.records_size) };

	TokenRecord token_records[TOKEN_MAX];
	is.read((char*)token_records, sizeof(TokenRecord) * header.records_size);
	PODNodeGuard<HufNode> tree{ DecodeTokenRecords(token_records, header.records_size) };

	if (!tree && header.records_size)
		throw exception{ "Invalid file header: Invalid token records: Huffman tree build faild" };
	
	ConvertToToken(is, os, tree.get(), header.padding_bits, header.data_size);
}

void Decoding(istream& is, ostream& os)
{
	Decode(is, os);
}

void DecodeFile(istream& is, const fs::path& file_path)
{
	ofstream os{ file_path, ios_base::binary };

	if (!os.good()) {
		error_code ec = make_error_code(huf_errc::invalid_fstream);
		throw fs::filesystem_error{ "DecodeFile", file_path, ec };
	}
	
	Decoding(is, os);
}

void DecodeDirectory(istream& is, const fs::path& prefix, size_t num_of_file)
{
	fs::create_directory(prefix);

	for (size_t i = 0; i < num_of_file; i++)
		Decompress(is, prefix);
}

void Decompress(istream& is, const fs::path& prefix)
{
	Header header{};
	is.read((char*)&header, sizeof(Header));

	if (header.name_size >= FILENAME_MAX || !header.name_size)
		throw out_of_range{ "Invalid file header: Invalid file name length: " + to_string(header.name_size) };

	NameType name[FILENAME_MAX];
	is.read((char*)name, sizeof(NameType) * header.name_size);
	name[header.name_size] = 0;

	switch (header.type) {
	case TYPE_REGULAR_FILE:
		DecodeFile(is, prefix / name);
		break;
	case TYPE_DIRECTORY:
		DecodeDirectory(is, prefix / name, header.data_size);
		break;
	}
}

fs::path DecompressRetFilename(istream& is, const fs::path& prefix)
{
	Header header{};
	is.read((char*)&header, sizeof(Header));

	if (header.name_size >= FILENAME_MAX || !header.name_size)
		throw out_of_range{ "Invalid file header: Invalid file name size: " + to_string(header.name_size) };

	NameType name[FILENAME_MAX];
	is.read((char*)name, sizeof(NameType) * header.name_size);
	name[header.name_size] = 0;

	switch (header.type) {
	case TYPE_REGULAR_FILE:
		DecodeFile(is, prefix / name);
		break;
	case TYPE_DIRECTORY:
		DecodeDirectory(is, prefix / name, header.data_size);
		break;
	}
	return name;
}

}