# Huffman-compression-program
+ See example.cpp

+ Structure of compressed file

  > |header| token records | data |
  > |---------|-----------|-----------|
  >
  > + **header**: 
  >   + padding bits: There may be padding at the end because it is stored in units of one byte
  >   + records size: number of records
  > + **token records**: token record * records size, See BuildTokenRecords and DecodeTokenRecords functions in huffman.cpp.
  >   + **token record**: 
  >     + level: tree level
  >     + token: 8-bit code
  > + **data**: compressed data

