#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <vector>
#include <iostream>
#include "node.hpp"

#define TOKEN_SIZE 0x100 // 1byte

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

using token_size_t = uint8_t;
using record_size_t = uint16_t;

struct Header
{
	//uint8_t file_name_len;
	uint8_t remain_bits;
	record_size_t records_size;
};

struct TokenRecord
{
	token_size_t level;
	token_size_t token;
};

#pragma pack(pop)

using HufNode = PODNode<TokenCount, 2>;
using Code = std::vector<bool>;
using HufList = PODNode<HufNode*, 2>;

std::vector<uint32_t> MakeTokenTable(std::istream& is);

HufNode* MakePrefixTree(const std::vector<uint32_t>& token_table);

std::vector<Code> MakeCodeTable(HufNode* tree);

record_size_t BuildTokenRecords(HufNode* tree, TokenRecord token_records[]);

void Encode(std::istream& is, std::ostream& os, const std::vector<Code>& code_table, HufNode* tree);

HufNode* DecodeTokenRecords(TokenRecord token_records[], record_size_t records_size);

void Decode(std::istream& is, std::ostream& os);

void Encoding(std::istream& is, std::ostream& os);
void Decoding(std::istream& is, std::ostream& os);

}

#endif // HUFFMAN_H