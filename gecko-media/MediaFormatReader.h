#ifndef MediaFormatReader_h
#define MediaFormatReader_h

#include "mozilla/Maybe.h"

struct DecoderData
{
  mozilla::Maybe<uint32_t> mNextStreamSourceID;
};

class MediaFormatReader
{
 public:
  MediaFormatReader();
 private:
  ~MediaFormatReader();
  DecoderData mAudio;
};

#endif
