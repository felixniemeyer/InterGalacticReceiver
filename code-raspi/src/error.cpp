#include "error.h"

// Global
#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const size_t buf_sz = 4096;

static void print_stacktrace(std::string &trace)
{
    char *buf = new char[buf_sz];
    const size_t depth = 64;
    void *array[depth];
    size_t size = backtrace(array, depth);

    trace.clear();

    for (size_t i = 0; i < size; ++i)
    {
        Dl_info info;
        if (dladdr(array[i], &info) && info.dli_sname)
        {
            int status = 0;
            char *demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
            snprintf(buf, buf_sz, "#%zu %p %s + %td",
                     i,
                     array[i],
                     (status == 0 && demangled) ? demangled : info.dli_sname,
                     (char *)array[i] - (char *)info.dli_saddr);
            if (!trace.empty()) trace.append("\n");
            trace.append(buf);
            free(demangled);
        }
        else
        {
            snprintf(buf, buf_sz, "#%zu %p ???", i, array[i]);
            if (!trace.empty()) trace.append("\n");
            trace.append(buf);
        }
    }
}

void throwf(const char *fmt, ...)
{
    va_list args;

    char *buf = new char[buf_sz];
    va_start(args, fmt);
    vsnprintf(buf, buf_sz, fmt, args);
    va_end(args);

    std::string trace;
    print_stacktrace(trace);
    throw igr_exception(buf, trace);
}

void throwf_errno(const char *fmt, ...)
{
    va_list args;

    char *buf1 = new char[buf_sz];
    va_start(args, fmt);
    vsnprintf(buf1, buf_sz, fmt, args);
    va_end(args);

    char *buf2 = new char[buf_sz];
    snprintf(buf2, buf_sz, "%s: %d: %s", buf1, errno, strerror(errno));

    std::string trace;
    print_stacktrace(trace);
    throw igr_exception(buf2, trace);
}
