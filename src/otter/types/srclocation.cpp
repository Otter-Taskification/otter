#include <iostream>
using std::cout;
using std::endl;
using std::ostream;

#include "otter/srclocation.hpp"

/* IMPLEMENTATION BELOW THIS LINE */

/* Source::obj */

class Source::obj
{
public:
    obj(const Source::tag &loc) : 
        id_(Source::obj::next()),
        iloc(loc) {};
    ~obj() = default;
    uint64_t getID() const { return id_; }
    auto getFile() const;
    int getLine() const;
    auto getFunc() const;
private:
    static uint64_t next()
    {
        static uint64_t id = 0;
        return id++;
    }
    const Source::tag &iloc;
    uint64_t id_;
};

auto Source::obj::getFile() const { return std::get<0>(this->iloc); }
int Source::obj::getLine() const { return std::get<1>(this->iloc); }
auto Source::obj::getFunc() const { return std::get<2>(this->iloc); }

ostream& operator<<(ostream& os, const Source::obj *obj)
{
    return (os 
        << "Source::obj" << "@[" << static_cast<const void*>(obj) << "]("
        << "id=" << obj->getID() << ", "
        << "loc=" << obj->getFile() << ":" << obj->getLine() << ", "
        << "func=" << obj->getFunc()
        << ")");
}

/* Source::reg */

class Source::reg
{
public:
    reg() = default;
    ~reg();
    Source::obj* operator[](const Source::tag&);
private:
    std::map<Source::tag, Source::obj*> iregistry;
};

Source::reg* Source::make_registry() { return new Source::reg; }
void Source::delete_registry(Source::reg *r) { delete r; }

Source::obj* Source::reg::operator[](const Source::tag &loc)
{
    if (iregistry.contains(loc))
    {
        // cout << "Source::reg: found, returning" << endl;
        return iregistry[loc];
    }
    Source::obj *obj = new Source::obj(loc);
    iregistry[loc] = obj;
    // cout << "Source::reg: not found, creating & returning" << endl;
    return obj;    
}

Source::reg::~reg()
{
    for (auto&[key, value] : iregistry)
    {
        cout << "Source::reg: deleting " << value << endl;
        delete value;
    }
}

/* Source::handle */

Source::handle::handle(Source::tag &src, Source::reg &reg)
{
    srcObj = reg[src];
}

uint64_t Source::handle::getID()
{
    return srcObj->getID();
}

ostream& Source::handle::stream(ostream& os) const
{
    return os << srcObj;
}

ostream& operator<<(std::ostream& os, Source::handle const& h)
{
    os << "Source::handle" << "@[" << static_cast<const void*>(&h) << "](";
    h.stream(os);
    os << ")";
    return os;
}

ostream& operator<<(ostream& os, Source::handle& h)
{
    os << "Source::handle(";
    h.stream(os);
    os << ")";
    return os;
}
