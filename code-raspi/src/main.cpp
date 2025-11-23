#include "main.h"

// Local dependencies
#include "error.h"

// Global
#include <stdexcept>

int main(int argc, char *argv[])
{
    try
    {
        // calibrate_readings();
        test_tuner();
        return 0;
    }
    catch (const igr_exception &e)
    {
        fprintf(stderr, "Runtime error in file %s line %d: %s:\n%s\n", e.file(), e.line(), e.func(), e.what());
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
