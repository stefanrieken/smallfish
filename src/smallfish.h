#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// In this particular implementation we lean heavily on malloc
// That is not to say that we can't re-write this header for
// a custom allocator instead.
#define allocate(type, n) malloc(sizeof(type) * n)
#define reallocate(o, type, n) realloc(o, sizeof(type) * n)

// Our own word size
#define WORD uint32_t

// Index #0 == nil; but tagged == 1(!!!)
#define nil 1
// These are the classics, but I guess...
#define idx(obj)  ((obj)-objects)
#define ptr(idx)  (&objects[idx])
// ...these may get more use?
#define is_obj(val) (val & 1)
#define is_int(val) (!is_obj(val))
#define tag_int(val) ((val) << 1)
#define tag_obj(obj) ((idx(obj) << 1) | 1)
#define as_obj(tagged) ptr(tagged >> 1)
#define as_int(tagged) (tagged >> 1)

typedef struct DictEntry {
    WORD name;
    WORD value;
} Dictionary, DictEntry;

// Object table (Entry)
// All allocated objects are stored in the object table,
// and referenced using indirect pointers, so that:
// - we retain a pointer to orphaned objects
// - we can centrally change pointers after realloc
// - we can offload the responsiblity for basic metadata, like size
//
// The allocation status of the object table itself is stored in entry #0 (TODO not the best way).
typedef struct ObjectTableEntry {
    // Putting value first because I imagine it makes for the simplest dereferencing
    union {
        void * ptr; // don't know, don't care
        char * str;
        WORD * ws; // word size (array) value
        struct ObjectTableEntry * obj;
        DictEntry * dict;
        int count; // only for entry #0!
    } value;

    // On 64-bit systems, this struct totals to 16 bytes (2 x 64 bits)
    // On 32-bit systems, this struct totals to 12 bytes (3 x 32 bits)
    uint16_t size;
    uint8_t refcount; // Could be only 1 'gc_mark' bit; but refcount MAY get use when actively cleaning up objects like stack frames
    bool compound; // Whether or not the object is a compound type (i.e., a Dictionary) independent of class
    uint32_t type; // object class; NOTE this assumes WORD <= 32 bits

} ObjectTable, Object;

#define GC_FREE UINT8_MAX

//
// Core type support
//

// Register for the built-in, physical storage types.
typedef struct CoreType {
    Object * type;
    // Apply a method type to its arguments
    // Since we can't derive CoreType from runtime object, this registry has little use over directly calling known
    // apply / eval functions. But TODO MAYBE we want to create a small array for all 'applicable' types instead.
    WORD (*apply)(WORD msg, WORD obj, Object * args, Object * ctx);
    bool (*parse)(int * ch, WORD * result);
} CoreType;

extern int num_core_types;
extern CoreType ** core_types;

extern WORD STR_EVAL;
extern WORD STR_PRINT;
extern WORD STR_MARK;
extern WORD STR_ENV;

// 1-arg message (= 1-arg apply)
WORD message1(WORD obj, WORD name, Object * ctx);
WORD eval(WORD val, Object * ctx);

void print_val(WORD val, Object * ctx);

// We keep just one object table (so not e.g. group all string literals),
// because I want to keep the indices into this table simple.
extern ObjectTable * objects;

//extern Object * root;

Object * add_object(ObjectTable ** table, void * value, Object * type, int size);
