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


#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "ndtypes.h"
#include "seq.h"


/*****************************************************************************/
/*                        Functions used in the lexer                        */
/*****************************************************************************/

char *
mk_stringlit(const char *lexeme, ndt_context_t *ctx)
{
    size_t len;
    char *s, *result;

    len = strlen(lexeme);
    if (len < 2) {
        ndt_err_format(ctx, NDT_RuntimeError, "invalid string literal");
        return NULL;
    }

    s = ndt_strdup(lexeme, ctx);
    if (s == NULL) {
        return NULL;
    }

    s[len-1] = '\0';
    result = ndt_asprintf(ctx, "%s", s+1);

    ndt_free(s);
    return result;
}


/*****************************************************************************/
/*                        Functions used in the parser                       */
/*****************************************************************************/

ndt_dim_t *
mk_fixed_dim(char *v, ndt_attr_seq_t *seq, ndt_context_t *ctx)
{
    size_t shape;
    int64_t stride = INT64_MAX;

    if (seq) {
        seq = ndt_attr_seq_finalize(seq);
    }

    shape = (size_t)ndt_strtoull(v, SIZE_MAX, ctx);
    ndt_free(v);
    if (ctx->err != NDT_Success) {
        return NULL;
    }

    if (seq) {
        if (seq->len != 1 || strcmp(seq->ptr[0].name, "stride") != 0) {
            ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid keyword");
            ndt_attr_array_del(seq->ptr, seq->len);
            ndt_free(seq);
            return NULL;
        }
        stride = seq->ptr[0].AttrInt64;
        ndt_attr_array_del(seq->ptr, seq->len);
        ndt_free(seq);
    }

    return ndt_fixed_dim(shape, stride, ctx);
}
 
ndt_dim_t *
mk_var_dim(ndt_attr_seq_t *seq, ndt_context_t *ctx)
{
    int64_t stride = INT64_MAX;

    if (seq) {
        seq = ndt_attr_seq_finalize(seq);

        if (seq->len != 1 || strcmp(seq->ptr[0].name, "stride") != 0) {
            ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid keyword");
            ndt_attr_array_del(seq->ptr, seq->len);
            ndt_free(seq);
            return NULL;
        }
        stride = seq->ptr[0].AttrInt64;
        ndt_attr_array_del(seq->ptr, seq->len);
        ndt_free(seq);
    }

    return ndt_var_dim(stride, ctx);
}

ndt_t *
mk_fixed_string(char *v, enum ndt_encoding encoding, ndt_context_t *ctx)
{
    size_t size;

    size = (size_t)ndt_strtoull(v, SIZE_MAX, ctx);
    if (ctx->err != NDT_Success) {
        ndt_free(v);
        return NULL;
    }

    ndt_free(v);
    return ndt_fixed_string(size, encoding, ctx);
}
 
ndt_t *
mk_bytes(ndt_attr_seq_t *seq, ndt_context_t *ctx)
{
    uint8_t align;

    seq = ndt_attr_seq_finalize(seq);

    if (seq->len != 1) {
        goto error;
    }

    if (strcmp(seq->ptr[0].name, "align") != 0) {
        goto error;
    }

    if (seq->ptr[0].AttrInt64 < 0 || UINT8_MAX < seq->ptr[0].AttrInt64) {
        goto error;
    }

    align = seq->ptr[0].AttrInt64; /* XXX: overflow */

    ndt_attr_array_del(seq->ptr, seq->len);
    ndt_free(seq);

    return ndt_bytes(align, ctx);

error:
    ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid or repeated keyword");
    ndt_attr_array_del(seq->ptr, seq->len);
    ndt_free(seq);
    return NULL;
}
 
ndt_t *
mk_fixed_bytes(ndt_attr_seq_t *seq, ndt_context_t *ctx)
{
    size_t sz;
    uint8_t target_align = 1;
    int have_size = 0;
    int have_target_align = 0;
    size_t i;

    seq = ndt_attr_seq_finalize(seq);

    for (i = 0; i < seq->len; i++) {
        if (strcmp(seq->ptr[i].name, "size") == 0) {
            if (have_size) {
                goto error;
            }
            sz = seq->ptr[i].AttrInt64; /* XXX: overflow */
            have_size = 1;
        }
        else if (strcmp(seq->ptr[i].name, "align") == 0) {
            if (have_target_align || seq->ptr[i].AttrInt64 < 0 || UINT8_MAX < seq->ptr[i].AttrInt64) {
                goto error;
            }
            target_align = seq->ptr[i].AttrInt64;
            have_target_align = 1;
        }
        else {
            goto error;
        }
    }

    if (!have_size) {
        goto error;
    }

    ndt_attr_array_del(seq->ptr, seq->len);
    ndt_free(seq);

    return ndt_fixed_bytes(sz, target_align, ctx);

error:
    ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid or repeated keyword");
    ndt_attr_array_del(seq->ptr, seq->len);
    ndt_free(seq);
    return NULL;
}

ndt_t *
mk_array(ndt_dim_seq_t *dims, ndt_t *dtype, ndt_attr_seq_t *attrs, ndt_context_t *ctx)
{
    ndt_t *t;
    char ord = 'C';

    dims = ndt_dim_seq_finalize(dims);

    if (attrs) {
        attrs = ndt_attr_seq_finalize(attrs);

        if (attrs->len != 1 || strcmp(attrs->ptr[0].name, "order") != 0) {
            goto error;
        }

        if (strcmp(attrs->ptr[0].AttrString, "C") == 0) {
            ;
        }
        else if (strcmp(attrs->ptr[0].AttrString, "F") == 0) {
            ord = 'F';
        }
        else {
            goto error;
        }

        ndt_attr_array_del(attrs->ptr, attrs->len);
        ndt_free(attrs);
        goto out;

    error:
        ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid keyword");
        ndt_dim_array_del(dims->ptr, dims->len);
        ndt_free(dims);
        ndt_del(dtype);
        ndt_attr_array_del(attrs->ptr, attrs->len);
        ndt_free(attrs);
        return NULL;
    }

out:
    t = ndt_array(ord, dims->ptr, dims->len, dtype, ctx);
    ndt_free(dims);
    return t;
}
 
ndt_tuple_field_t *
mk_tuple_field(ndt_t *type, ndt_attr_seq_t *seq, ndt_context_t *ctx)
{
    uint8_t pad = 0;
    uint8_t align = 0;
    int have_pad = 0;
    int have_align = 0;
    size_t i;

    if (seq) {
        seq = ndt_attr_seq_finalize(seq);

        if (seq->len > 2) {
            goto error;
        }

        for (i = 0; i < seq->len; i++) {
            if (strcmp(seq->ptr[0].name, "pad") == 0) {
                if (have_pad) {
                    goto error;
                }
                pad = seq->ptr[0].AttrInt64; /* XXX: overflow */
                have_pad = 1;
            }
            else if (strcmp(seq->ptr[0].name, "align") == 0) {
                if (have_align) {
                    goto error;
                }
                align = seq->ptr[0].AttrInt64; /* XXX: overflow */
                have_align = 1;
            }
            else {
                goto error;
            }
        }

        ndt_attr_array_del(seq->ptr, seq->len);
        ndt_free(seq);
        goto out;

    error:
        ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid or repeated keyword");
        ndt_del(type);
        ndt_attr_array_del(seq->ptr, seq->len);
        ndt_free(seq);
        return NULL;
    }

out:
    return ndt_tuple_field(type, align, pad, ctx);
}

ndt_t *
mk_tuple(enum ndt_variadic_flag flag, ndt_tuple_field_seq_t *seq, ndt_context_t *ctx)
{
    ndt_t *t;

    if (seq == NULL) {
        return ndt_tuple(flag, NULL, 0, ctx);
    }

    seq = ndt_tuple_field_seq_finalize(seq);
    t = ndt_tuple(flag, seq->ptr, seq->len, ctx);

    ndt_free(seq);
    return t;
}

ndt_record_field_t *
mk_record_field(char *name, ndt_t *type, ndt_attr_seq_t *seq, ndt_context_t *ctx)
{
    uint8_t pad = 0;
    uint8_t align = 0;
    int have_pad = 0;
    int have_align = 0;
    size_t i;

    if (seq) {
        seq = ndt_attr_seq_finalize(seq);

        if (seq->len > 2) {
            goto error;
        }

        for (i = 0; i < seq->len; i++) {
            if (strcmp(seq->ptr[0].name, "pad") == 0) {
                if (have_pad) {
                    goto error;
                }
                pad = seq->ptr[0].AttrInt64; /* XXX: overflow */
                have_pad = 1;
            }
            else if (strcmp(seq->ptr[0].name, "align") == 0) {
                if (have_align) {
                    goto error;
                }
                align = seq->ptr[0].AttrInt64; /* XXX: overflow */
                have_align = 1;
            }
            else {
                goto error;
            }
        }

        ndt_attr_array_del(seq->ptr, seq->len);
        ndt_free(seq);
        goto out;

    error:
        ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid or repeated keyword");
        ndt_free(name);
        ndt_del(type);
        ndt_attr_array_del(seq->ptr, seq->len);
        ndt_free(seq);
        return NULL;
    }

out:
    return ndt_record_field(name, type, align, pad, ctx);
}


ndt_t *
mk_record(enum ndt_variadic_flag flag, ndt_record_field_seq_t *seq, ndt_context_t *ctx)
{
    ndt_t *t;

    if (seq == NULL) {
        return ndt_record(flag, NULL, 0, ctx);
    }

    seq = ndt_record_field_seq_finalize(seq);
    t = ndt_record(flag, seq->ptr, seq->len, ctx);

    ndt_free(seq);
    return t;
}
 
ndt_t *
mk_function(ndt_t *ret,
            enum ndt_variadic_flag tflag, ndt_tuple_field_seq_t *tseq,
            enum ndt_variadic_flag rflag, ndt_record_field_seq_t *rseq,
            ndt_context_t *ctx)
{
    ndt_t *pos = NULL;
    ndt_t *kwds = NULL;

    pos = mk_tuple(tflag, tseq, ctx);
    if (pos == NULL) {
        ndt_del(ret);
        ndt_record_field_seq_del(rseq);
        return NULL;
    }

    kwds = mk_record(rflag, rseq, ctx);
    if (kwds == NULL) {
        ndt_del(ret);
        ndt_del(pos);
        return NULL;
    }

    return ndt_function(ret, pos, kwds, ctx);
}

ndt_t *
mk_function_from_tuple(ndt_t *ret, ndt_t *pos, ndt_context_t *ctx)
{
    ndt_t *kwds = NULL;

    kwds = ndt_record(Nonvariadic, NULL, 0, ctx);
    if (kwds == NULL) {
        ndt_del(ret);
        ndt_del(pos);
        return NULL;
    }

    return ndt_function(ret, pos, kwds, ctx);
}

ndt_t *
mk_categorical(ndt_memory_seq_t *seq, ndt_context_t *ctx)
{
    ndt_t *t;

    seq = ndt_memory_seq_finalize(seq);
    t = ndt_categorical(seq->ptr, seq->len, ctx);

    ndt_free(seq);
    return t;
}
