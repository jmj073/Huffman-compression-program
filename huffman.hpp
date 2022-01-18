#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <limits>
#include <bitset>

#include "node.hpp"
#include "huf_exception.hpp"

#define TOKEN_MAX 0x100 // 1byte
#define TOKEN_BITS 8

#define TYPE_REGULAR_FILE 0
#define TYPE_DIRECTORY 1

namespace Huffman
{

struct TokenCount
{
	unsigned char token;
	unsigned count;

	bool operator<(const TokenCount& other) const
	{
		return count < other.count;
	}
	bool operator>(const TokenCount& other) const
	{
		return count > other.count;
	}
	bool operator==(const TokenCount& other) const
	{
		return count == other.count;
	}
	bool operator<=(const TokenCount& other) const
	{
		return count <= other.count;
	}
	bool operator>=(const TokenCount& other) const
	{
		return count >= other.count;
	}
	bool operator!=(const TokenCount& other) const
	{
		return count != other.count;
	}
};

#pragma pack(push, 1)

using token_t = uint8_t;

struct HufHeader
{
	uint16_t padding_bits : 3;
	uint16_t records_size : 13;
	size_t data_size;
};

struct Header
{
	uint16_t type : 1;
	uint16_t name_size : 15; // name is std::filesystem::path::value_type
	size_t data_size;
};

using NameType = std::filesystem::path::value_type;

using FileHeader = Header;
using DirectoryHeader = Header;

struct TokenRecord
{
	token_t level;
	token_t token;
};

#pragma pack(pop)

using HufNode = PODNode<TokenCount, 2>;
//using Code = std::vector<bool>;

struct Code
{
	std::bitset<TOKEN_MAX> code;
	size_t size;
};

// preprocessing for encoding------------------------------
std::vector<size_t> MakeTokenTable(std::istream& is);
HufNode* MakePrefixTree(const std::vector<size_t>& token_table);
std::vector<Code> MakeCodeTable(const HufNode* tree);
uint16_t BuildTokenRecords(const HufNode* tree, TokenRecord token_records[]);

// encoding process----------------------------------------

// return last bit position + 1
int ConvertToHufCode(std::istream& src, std::ostream& dst, const std::vector<Code>& code_table);

void Encode(std::istream& src, std::ostream& dst, const std::vector<Code>& code_table, const HufNode* tree);

void Encoding(std::istream& src, std::ostream& dst);

void EncodeFile(const std::filesystem::path& file_path, std::ostream& dst);

void EncodeDirectory(const std::filesystem::path& dir_path, std::ostream& dst);

void Compress(const std::filesystem::path& src_path, std::ostream& dst);

// decoding process----------------------------------------
HufNode* DecodeTokenRecords(const TokenRecord token_records[], uint16_t records_size);

void ConvertToToken(std::istream& src, std::ostream& dst, const HufNode* tree, int padding_bits, size_t max_len = std::numeric_limits<size_t>::max());

void Decode(std::istream& src, std::ostream& dst);

void Decoding(std::istream& src, std::ostream& dst);

void DecodeFile(std::istream& src, const std::filesystem::path& prefix);

void DecodeDirectory(std::istream& src, const std::filesystem::path& prefix, size_t num_of_file);

void Decompress(std::istream& src, const std::filesystem::path& prefix);

// 패스를 얻고 싶다면 이것?!
std::filesystem::path DecompressRetFilename(std::istream& src, const std::filesystem::path& prefix);

}

#endif // HUFFMAN_H