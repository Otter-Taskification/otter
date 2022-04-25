#include <ostream>
#include <cstdint>
#include <tuple>
#include <map>

using std::ostream;

namespace Source {

using tag = std::tuple<const char *const, int, const char *const>;

// Opaque class represents the information needed for OTF2 to define a 
// source location
class obj;

// Opaque class which maps SrcLoc to SrcLocObj*
class reg;

reg *make_registry();
void delete_registry(reg *);

// Handle around opaque class SrcLocObj (PIMPL)
class handle
{
public:
    handle(tag &src, reg &reg);
    ~handle() = default;
    uint64_t getID();
    ostream& stream(ostream&) const;
private:
    const obj *srcObj;
};

};

// Declare ostream operators
ostream& operator<<(ostream& os, const Source::handle& h);
ostream& operator<<(ostream&, Source::handle const&);
