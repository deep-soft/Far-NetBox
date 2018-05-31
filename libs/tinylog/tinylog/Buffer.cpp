#include <io.h>
#include <memory>

#include "Buffer.h"

namespace tinylog {

Buffer::Buffer(uint32_t capacity) :
  size_(0),
  capacity_(capacity)
{
  pt_data_ = nb::chcalloc(capacity);
}

Buffer::~Buffer()
{
  nb_free(pt_data_);
}

/*
 * Append time and log to buffer.
 * This function should be locked.
 * return value:
 * 0  : success
 * -1 : fail, buffer full
 */
int32_t Buffer::TryAppend(const void *pt_log, intptr_t ToWrite)
{
  /*
   * date: 11 byte
   * time: 13 byte
   * line number: at most 5 byte
   * log level: 9 byte
  */
  size_t append_len = ToWrite; // 24 + strlen(pt_file) + 5 + strlen(pt_func) + 9 + strlen(pt_log);

  if (append_len + size_ > capacity_)
  {
    return -1;
  }

  memmove(pt_data_ + size_, pt_log, ToWrite);
  intptr_t n_append = ToWrite;
  if (n_append > 0)
  {
    size_ += n_append;
  }

  return 0;
}

void Buffer::Clear()
{
  size_ = 0;
}

size_t Buffer::Size() const
{
  return size_;
}

size_t Buffer::Capacity() const
{
  return capacity_;
}

int32_t Buffer::Flush(FILE *file)
{
  size_t n_write = fwrite(pt_data_, 1, size_, file);
  if (n_write != size_)
  {
    // error
    return -1;
  }

  return 0;
}

} // namespace tinylog
