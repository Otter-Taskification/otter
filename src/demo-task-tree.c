#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <modules/task-tree.h>
#include <macros/debug.h>

#define NCHILDREN 6

int main(void)
{
    LOG_INFO("running task tree demo...");

    tt_destroy_tree();

    assert(tt_init_tree());
    assert(tt_init_tree());

    tt_node_t *parent = tt_new_node(
        (tt_node_id_t){.ptr = (void*) 0xBEEF}, NCHILDREN/2);
    assert(parent != NULL);

    for (int i=0; i<NCHILDREN; i++)
    {
        assert(tt_add_child_to_node(parent, (tt_node_id_t){.value = i}));
    }

    assert(false == tt_write_tree(""));

    tt_destroy_tree();
    assert(tt_init_tree());

    tt_destroy_tree();

    return 0;
}