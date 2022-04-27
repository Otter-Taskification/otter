#include <iostream>
using std::cout;
using std::endl;
using std::ostream;

#include "otter/srclocation.hpp"

/* Source::object */

class Source::object {
public:
    object(Source::source_tag tag) : 
        id_(Source::object::next()),
        iloc(tag)
    {
        cout << "Source::object(file=" << std::get<0>(iloc) << ", line=" << std::get<1>(iloc) << ", func=" << std::get<2>(iloc) << ")" << endl;
    };
    ~object() = default;
    uint64_t getID() const { return id_; }
    const char* getFile() const { return std::get<0>(this->iloc); };
    int getLine() const { return std::get<1>(this->iloc); };
    const char* getFunc() const { return std::get<2>(this->iloc); };
private:
    static uint64_t next() {
        static uint64_t id = 0;
        return id++;
    }
    const Source::source_tag iloc;
    uint64_t id_;
};

ostream& operator<<(ostream& os, const Source::object *object) {
    return (os 
        << "Source::object" << "@[" << static_cast<const void*>(object) << "]("
        << "id=" << object->getID() << ", "
        << "loc=" << object->getFile() << ":" << object->getLine() << ", "
        << "func=" << object->getFunc()
        << ")");
}

/* Source::source_registry */

class Source::source_registry {
public:
    source_registry() = default;
    ~source_registry();
    Source::object* operator[](Source::source_tag);
private:
    std::map<Source::source_tag, Source::object*> iregistry;
};

Source::source_registry* Source::source_registry_make() {
    return new Source::source_registry;
}

void Source::source_registry_delete(Source::source_registry *r) {
    delete r;
}

Source::object* Source::source_registry::operator[](Source::source_tag tag) {
    if (iregistry.contains(tag)) {
        cout << "Source::source_registry: found, returning" << endl;
        return iregistry[tag];
    }
    cout << "Source::source_registry: not found, creating & returning" << endl;
    Source::object *srcOjb = new Source::object(tag);
    iregistry[tag] = srcOjb;
    return srcOjb;
}

Source::source_registry::~source_registry() {
    for (auto&[tag, srcOjb] : iregistry) {
        cout << "Source::source_registry: deleting " << srcOjb << endl;
        delete srcOjb;
    }
    cout << "Source::source_registry: deleting self" << endl;
}

/* Source::source_handle */

Source::source_handle::source_handle(Source::source_tag tag, Source::source_registry &registry) {
    srcObj = registry[tag];
}

Source::source_handle::source_handle(const char *file, int line, const char *func, source_registry &reg)
    : source_handle(source_tag(file, line, func), reg) {}

Source::source_handle::source_handle(const char *file, int line, const char *func, source_registry *reg)
    : source_handle(source_tag(file, line, func), *reg) {}

Source::source_handle* Source::source_handle_make(const char* file, int line, const char* func, source_registry* registry) {
    return new Source::source_handle(file, line, func, registry);
}

void Source::source_handle_delete(Source::source_handle *h) {
    delete h;
}

uint64_t Source::source_handle::getID() {
    return srcObj->getID();
}

const char* Source::source_handle::get_file() {
    return this->srcObj->getFile();
}
const char* Source::source_handle::get_func() {
    return this->srcObj->getFunc();
}
int Source::source_handle::get_line() {
    return this->srcObj->getLine();
}

const char* Source::source_handle_get_file(source_handle* h) {
    return h->get_file();
}
const char* Source::source_handle_get_func(source_handle* h) {
    return h->get_func();
}
int Source::source_handle_get_line(source_handle* h) {
    return h->get_line();
}



ostream& Source::source_handle::stream(ostream& os) const {
    return os << srcObj;
}

ostream& operator<<(std::ostream& os, Source::source_handle const& h) {
    os << "Source::source_handle" << "@[" << static_cast<const void*>(&h) << "](";
    h.stream(os);
    os << ")";
    return os;
}

ostream& operator<<(ostream& os, Source::source_handle& h) {
    os << "Source::source_handle(";
    h.stream(os);
    os << ")";
    return os;
}
