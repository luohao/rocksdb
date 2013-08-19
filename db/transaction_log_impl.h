// Copyright 2008-present Facebook. All Rights Reserved.
#ifndef STORAGE_LEVELDB_INCLUDE_WRITES_ITERATOR_IMPL_H_
#define STORAGE_LEVELDB_INCLUDE_WRITES_ITERATOR_IMPL_H_

#include <vector>

#include "leveldb/env.h"
#include "leveldb/options.h"
#include "leveldb/types.h"
#include "leveldb/transaction_log.h"
#include "db/log_reader.h"
#include "db/filename.h"

namespace leveldb {

struct LogReporter : public log::Reader::Reporter {
  Env* env;
  Logger* info_log;
  virtual void Corruption(size_t bytes, const Status& s) {
    Log(info_log, "dropping %zu bytes; %s", bytes, s.ToString().c_str());
  }
};

class LogFileImpl : public LogFile {
 public:
  LogFileImpl(uint64_t logNum, WalFileType logType, SequenceNumber startSeq,
              uint64_t sizeBytes) :
    logNumber_(logNum),
    type_(logType),
    startSequence_(startSeq),
    sizeFileBytes_(sizeBytes) {
  }

  std::string Filename() const { return LogFileName("", logNumber_); }

  uint64_t LogNumber() const { return logNumber_; }

  WalFileType Type() const { return type_; }

  SequenceNumber StartSequence() const { return startSequence_; }

  uint64_t SizeFileBytes() const { return sizeFileBytes_; }

  bool operator < (const LogFile& that) const {
    return LogNumber() < that.LogNumber();
  }

 private:
  uint64_t logNumber_;
  WalFileType type_;
  SequenceNumber startSequence_;
  uint64_t sizeFileBytes_;

};

class TransactionLogIteratorImpl : public TransactionLogIterator {
 public:
  TransactionLogIteratorImpl(const std::string& dbname,
                             const Options* options,
                             const EnvOptions& soptions,
                             const SequenceNumber seqNum,
                             std::unique_ptr<VectorLogPtr> files,
                             SequenceNumber const * const lastFlushedSequence);

  virtual bool Valid();

  virtual void Next();

  virtual Status status();

  virtual BatchResult GetBatch();

 private:
  const std::string& dbname_;
  const Options* options_;
  const EnvOptions& soptions_;
  const SequenceNumber startingSequenceNumber_;
  std::unique_ptr<VectorLogPtr> files_;
  bool started_;
  bool isValid_;  // not valid when it starts of.
  Status currentStatus_;
  size_t currentFileIndex_;
  std::unique_ptr<WriteBatch> currentBatch_;
  unique_ptr<log::Reader> currentLogReader_;
  Status OpenLogFile(const LogFile* logFile, unique_ptr<SequentialFile>* file);
  LogReporter reporter_;
  SequenceNumber const * const lastFlushedSequence_;
  // represents the sequence number being read currently.
  SequenceNumber currentSequence_;

  void UpdateCurrentWriteBatch(const Slice& record);
  Status OpenLogReader(const LogFile* file);
};



}  //  namespace leveldb
#endif  //  STORAGE_LEVELDB_INCLUDE_WRITES_ITERATOR_IMPL_H_