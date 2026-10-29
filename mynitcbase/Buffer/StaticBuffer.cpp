#include "StaticBuffer.h"
#include<cstring>
// the declarations for this class can be found at "StaticBuffer.h"

unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];

// declare the blockAllocMap array
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];

StaticBuffer::StaticBuffer() {
  // copy blockAllocMap blocks from disk to buffer (using readblock() of disk)
  // blocks 0 to 3
  unsigned char tempBlock[BLOCK_SIZE];

  for (int i = 0; i < 4; i++) {
    // read disk block i into tempBlock
    Disk::readBlock(tempBlock, i);

    // copy into blockAllocMap
    memcpy(&blockAllocMap[i * BLOCK_SIZE], tempBlock, BLOCK_SIZE);
  }
  /* initialise metainfo of all the buffer blocks with
     dirty:false, free:true, timestamp:-1 and blockNum:-1
     (you did this already)
  */
  for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++) {
    metainfo[bufferIndex].free = true;
    metainfo[bufferIndex].blockNum = -1;
    metainfo[bufferIndex].dirty=false;
    metainfo[bufferIndex].timeStamp=-1;
  }
}

// write back all modified blocks on system exit
StaticBuffer::~StaticBuffer() {
  // copy blockAllocMap blocks from buffer to disk(using writeblock() of disk)
  unsigned char tempBlock[BLOCK_SIZE];

  for (int i = 0; i < 4; i++) {

    // copy from blockAllocMap into tempBlock
    memcpy(tempBlock, &blockAllocMap[i * BLOCK_SIZE], BLOCK_SIZE);

    // write to disk block i
    Disk::writeBlock(tempBlock, i);
  }
  /*iterate through all the buffer blocks,
    write back blocks with metainfo as free:false,dirty:true
    (you did this already)
  */
   for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++) {
    if(metainfo[bufferIndex].free==false && metainfo[bufferIndex].dirty==true)
    {
      Disk::writeBlock(StaticBuffer::blocks[bufferIndex],metainfo[bufferIndex].blockNum);
    }
  }
}

int StaticBuffer::getFreeBuffer(int blockNum){
    // Check if blockNum is valid (non zero and less than DISK_BLOCKS)
    // and return E_OUTOFBOUND if not valid.
    if (blockNum < 0 || blockNum > DISK_BLOCKS) {
      return E_OUTOFBOUND;
    }
    // increase the timeStamp in metaInfo of all occupied buffers.
    for (int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++) {
      if(metainfo[bufferIndex].free==false)
      metainfo[bufferIndex].timeStamp++;
    }
    // let bufferNum be used to store the buffer number of the free/freed buffer.
    int bufferNum=-1;

    // iterate through metainfo and check if there is any buffer free

    // if a free buffer is available, set bufferNum = index of that free buffer.

    // if a free buffer is not available,
    //     find the buffer with the largest timestamp
    //     IF IT IS DIRTY, write back to the disk using Disk::writeBlock()
    //     set bufferNum = index of this buffer
    int maxts=-1;
    for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++)
    {
      if(metainfo[bufferIndex].free == true)
      {
          bufferNum=bufferIndex;
          break;
      }
      if(maxts<metainfo[bufferIndex].timeStamp)
      {
        maxts=metainfo[bufferIndex].timeStamp;
        bufferNum=bufferIndex;
      }
    }
    if(bufferNum==-1)
    {
      return E_OUTOFBOUND;
    }
    if(metainfo[bufferNum].free==false && metainfo[bufferNum].dirty==true)
    {
      Disk::writeBlock(StaticBuffer::blocks[bufferNum],metainfo[bufferNum].blockNum);
    }
    // update the metaInfo entry corresponding to bufferNum with
    // free:false, dirty:false, blockNum:the input block number, timeStamp:0.
    metainfo[bufferNum].free = false;
    metainfo[bufferNum].blockNum = blockNum;
    metainfo[bufferNum].dirty=false;
    metainfo[bufferNum].timeStamp=0;
    // return the bufferNum.
    return bufferNum;
}

/* Get the buffer index where a particular block is stored
   or E_BLOCKNOTINBUFFER otherwise
*/
int StaticBuffer::getBufferNum(int blockNum) {
  // Check if blockNum is valid (between zero and DISK_BLOCKS)
  // and return E_OUTOFBOUND if not valid.
    if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
      return E_OUTOFBOUND;
    }
  // find and return the bufferIndex which corresponds to blockNum (check metainfo)
  for(int bufferIndex=0;bufferIndex<BUFFER_CAPACITY;bufferIndex++)
  {
    if(!metainfo[bufferIndex].free && metainfo[bufferIndex].blockNum == blockNum)
    {
        return bufferIndex;
    }
  }
  // if block is not in the buffer
  return E_BLOCKNOTINBUFFER;
}

int StaticBuffer::setDirtyBit(int blockNum){
    // find the buffer index corresponding to the block using getBufferNum().
    int bufferNum=getBufferNum(blockNum);
    // if block is not present in the buffer (bufferNum = E_BLOCKNOTINBUFFER)
    //     return E_BLOCKNOTINBUFFER
    if(bufferNum==E_BLOCKNOTINBUFFER)
    {
      return E_BLOCKNOTINBUFFER;
    }
    // if blockNum is out of bound (bufferNum = E_OUTOFBOUND)
    //     return E_OUTOFBOUND
    if (bufferNum==E_OUTOFBOUND) {
      return E_OUTOFBOUND;
    }
    // else
    //     (the bufferNum is valid)
    //     set the dirty bit of that buffer to true in metainfo
    metainfo[bufferNum].dirty=true;
    // return SUCCESS
    return SUCCESS;
}

int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.
    if (blockNum < 0 || blockNum >= DISK_BLOCKS) {
        return E_OUTOFBOUND;
    }
    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
    return (int) blockAllocMap[blockNum];
}