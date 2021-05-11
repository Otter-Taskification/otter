import itertools
from collections import deque, defaultdict

l1 = deque([{'id': 0, 'type': 'foo'}, {'id': 0, 'type': 'bar'}, {'id': 0, 'type': 'bar'}, {'id': 0, 'type': 'bar'}, {'id': 0, 'type': 'baz'}])
l2 = deque([{'id': 1, 'type': 'foo'}, {'id': 1, 'type': 'baz'}])
l3 = deque([{'id': 2, 'type': 'foo'}, {'id': 2, 'type': 'bar'}, {'id': 2, 'type': 'baz'}])


def popleftwhile(predicate, queue):
    while predicate(queue[0]):
        yield queue.popleft()


def pad_events(dict_of_queues: dict):
    events = defaultdict(list)
    num_queues = len(dict_of_queues)
    print(f"There are {num_queues} event queues here")
    """Get the set of events for each location up to the next collective event"""
    chunks = [(key, list(popleftwhile(lambda x: x['type'] != 'baz', q))) for key, q in dict_of_queues.items()]
    """Pad the events for each location so all have the same length"""
    padded = list(zip(*list(itertools.zip_longest(*list(c[1] for c in chunks)))))
    """Append the padded event lists to each location's events"""
    for (key, _), padded in zip(chunks, padded):
        events[key].extend(padded)
    for key, value in events.items():
        print(key)
        for index, val in enumerate(value):
            print(index, val)


if __name__ == "__main__":
    print("Running example...")
    print(l1)
    print(l2)
    print(l3)
    pad_events({'0': l1, '1': l2, '2': l3})
