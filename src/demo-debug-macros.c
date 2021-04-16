#include <stdio.h>
#include <stdbool.h>

#include <macros/debug.h>

int main(void)
{

    LOG_DEBUG("debug");
    LOG_INFO("info");
    LOG_WARN("warning");
    LOG_ERROR("error");

    LOG_DEBUG_IF(true, "this debug should print");
    LOG_DEBUG_IF(false, "this debug shouldn't print");

    LOG_INFO_IF(true, "this info should print");
    LOG_INFO_IF(false, "this info shouldn't print");

    LOG_WARN_IF(true, "this warning should print");
    LOG_WARN_IF(false, "this warning shouldn't print");

    LOG_ERROR_IF(true, "this error should print");
    LOG_ERROR_IF(false, "this error shouldn't print");

    return 0;
}