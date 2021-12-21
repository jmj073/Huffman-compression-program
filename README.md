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

??????
C:\Users\user\Desktop>Huffman.exe -e -s qthttpserver
source: 678002bytes, destination: 558922bytes, decrease: 119080bytes, 17.5634%

C:\Users\user\Desktop>Huffman.exe -d -s qthttpserver.huf
source: 558922bytes, destination: 678080bytes, increase: 119158bytes, 21.3193%
