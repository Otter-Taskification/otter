#define CONCAT_I(x, y) x##y
#define CONCAT(x, y) CONCAT_I(x, y)
#define LINEID CONCAT(_i_line_id_, __LINE__)

#define WITH_CONTEXT(before, onfailure, after)                                 \
  for (int LINEID = 0; LINEID < 1 && ((before) || ((onfailure) && 0));         \
       (after), LINEID++)

#define WITH_LOCK(obj)                                                         \
  WITH_CONTEXT(pthread_mutex_lock(&obj), 0, pthread_mutex_unlock(&obj))
