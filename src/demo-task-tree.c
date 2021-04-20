#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <modules/task-tree.h>
#include <macros/debug.h>

#define NPARENTS 5
#define NCHILDREN 3

int main(void)
{
    LOG_INFO("running task tree demo...");

    tree_destroy();

    assert(tree_init());
    assert(tree_init());

    tree_node_t *parents[NPARENTS] = {0};

    for (int k=0; k<NPARENTS; k++)
    {
        parents[k] = tree_add_node(
            (tree_node_id_t){.value = k}, NCHILDREN/2);
        assert(parents[k] != NULL);
    }

    for (int k=0; k<NPARENTS; k++)
    {
        for (uint64_t i=0; i<NCHILDREN; i++)
        {
            assert(tree_add_child_to_node(parents[k],
                (tree_node_id_t){.value = i+k+NPARENTS}));
        }
    }

    assert(tree_write(""));

    tree_destroy();

    return 0;
}