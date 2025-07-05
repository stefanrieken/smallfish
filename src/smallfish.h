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

// Helpers to make nicely aligned structs
#if UINTPTR_MAX == UINT64_MAX

#define HALFPTR uint32_t
#define QUARTPTR uint16_t
#define MAX_QUART UINT16_MAX
#define MSB (1 << 63)

#else

#define HALFPTR uint16_t
#define QUARTPTR uint8_t
#define QUARTPTR int8_t
#define MAX_QUART UINT8_MAX
#define MSB (1 << 31)

#endif

#define GC_FREE MAX_QUART

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

// 'lazy' evaluation actually makes the individual primitives do
// more active work by evaluating their own argument expressions.
// With 'eager' evaluation, the interpreter will pre-eval all
// expressions except for 'special' arguments (= the block
// argumnents to 'if', etc.
#ifdef LAZY
#define e(val,ctx) (eval(val,ctx))
#else
#define e(val,ctx) val
#endif

// LSB == indicator bit; so

// I'm NOT a fan of taking bits off perfectly good integers,
// but ultimately this is the more flexible method of in-lining
// integer values.


typedef struct DictEntry {
    WORD name;// : 31;
//    bool is_ptr : 1;
    WORD value;
} Dictionary, DictEntry;

// Object table (Entry)
// All allocated objects are stored in the object table,
// and referenced using indirect pointers, so that:x	
// - we retain a pointer to orphaned objects
// - we can centrally change pointers after realloc
// - we can use this to track the object's size
//
// The allocation status of the object table itself is stored in entry #0 .

typedef struct ObjectTableEntry {
    // Putting value first because I imagine it makes for the simplest dereferencing
    union {
        void * ptr; // don't know, don't care
        uint8_t * str;
        WORD * ws; // word size (array) value
        DictEntry * dict;        
        int count; // only for entry #0!
    } value;

    HALFPTR size;
    QUARTPTR refcount; // or GC mark, or whatever
    QUARTPTR type; // struct CoreType; TODO replace with:
    HALFPTR type1; // object class

} ObjectTable, Object;

//
// Core type support
//

WORD eval_to_self(WORD val, Object * ctx);
// E.g. for class values to return themselves when called as method
WORD apply_to_self(WORD msg, WORD obj, Object * args, Object * ctx);

// Register for the built-in, physical storage types.
typedef struct CoreType {
    Object * type;
    WORD (*apply)(WORD msg, WORD obj, Object * args, Object * ctx); // in use to run method types
    bool (*parse)(int * ch, WORD * result);
} CoreType;

extern int num_core_types;
extern CoreType ** core_types;

extern WORD STR_EVAL;
extern WORD STR_PRINT;
extern WORD STR_MARK;

// 1-arg message (= 1-arg apply)
WORD message1(WORD obj, WORD name, Object * ctx);
WORD eval(WORD val, Object * ctx);

void print_val(WORD val, Object * ctx);

// We keep just one object table (so not e.g. group all string literals),
// because I want to keep the indices into this table simple.
extern ObjectTable * objects;

//extern Object * root;

Object * add_object(ObjectTable ** table, void * value, int type, Object * type1, int size);
