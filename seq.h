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


#ifndef SEQ_H
#define SEQ_H

#include "ndtypes.h"


/* Strongly typed sequences (used in the parser). */

typedef struct {
    size_t len;
    size_t reserved;
    ndt_dim_t *ptr;
} ndt_dim_seq_t;

typedef struct {
    size_t len;
    size_t reserved;
    ndt_tuple_field_t *ptr;
} ndt_tuple_field_seq_t;

typedef struct {
    size_t len;
    size_t reserved;
    ndt_record_field_t *ptr;
} ndt_record_field_seq_t;

typedef struct {
    size_t len;
    size_t reserved;
    ndt_memory_t *ptr;
} ndt_memory_seq_t;

typedef struct {
    size_t len;
    size_t reserved;
    ndt_attr_t *ptr;
} ndt_attr_seq_t;


ndt_dim_seq_t *ndt_dim_seq_new(ndt_dim_t *, ndt_context_t *ctx);
void ndt_dim_seq_del(ndt_dim_seq_t *);
ndt_dim_seq_t *ndt_dim_seq_append(ndt_dim_seq_t *, ndt_dim_t *, ndt_context_t *ctx);
ndt_dim_seq_t *ndt_dim_seq_finalize(ndt_dim_seq_t *);

ndt_tuple_field_seq_t *ndt_tuple_field_seq_new(ndt_tuple_field_t *, ndt_context_t *ctx);
void ndt_tuple_field_seq_del(ndt_tuple_field_seq_t *);
ndt_tuple_field_seq_t *ndt_tuple_field_seq_append(ndt_tuple_field_seq_t *, ndt_tuple_field_t *, ndt_context_t *ctx);
ndt_tuple_field_seq_t *ndt_tuple_field_seq_finalize(ndt_tuple_field_seq_t *);

ndt_record_field_seq_t *ndt_record_field_seq_new(ndt_record_field_t *, ndt_context_t *ctx);
void ndt_record_field_seq_del(ndt_record_field_seq_t *);
ndt_record_field_seq_t *ndt_record_field_seq_append(ndt_record_field_seq_t *, ndt_record_field_t *, ndt_context_t *ctx);
ndt_record_field_seq_t *ndt_record_field_seq_finalize(ndt_record_field_seq_t *);

ndt_memory_seq_t *ndt_memory_seq_new(ndt_memory_t *, ndt_context_t *ctx);
void ndt_memory_seq_del(ndt_memory_seq_t *);
ndt_memory_seq_t *ndt_memory_seq_append(ndt_memory_seq_t *, ndt_memory_t *, ndt_context_t *ctx);
ndt_memory_seq_t *ndt_memory_seq_finalize(ndt_memory_seq_t *);

ndt_attr_seq_t *ndt_attr_seq_new(ndt_attr_t *, ndt_context_t *ctx);
void ndt_attr_seq_del(ndt_attr_seq_t *);
ndt_attr_seq_t *ndt_attr_seq_append(ndt_attr_seq_t *, ndt_attr_t *, ndt_context_t *ctx);
ndt_attr_seq_t *ndt_attr_seq_finalize(ndt_attr_seq_t *);


#endif /* SEQ_H */
