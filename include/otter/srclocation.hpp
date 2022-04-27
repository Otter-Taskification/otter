#ifdef __cplusplus
#include <ostream>
#include <cstdint>
#include <tuple>
#include <map>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
using std::ostream;

namespace Source {

using source_tag = std::tuple<const char *const, int, const char *const>;

// Opaque class represents the information needed for OTF2 to define a 
// source location
class object;

// Opaque class which maps SrcLoc to SrcLocObj*
class source_registry;

extern "C" {
source_registry *source_registry_make();
void source_registry_delete(source_registry *);
}

// Handle around opaque class SrcLocObj (PIMPL)
class source_handle
{
public:
    source_handle(source_tag src, source_registry &reg);
    source_handle(const char *file, int line, const char *func, source_registry &reg);
    source_handle(const char *file, int line, const char *func, source_registry *reg);
    ~source_handle() = default;
    uint64_t getID();
    const char* get_file();
    const char* get_func();
    int get_line();
    ostream& stream(ostream&) const;
private:
    const object *srcObj;
};

extern "C" {
source_handle *source_handle_make(const char*, int, const char*, source_registry*);
void source_handle_delete(source_handle*);
const char* source_handle_get_file(source_handle*);
const char* source_handle_get_func(source_handle*);
int source_handle_get_line(source_handle*);
}

};

// Declare ostream operators
ostream& operator<<(ostream& os, const Source::source_handle& h);
ostream& operator<<(ostream&, Source::source_handle const&);

#else

typedef struct source_tag source_tag;
typedef struct source_handle source_handle;
typedef struct source_registry source_registry;

source_registry *source_registry_make(void);
void source_registry_delete(source_registry *);

source_handle *source_handle_make(const char*, int, const char*, source_registry*);
void source_handle_delete(source_handle*);
const char* source_handle_get_file(source_handle*);
const char* source_handle_get_func(source_handle*);
int source_handle_get_line(source_handle*);

#endif // #ifdef __cplusplus
