#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <modules/task-tree.h>
#include <macros/debug.h>

int main(void)
{
    LOG_INFO("running task tree demo...");

    tt_destroy_tree();

    assert(tt_init_tree());
    assert(tt_init_tree());

    tt_destroy_tree();
    assert(tt_init_tree());

    tt_destroy_tree();

    return 0;
}