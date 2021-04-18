#include <gvc.h>
#include <string.h>
#include <modules/task-tree.h>
#include <modules/task-tree-graphviz.h>

#include <stdlib.h>

static GVC_t *gvc = NULL;

static char *gv_argv[] = {
    "dot",
    "-Tdot",
    "-omydot.dot"
};

static char *shape[] = {
    "box",
    "diamond",
    "folder",
    "note"
};

static char *color[] = {
    "red",
    "blue",
    "green"
};

#define node_fmt "%lu"

Agraph_t *
gv_new_graph(const char *fname)
{
    Agraph_t *g = NULL;
    g = agopen("OTTer Task Tree", Agdirected, 0);
    gvc = gvContext();
    gvParseArgs(gvc, sizeof(gv_argv)/sizeof(char *), gv_argv);
    return g;
}

bool        
gv_add_children_to_graph(Agraph_t *g, tree_node_id_t parent_id, array_element_t *child_ids, size_t length)
{
    Agnode_t *parent = NULL, *child = NULL;
    Agedge_t *edge = NULL;
    char parent_id_buffer[32] = {0};
    snprintf(parent_id_buffer, sizeof(parent_id_buffer), node_fmt, parent_id.value);
    parent = agnode(g, parent_id_buffer, true);
    agsafeset(parent, "shape", shape[rand()%(sizeof(shape)/sizeof(char*))], "");
    agsafeset(parent, "color", color[rand()%(sizeof(color)/sizeof(char*))], "");

    char child_id_buffer[32] = {0};
    for (size_t n=0; n<length; n++)
    {
        snprintf(child_id_buffer, sizeof(child_id_buffer), node_fmt, child_ids[n].value);
        child  = agnode(g, child_id_buffer, true);
        agsafeset(child, "shape", shape[rand()%(sizeof(shape)/sizeof(char*))], "");
        agsafeset(child, "color", color[rand()%(sizeof(color)/sizeof(char*))], "");
        edge   = agedge(g, parent, child, NULL, true);
    }

    return false;
}

bool        
gv_write_graph(Agraph_t *g)
{
    gvLayoutJobs(gvc, g);
    gvRenderJobs(gvc, g);
    return false;
}

bool
gv_finalise(Agraph_t *g)
{
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
    gvc = NULL;
    return true;
}