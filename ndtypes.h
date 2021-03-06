/*
 * Copyright (c) 2016, Continuum Analytics, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * 
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef NDTYPES_H
#define NDTYPES_H


#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <complex.h>


#if SIZE_MAX > ULLONG_MAX
  #error "need SIZE_MAX <= ULLONG_MAX"
#endif

#ifdef _MSC_VER
  typedef _Dcomplex ndt_complex128_t;
  typedef _Fcomplex ndt_complex64_t;
  #define alignof __alignof
#else
  #if !defined(__APPLE__) && !defined(__STDC_IEC_559__)
    #error "ndtypes requires IEEE floating point arithmetic"
  #endif
  #include <stdalign.h>
  typedef double complex ndt_complex128_t;
  typedef float complex ndt_complex64_t;
#endif


/*****************************************************************************/
/*                                 Datashape                                 */
/*****************************************************************************/

/* Types: ndt_t */
typedef struct _ndt ndt_t;

/* Values: ndt_value_t */
typedef union {
  bool Bool;
  int8_t Int8;
  int16_t Int16;
  int32_t Int32;
  int64_t Int64;
  uint8_t Uint8;
  uint16_t Uint16;
  uint32_t Uint32;
  uint64_t Uint64;
  float Float32;
  double Float64;
  char *String;
} ndt_value_t;


/* Typed memory (could be extended to a memoryview) */
typedef struct {
    ndt_t *t;
    ndt_value_t v;
} ndt_memory_t;


/* Supported attribute types */
enum ndt_attr {
  AttrInt64,
  AttrString,
  AttrType
};

/* Attribute (name=value) */
typedef struct {
    char *name;
    enum ndt_attr tag;
    union {
      int64_t AttrInt64;
      char *AttrString;
      ndt_t *AttrType;
    };
} ndt_attr_t;


/* Flag for variadic tuples and records */
enum ndt_variadic_flag {
  Nonvariadic,
  Variadic
};

/* Encoding for characters and strings */
enum ndt_encoding {
  Ascii,
  Utf8,
  Utf16,
  Utf32,
  Ucs2,
  ErrorEncoding
};

/* Dimension kinds */
enum ndt_dim {
  FixedDimKind,
  FixedDim,
  VarDim,
  SymbolicDim,
  EllipsisDim
};

/* Datashape kinds */
enum ndt {
  /* Any */
  AnyKind,
    Array,

    Option,
    Nominal,
    Constr,

      /* Dtype */
      Tuple,
      Record,
      Function,
      Typevar,

      /* Scalar */
      ScalarKind,
        Void,
        Bool,

        SignedKind,
          Int8,
          Int16,
          Int32,
          Int64,

        UnsignedKind,
          Uint8,
          Uint16,
          Uint32,
          Uint64,

        RealKind,
          Float16,
          Float32,
          Float64,

        ComplexKind,
          Complex64,
          Complex128,

        FixedStringKind,
          FixedString,

        FixedBytesKind,
          FixedBytes,

        Char,
        String,
        Bytes,

        Categorical,
        Pointer,
};

enum ndt_alias {
  Size,
  Intptr,
  Uintptr
};

/* Tuple field */
typedef struct {
  ndt_t *type;
  size_t offset;
  uint8_t align;
  uint8_t pad;
} ndt_tuple_field_t;

/* Record field */
typedef struct {
  char *name;
  ndt_t *type;
  size_t offset;
  uint8_t align;
  uint8_t pad;
} ndt_record_field_t;

/* Dimension type */
typedef struct {
    enum ndt_dim tag;

    union {
        struct {
            size_t shape;
            size_t stride;
        } FixedDim;

        struct {
            size_t stride;
        } VarDim;

        struct {
            char *name;
        } SymbolicDim;
    };

    size_t itemsize;
    uint8_t itemalign;
    bool abstract;
} ndt_dim_t;

/* Datashape type */
struct _ndt {
    enum ndt tag;

    union {
        struct {
            size_t ndim;
            ndt_dim_t *dim;
            ndt_t *dtype;
            char order;
        } Array;

        struct {
            ndt_t *type;
        } Option;

        struct {
            char *name;
        } Nominal;

        struct {
            char *name;
            ndt_t *type;
        } Constr;

        struct {
            enum ndt_variadic_flag flag;
            size_t shape;
            ndt_tuple_field_t *fields;
        } Tuple;

        struct {
            enum ndt_variadic_flag flag;
            size_t shape;
            ndt_record_field_t *fields;
        } Record;

        struct {
            ndt_t *ret;
            ndt_t *pos;
            ndt_t *kwds;
        } Function;

        struct {
            char *name;
        } Typevar;

        struct {
            enum ndt_encoding encoding;
        } Char;

        struct {
            uint8_t target_align;
        } Bytes;

        struct {
            size_t size;
            enum ndt_encoding encoding;
        } FixedString;

        struct {
            size_t size;
            uint8_t align;
        } FixedBytes;

        struct {
            size_t ntypes;
            ndt_memory_t *types;
        } Categorical;

        struct {
            ndt_t *type;
        } Pointer;
    };

    size_t size;
    uint8_t align;
    bool abstract;
};




/*****************************************************************************/
/*                        Context and  error handling                        */
/*****************************************************************************/

enum ndt_error {
  NDT_Success,
  NDT_MemoryError,
  NDT_ValueError,
  NDT_TypeError,
  NDT_InvalidArgumentError,
  NDT_RuntimeError,
  NDT_NotImplementedError,
  NDT_LexError,
  NDT_ParseError,
  NDT_OSError
};

enum ndt_msg {
  ConstMsg,
  DynamicMsg
};

typedef struct {
    // ndt_t *ast;
    enum ndt_error err;
    enum ndt_msg msg;
    union {
        const char *ConstMsg;
        char *DynamicMsg;
    };
} ndt_context_t;

ndt_context_t *ndt_context_new(void);
void ndt_context_del(ndt_context_t *ctx);

void ndt_err_clear(ndt_context_t *ctx);
const char *ndt_context_msg(ndt_context_t *ctx);
const char *ndt_err_as_string(enum ndt_error err);
void ndt_err_format(ndt_context_t *ctx, enum ndt_error err, const char *fmt, ...);
void ndt_err_fprint(FILE *fp, ndt_context_t *ctx);


/*****************************************************************************/
/*                                Functions                                  */
/*****************************************************************************/

/*** Various ***/
char *ndt_strdup(const char *s, ndt_context_t *ctx);
char *ndt_asprintf(ndt_context_t *ctx, const char *fmt, ...);
const char *ndt_tag_as_string(enum ndt tag);
enum ndt_encoding ndt_encoding_from_string(char *s, ndt_context_t *ctx);
const char *ndt_encoding_as_string(enum ndt_encoding encoding);

int ndt_is_signed(const ndt_t *t);
int ndt_is_unsigned(const ndt_t *t);
int ndt_is_real(const ndt_t *t);
int ndt_is_complex(const ndt_t *t);
int ndt_is_scalar(const ndt_t *t);
int ndt_equal(const ndt_t *p, const ndt_t *c);
int ndt_match(const ndt_t *p, const ndt_t *c, ndt_context_t *ctx);


/*** String conversion ***/
bool ndt_strtobool(const char *v, ndt_context_t *ctx);
long ndt_strtol(const char *v, long min, long max, ndt_context_t *ctx);
long long ndt_strtoll(const char *v, long long min, long long max, ndt_context_t *ctx);
unsigned long ndt_strtoul(const char *v, unsigned long max, ndt_context_t *ctx);
unsigned long long ndt_strtoull(const char *v, unsigned long long max, ndt_context_t *ctx);
float ndt_strtof(const char *v, ndt_context_t *ctx);
double ndt_strtod(const char *v, ndt_context_t *ctx);


/*** Sequence elements ***/
void ndt_memory_del(ndt_memory_t *mem);
void ndt_memory_array_del(ndt_memory_t *types, size_t ntypes);

void ndt_attr_del(ndt_attr_t *attr);
void ndt_attr_array_del(ndt_attr_t *attr, size_t nattr);

ndt_tuple_field_t *ndt_tuple_field(ndt_t *type, uint8_t align, uint8_t pad, ndt_context_t *ctx);
void ndt_tuple_field_del(ndt_tuple_field_t *field);
void ndt_tuple_field_array_del(ndt_tuple_field_t *fields, size_t shape);

ndt_record_field_t *ndt_record_field(char *name, ndt_t *type, uint8_t align, uint8_t pad, ndt_context_t *ctx);
void ndt_record_field_del(ndt_record_field_t *field);
void ndt_record_field_array_del(ndt_record_field_t *fields, size_t shape);

ndt_dim_t *ndt_dim_new(enum ndt_dim tag, ndt_context_t *ctx);
void ndt_dim_del(ndt_dim_t *d);
void ndt_dim_array_del(ndt_dim_t *d, size_t shape);


/*** Dimensions ***/
ndt_dim_t *ndt_fixed_dim_kind(ndt_context_t *ctx);
ndt_dim_t *ndt_fixed_dim(size_t shape, int64_t stride, ndt_context_t *ctx);
ndt_dim_t *ndt_var_dim(int64_t stride, ndt_context_t *ctx);
ndt_dim_t *ndt_symbolic_dim(char *name, ndt_context_t *ctx);
ndt_dim_t *ndt_ellipsis_dim(ndt_context_t *ctx);


/*** Datashape ***/
ndt_t *ndt_new(enum ndt tag, ndt_context_t *ctx);
void ndt_del(ndt_t *t);

/* Typedef for nominal types */
int ndt_typedef(const char *name, ndt_t *type, ndt_context_t *ctx);

/* Any */
ndt_t *ndt_any_kind(ndt_context_t *ctx);
ndt_t *ndt_array(char order, ndt_dim_t *dim, size_t ndim, ndt_t *dtype, ndt_context_t *ctx);
ndt_t *ndt_option(ndt_t *type, ndt_context_t *ctx);
ndt_t *ndt_nominal(char *name, ndt_context_t *ctx);
ndt_t *ndt_constr(char *name, ndt_t *type, ndt_context_t *ctx);


/* Dtypes */
ndt_t *ndt_tuple(enum ndt_variadic_flag flag, ndt_tuple_field_t *fields, size_t shape,
                 ndt_context_t *ctx);
ndt_t *ndt_record(enum ndt_variadic_flag flag, ndt_record_field_t *fields, size_t shape,
                  ndt_context_t *ctx);
ndt_t *ndt_function(ndt_t *ret, ndt_t *pos, ndt_t *kwds, ndt_context_t *ctx);
ndt_t *ndt_typevar(char *name, ndt_context_t *ctx);


/* Scalar Kinds */
ndt_t *ndt_scalar_kind(ndt_context_t *ctx);
ndt_t *ndt_signed_kind(ndt_context_t *ctx);
ndt_t *ndt_unsigned_kind(ndt_context_t *ctx);
ndt_t *ndt_real_kind(ndt_context_t *ctx);
ndt_t *ndt_complex_kind(ndt_context_t *ctx);
ndt_t *ndt_fixed_string_kind(ndt_context_t *ctx);
ndt_t *ndt_fixed_bytes_kind(ndt_context_t *ctx);


/* Primitive Scalars */
ndt_t *ndt_primitive(enum ndt tag, ndt_context_t *ctx);
ndt_t *ndt_signed(int size, ndt_context_t *ctx);
ndt_t *ndt_unsigned(int size, ndt_context_t *ctx);
ndt_t *ndt_from_alias(enum ndt_alias tag, ndt_context_t *ctx);
ndt_t *ndt_char(enum ndt_encoding encoding, ndt_context_t *ctx);


/* Scalars */
ndt_t *ndt_string(ndt_context_t *ctx);
ndt_t *ndt_fixed_string(size_t size, enum ndt_encoding encoding, ndt_context_t *ctx);
ndt_t *ndt_bytes(uint8_t target_align, ndt_context_t *ctx);
ndt_t *ndt_fixed_bytes(size_t size, uint8_t align, ndt_context_t *ctx);
ndt_t *ndt_categorical(ndt_memory_t *types, size_t ntypes, ndt_context_t *ctx);
ndt_t *ndt_pointer(ndt_t *type, ndt_context_t *ctx);


/* Typed values */
ndt_memory_t *ndt_memory_from_number(char *v, ndt_t *t, ndt_context_t *ctx);
ndt_memory_t *ndt_memory_from_string(char *v, ndt_t *t, ndt_context_t *ctx);
int ndt_memory_equal(const ndt_memory_t *x, const ndt_memory_t *y);
int ndt_memory_compare(const ndt_memory_t *x, const ndt_memory_t *y);


/* Attributes */
ndt_attr_t *ndt_attr_from_number(char *name, char *v, ndt_context_t *ctx);
ndt_attr_t *ndt_attr_from_string(char *name, char *v, ndt_context_t *ctx);
ndt_attr_t *ndt_attr_from_type(char *name, ndt_t *t, ndt_context_t *ctx);


/******************************************************************************/
/*                                  Parsing                                   */
/******************************************************************************/

ndt_t *ndt_from_file(const char *name, ndt_context_t *ctx);
ndt_t *ndt_from_string(const char *input, ndt_context_t *ctx);


/******************************************************************************/
/*                       Initialization and tables                            */
/******************************************************************************/

int ndt_init(ndt_context_t *ctx);
void ndt_finalize(void);
int ndt_typedef_add(const char *name, const ndt_t *type, ndt_context_t *ctx);
const ndt_t *ndt_typedef_find(const char *name, ndt_context_t *ctx);


/******************************************************************************/
/*                                 Printing                                   */
/******************************************************************************/

char *ndt_as_string(ndt_t *t, ndt_context_t *ctx);
char *ndt_as_string_with_meta(ndt_t *t, ndt_context_t *ctx);
char *ndt_indent(ndt_t *t, ndt_context_t *ctx);


/******************************************************************************/
/*                            Memory handling                                 */
/******************************************************************************/

extern void *(* ndt_mallocfunc)(size_t size);
extern void *(* ndt_reallocfunc)(void *ptr, size_t size);
extern void (* ndt_free)(void *ptr);

void *ndt_alloc(size_t nmemb, size_t size);
void *ndt_realloc(void *ptr, size_t nmemb, size_t size);


/******************************************************************************/
/*                            Low level details                               */
/******************************************************************************/

/* Example of two possible low-level alternatives for the String type */
typedef struct {
    char *ptr;
    size_t size;
} ndt_sized_string_t;

typedef char * ndt_string_t;

typedef struct {
    char *ptr;
    size_t size;
} ndt_bytes_t;

typedef struct {
    char *ptr;
    size_t size;
} ndt_var_dim_t;


#endif /* NDTYPES_H */
