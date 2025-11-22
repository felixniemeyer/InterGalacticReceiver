#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>

class igr_exception : public std::runtime_error
{
  private:
    const std::string _stack_trace;

  public:
    igr_exception(const std::string &msg, const std::string &trace)
        : std::runtime_error(msg)
        , _stack_trace(trace)
    {
    }

    const char *stack_trace() const
    {
        return _stack_trace.c_str();
    }
};

void throwf(const char *fmt, ...);
void throwf_errno(const char *fmt, ...);

#endif
