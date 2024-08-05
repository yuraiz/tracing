#ifndef CORESYMBOLICATION_CSTYPEREF
#define CORESYMBOLICATION_CSTYPEREF

/*
 * Types
 */
// Under the hood the framework basically just calls through to a set of C++
// libraries
struct sCSTypeRef {
    void* csCppData;  // typically retrieved using CSCppSymbol...::data(csData &
                      // 0xFFFFFFF8)
    void* csCppObj;   // a pointer to the actual CSCppObject
};
typedef struct sCSTypeRef CSTypeRef;

#endif  // __CORESYMBOLICATION_CSTYPEREF__
