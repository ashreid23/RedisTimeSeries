/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/
#ifndef TSDB_H
#define TSDB_H

#include "redismodule.h"
#include "compaction.h"
#include "consts.h"
#include "chunk.h"
#include "indexer.h"

typedef struct CompactionRule {
    RedisModuleString *destKey;
    int32_t timeBucket;
    AggregationClass *aggClass;
    int aggType;
    void *aggContext;
    struct CompactionRule *nextRule;
} CompactionRule;

typedef struct Series {
    RedisModuleDict* chunks;
    Chunk* lastChunk;
    int32_t retentionTime;
    CompactionRule *rules;
    timestamp_t lastTimestamp;
    double lastValue;
    Label *labels;
    RedisModuleString *keyName;
    RedisModuleString *srcKey;
    short labelsCount;
    short maxSamplesPerChunk;
} Series;

typedef struct SeriesIterator {
    Series *series;
    RedisModuleDictIter *dictIter;
    Chunk *currentChunk;
    int chunkIteratorInitialized;
    ChunkIterator chunkIterator;
    api_timestamp_t maxTimestamp;
    api_timestamp_t minTimestamp;
} SeriesIterator;

Series *NewSeries(RedisModuleString *keyName, Label *labels, size_t labelsCount, int32_t retentionTime, short maxSamplesPerChunk);
void FreeSeries(void *value);
void CleanLastDeletedSeries(RedisModuleCtx *ctx, RedisModuleString *key);
void FreeCompactionRule(void *value);
size_t SeriesMemUsage(const void *value);
int SeriesAddSample(Series *series, api_timestamp_t timestamp, double value);
int SeriesHasRule(Series *series, RedisModuleString *destKey);
int SeriesDeleteRule(Series *series, RedisModuleString *destKey);
int SeriesSetSrcRule(Series *series, RedisModuleString *srctKey);
int SeriesDeleteSrcRule(Series *series, RedisModuleString *srctKey);

CompactionRule *SeriesAddRule(Series *series, RedisModuleString *destKeyStr, int aggType, long long timeBucket);
int SeriesCreateRulesFromGlobalConfig(RedisModuleCtx *ctx, RedisModuleString *keyName, Series *series, Label *labels, size_t labelsCount);
size_t SeriesGetNumSamples(Series *series);

// Iterator over the series
SeriesIterator SeriesQuery(Series *series, api_timestamp_t minTimestamp, api_timestamp_t maxTimestamp);
int SeriesIteratorGetNext(SeriesIterator *iterator, Sample *currentSample);
void SeriesIteratorClose(SeriesIterator *iterator);

CompactionRule *NewRule(RedisModuleString *destKey, int aggType, int timeBucket);
#endif /* TSDB_H */
