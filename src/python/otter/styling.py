from collections import defaultdict

"""
Styling dictionaries for __main__.py
"""

# Map region type to node color
colormap_region_type = defaultdict(lambda: 'white', **{
    'initial_task': 'green',
    'implicit_task': 'fuchsia',
    'explicit_task': 'cyan',
    'parallel': 'yellow',
    'single_executor': 'blue',
    'single_other': 'orange',
    'taskwait': 'red',
    'taskgroup': 'purple',
    'barrier_implicit': 'darkgreen',

    # Workshare regions
    'loop': 'brown',
    'taskloop': 'orange',

    # For colouring by endpoint
    'enter': 'green',
    'leave': 'red'
})

colormap_edge_type = defaultdict(lambda: 'black', **{
    'taskwait': 'red',
    'taskgroup': 'red',
})

shapemap_region_type = defaultdict(lambda: 'circle', **{
    'initial_task': 'square',
    'implicit_task': 'square',
    'explicit_task': 'square',
    'parallel': 'parallelogram',

    # Sync regions
    'taskwait': 'octagon',
    'taskgroup': 'octagon',
    'barrier_implicit': 'octagon',

    # Workshare regions
    'loop': 'diamond',
    'taskloop': 'diamond',
    'single_executor': 'diamond',
})
