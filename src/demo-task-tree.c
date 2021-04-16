#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <modules/task-tree.h>
#include <macros/debug.h>

#define NPARENTS 200
#define NCHILDREN 30

int main(void)
{
    LOG_INFO("running task tree demo...");

    tt_destroy_tree();

    assert(tt_init_tree());
    assert(tt_init_tree());

    tt_node_t *parents[NPARENTS] = {0};

    for (int k=0; k<NPARENTS; k++)
    {
        parents[k] = tt_new_node((tt_node_id_t){.value = 0xAABBCCDD + 17171*k},
            NCHILDREN/2);
        assert(parents[k] != NULL);
    }

    for (int k=0; k<NPARENTS; k++)
    {
        for (uint64_t i=0; i<NCHILDREN; i++)
        {
            assert(tt_add_child_to_node(parents[k],
                (tt_node_id_t){.value = i+k+NPARENTS}));
        }
    }

    assert(tt_write_tree(""));

    tt_destroy_tree();

    return 0;
}