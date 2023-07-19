#include "public/otter-common.h"

static unique_id_t get_unique_id(void) {
  static unique_id_t id = 0;
  return __sync_fetch_and_add(&id, 1L);
}
