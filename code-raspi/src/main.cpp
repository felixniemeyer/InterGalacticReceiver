#include "main.h"

// Local dependencies
#include "arg_parse.h"
#include "error.h"
#include "horrors.h"
#include "magic.h"

// Global
#include <csignal>
#include <stdexcept>

bool app_running = true;

// clang-format off
#define ACT_CALIBRATE   "action_calibrate"
#define ACT_TUNER       "action_test_tuner"
#define ACT_RUN         "action_run"
// clang-format on

static std::string device_path;
static std::string action;

static void sighandler(int);
static bool parse_args(int argc, const char *argv[]);

int main(int argc, const char *argv[])
{

    try
    {
        signal(SIGINT, sighandler);
        signal(SIGTERM, sighandler);

        if (!parse_args(argc, argv)) return -1;

        const char *found_device = find_display_device();
        if (found_device == nullptr) THROWF("No connected display device found");
        device_path.assign(found_device);
        delete[] found_device;

        if (action == ACT_CALIBRATE) calibrate_readings();
        else if (action == ACT_TUNER) test_tuner();
        else if (action == ACT_RUN)
        {
            init_horrors(device_path.c_str());
            main_icr();
            cleanup_horrors();
        }
        printf("\nGoodbye!\n");
        return 0;
    }
    catch (const igr_exception &e)
    {
        fprintf(stderr, "Runtime error in file %s line %d: %s:\n%s\n", e.file(), e.line(), e.func(), e.what());
        cleanup_horrors();
        return -1;
    }
    catch (const std::bad_alloc &e)
    {
        fprintf(stderr, "Out of memory: %s\n", e.what());
        cleanup_horrors();
        return -1;
    }
    catch (...)
    {
        fprintf(stderr, "Unexpected error\n");
        cleanup_horrors();
        return -1;
    }
}

static void sighandler(int)
{
    app_running = false;
}

static bool parse_args(int argc, const char *argv[])
{
    auto parser = ArgumentParser("igr");
    parser.add_argument(ACT_CALIBRATE, "calib", "calibrate-readings", "Action: Calibrate readings");
    parser.add_argument(ACT_TUNER, "tuner", "test-tuner", "Action: Test tuner");
    parser.add_argument(ACT_RUN, "run", "", "Action: Run normally with sketches");
    parser.add_argument("help", "--help", "", "Displays this help message");
    parser.add_argument("dev", "", "--dev", "Device path (default: /dev/dri/card0)", STORE);

    bool success = parser.parse(argv, argc, stdout);
    if (!success || parser.get("help").is_set)
    {
        parser.print_usage(stdout);
        exit(success ? 0 : 1);
    }

    bool ok = true;
    bool multiple_actions = false;

    if (parser.get(ACT_CALIBRATE).is_set) action = ACT_CALIBRATE;
    if (parser.get(ACT_TUNER).is_set)
    {
        if (!action.empty()) multiple_actions = true;
        else action = ACT_TUNER;
    }
    if (parser.get(ACT_RUN).is_set)
    {
        if (!action.empty()) multiple_actions = true;
        else action = ACT_RUN;
    }

    if (multiple_actions || action.empty())
    {
        parser.print_usage(stdout);
        printf("\nBad arguments: Exactly one action must be specified\n");
        return false;
    }

    if (parser.get("dev").is_set) device_path.assign(parser.get("dev").value.c_str());

    if (!ok)
    {
        parser.print_usage(stdout);
        return false;
    }
    else return true;
}
