#ifndef HELPERS_H
#define HELPERS_H

// this declares unsafe (in the type unsafe sense) functions which can be
// used as mappers and sorters for array maps. these are needed because casts
// between (for example) int (*)(char*, char*) and int (*)(void*, void*) are
// undefined in C. also, i would rather not turn the regular functions lke
// mat_destroy into unsafe functions.

// these functions implement the interface of ArraySorter, as defined in 
// array.h

// compares two char*'s, passed as void*'s
int ah_strcmp(void*, void*);
// compares the integer pointed to by two int*'s, passed as void*'s
int ah_intcmp(void*, void*);

// these functions implement ArrayMapper from array.h

// returns the name of the material, as a char* cast to void*
void* ah_mat_mapper(void*);
// returns the name of the connection, as char* cast to void*
void* ah_conn_mapper(void*);
// returns pointer to defer group key, as int* cast to void*
void* ah_dg_mapper(void*);

// the below functions implement an interface needed for array_foreach, used
// in cleanup functions.

// destroys the given material, pssed as Material* cast to void*
void ah_mat_destroy(void*);
// destroys the given connection, passed as Connection* cast to void*
void ah_conn_destroy(void*);
// destroys the given deger group, passed as DeferGroup* cast to void*
void ah_dg_destroy(void*);
// destroys the given message, passed as Message* cast to void*
void ah_msg_destroy(void*);
// destroys the given message from, passed as MessageFrom* cast to void*
void ah_msgfrom_destroy(void*);

#endif
