#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include "globalheap/globalheap.hh"
#include "KVPairLoader.h"

using namespace std;
using namespace alps;

size_t
PassThroughLoader::load(int reducerId) {
  // decode the index chunk.
  vector<pair<region_id, offset>> dataChunkPtrs;
  vector<int> numPairs;
  for (auto chunkPtr : chunkPtrs) {
    RRegion::TPtr<void> idxChunkPtr(chunkPtr.first, chunkPtr.second);
    byte* index = (byte*)idxChunkPtr.get();
    {
      // discard key's type
      index += sizeof(int);

      // decode # of partitions in this map output.
      int numBuckets;
      memcpy(&numBuckets, index, sizeof(int));
      index +=  sizeof(int);

      // drop unnecessary chunks ptr considering for  # of partitions.
      if (reducerId > numBuckets-1) {
        continue;
      }
      index = dropUntil(reducerId, index);

      // decode data chunk ptr and its size.
      region_id rid;
      memcpy(&rid, index, sizeof(region_id));
      index += sizeof(region_id);

      offset roffset;
      memcpy(&roffset, index, sizeof(offset));
      index += sizeof(offset);

      int size; // in bytes.
      memcpy(&size, index, sizeof(int));
      index += sizeof(int);

      int numPair; // # of pairs in this chunk.
      memcpy(&numPair, index, sizeof(int));
      index += sizeof(int);

      dataChunkPtrs.push_back(make_pair(rid, roffset));
      numPairs.push_back(numPair);
    }
  }

  // decode data chunks.
  for (size_t i=0; i<dataChunkPtrs.size(); ++i) {
    RRegion::TPtr<void> dataChunkPtr(dataChunkPtrs[i].first, dataChunkPtrs[i].second);
    byte* index = (byte*) dataChunkPtr.get();

    dataChunks.push_back(make_pair(i, chunk()));
    for (int j=0; j<numPairs[i]; ++j) {
      // [serKeySize, serKey, serValueSize, serValue]
      {
        int serKeySize;
        memcpy(&serKeySize, index, sizeof(int));
        index += sizeof(int);

        // who frees?
        byte* serKey = new byte[serKeySize];
        memcpy(serKey, index, serKeySize);
        index += serKeySize;

        int serValueSize;
        memcpy(&serValueSize, index, sizeof(int));
        index += sizeof(int);

        // who frees?
        unsigned char* serValue = new unsigned char[serValueSize];
        memcpy(serValue, index, serValueSize);
        index += serValueSize;

        KVPair pair(serKey, serKeySize, serValue, serValueSize, reducerId);
        dataChunks[i].second.push_back(pair);
      }
    }
  }

  return dataChunks.size();
}

vector<KVPair>
PassThroughLoader::fetch(int num) {
  auto first = flatChunk.begin();
  auto last = first + min(num, static_cast<int>(flatChunk.size()));

  auto res = vector<KVPair>(first, last);
  flatChunk.erase(first, last);

  return res;
}

void
PassThroughLoader::flatten() {
  for (auto&& [chunk_id, chunk] : dataChunks) {
    flatChunk.insert(flatChunk.end(), chunk.begin(), chunk.end());
  }
}

byte*
KVPairLoader::dropUntil(int partitionId, byte* index) {
  for (int i=0; i<partitionId; ++i) {
    index += sizeof(region_id) + sizeof(offset) + sizeof(int)*2;
  }

  return index;
}
