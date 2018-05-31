#include <io.h>
#include <fcntl.h>

#include "platform_win32.h"

#include "LogStream.h"
#include "Config.h"


namespace tinylog {

LogStream::LogStream(FILE *file, pthread_mutex_t &mutex, pthread_cond_t &cond, bool &already_swap) :
  pt_front_buff_(std::make_unique<Buffer>(BUFFER_SIZE)),
  pt_back_buff_(std::make_unique<Buffer>(BUFFER_SIZE)),
  file_(file),
  i_line_(0),
  pt_func_(nullptr),
  pt_tm_base_(nullptr),
  mutex_(mutex),
  cond_(cond),
  already_swap_(already_swap)
{

  Utils::CurrentTime(&tv_base_, &pt_tm_base_);
}

LogStream::~LogStream()
{
  if (file_ != nullptr)
  {
    fclose(file_);
    file_ = nullptr;
  }
}

intptr_t LogStream::Write(const void *data, intptr_t ToWrite)
{
  return InternalWrite(data, ToWrite);
}

/*
 * Swap front buffer and back buffer.
 * This function should be locked.
 */
void LogStream::SwapBuffer()
{
//  Buffer *pt_tmp = pt_front_buff_.release();
//  pt_front_buff_.reset(pt_back_buff_.release());
//  pt_back_buff_.reset(pt_tmp);
  std::swap(pt_front_buff_, pt_back_buff_);
}

/*
 * Write buffer data to log file.
 * This function should be locked.
 */
void LogStream::WriteBuffer()
{
  pt_back_buff_->Flush(file_);
  pt_back_buff_->Clear();
}

LogStream &LogStream::operator<<(const char *pt_log)
{
  InternalWrite(pt_log, strlen(pt_log));

  return *this;
}

void LogStream::UpdateBaseTime()
{
  struct timeval tv;
  time_t now = time(nullptr);
  tv.tv_sec = (long)now;
  tv.tv_usec = 0;
  struct tm *tm = localtime(&now);

  if (tv.tv_sec != tv_base_.tv_sec)
  {
    int new_sec = pt_tm_base_->tm_sec + int(tm->tm_sec - tv_base_.tv_sec);
    if (new_sec >= 60)
    {
      pt_tm_base_->tm_sec = new_sec % 60;
      int new_min = pt_tm_base_->tm_min + new_sec / 60;
      if (new_min >= 60)
      {
        pt_tm_base_->tm_min = new_min % 60;
        int new_hour = pt_tm_base_->tm_hour + new_min / 60;
        if (new_hour >= 24)
        {
          Utils::CurrentTime(&tv, &pt_tm_base_);
        }
        else
        {
          pt_tm_base_->tm_hour = new_hour;
        }
      }
      else
      {
        pt_tm_base_->tm_min = new_min;
      }
    }
    else
    {
      pt_tm_base_->tm_sec = new_sec;
    }

    tv_base_ = tv;
  }
  else
  {
    tv_base_.tv_usec = tv.tv_usec;
  }
}

intptr_t LogStream::InternalWrite(const void *data, intptr_t ToWrite)
{
  intptr_t Result = ToWrite;
  UpdateBaseTime();

  pthread_mutex_lock(&mutex_);

  if (pt_front_buff_->TryAppend(data, ToWrite) < 0)
  {
    SwapBuffer();
    already_swap_ = true;
    pt_front_buff_->TryAppend(data, ToWrite);
  }

  pthread_cond_signal(&cond_);
  pthread_mutex_unlock(&mutex_);
  return Result;
}

} // namespace tinylog
