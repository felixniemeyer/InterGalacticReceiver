#include "main.h"

// Local dependencies
#include "error.h"

// Global
#include <stdexcept>

int main(int argc, char *argv[])
{
    try
    {
        calibrate_readings();
        return 0;
    }
    catch (const igr_exception &e)
    {
        fprintf(stderr, "Runtime error: %s\n%s\n", e.what(), e.stack_trace());
        return -1;
    }
    catch (const std::bad_alloc &e)
    {
        fprintf(stderr, "Out of memory: %s\n", e.what());
        return -1;
    }
    catch (...)
    {
        fprintf(stderr, "Unexpected error\n");
        return -1;
    }
}
