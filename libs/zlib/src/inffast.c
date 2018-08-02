/* inffast.c -- fast decoding
 * Copyright (C) 1995-2017 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#include "zbuild.h"
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"
#include "memcopy.h"

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    (hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } while (0)

#ifdef INFFAST_CHUNKSIZE
/*
   Ask the compiler to perform a wide, unaligned load with an machine
   instruction appropriate for the inffast_chunk_t type.
 */
static inline inffast_chunk_t loadchunk(unsigned char const* s) {
    inffast_chunk_t c;
    __builtin_memcpy(&c, s, sizeof(c));
    return c;
}

/*
   Ask the compiler to perform a wide, unaligned store with an machine
   instruction appropriate for the inffast_chunk_t type.
 */
static inline void storechunk(unsigned char* d, inffast_chunk_t c) {
    __builtin_memcpy(d, &c, sizeof(c));
}

/*
   Behave like memcpy, but assume that it's OK to overwrite at least
   INFFAST_CHUNKSIZE bytes of output even if the length is shorter than this,
   that the length is non-zero, and that `from` lags `out` by at least
   INFFAST_CHUNKSIZE bytes (or that they don't overlap at all or simply that
   the distance is less than the length of the copy).

   Aside from better memory bus utilisation, this means that short copies
   (INFFAST_CHUNKSIZE bytes or fewer) will fall straight through the loop
   without iteration, which will hopefully make the branch prediction more
   reliable.
 */
static inline unsigned char* chunkcopy(unsigned char *out, unsigned char const *from, unsigned len) {
    --len;
    storechunk(out, loadchunk(from));
    out += (len % INFFAST_CHUNKSIZE) + 1;
    from += (len % INFFAST_CHUNKSIZE) + 1;
    len /= INFFAST_CHUNKSIZE;
    while (len-- > 0) {
        storechunk(out, loadchunk(from));
        out += INFFAST_CHUNKSIZE;
        from += INFFAST_CHUNKSIZE;
    }
    return out;
}

/*
   Behave like chunkcopy, but avoid writing beyond of legal output.
 */
static inline unsigned char* chunkcopysafe(unsigned char *out, unsigned char const *from, unsigned len,
                                           unsigned char *safe) {
    if (out > safe) {
        while (len-- > 0) {
          *out++ = *from++;
        }
        return out;
    }
    return chunkcopy(out, from, len);
}

/*
   Perform short copies until distance can be rewritten as being at least
   INFFAST_CHUNKSIZE.

   This assumes that it's OK to overwrite at least the first
   2*INFFAST_CHUNKSIZE bytes of output even if the copy is shorter than this.
   This assumption holds because inflate_fast() starts every iteration with at
   least 258 bytes of output space available (258 being the maximum length
   output from a single token; see inflate_fast()'s assumptions below).
 */
static inline unsigned char* chunkunroll(unsigned char *out, unsigned *dist, unsigned *len) {
    unsigned char const *from = out - *dist;
    while (*dist < *len && *dist < INFFAST_CHUNKSIZE) {
        storechunk(out, loadchunk(from));
        out += *dist;
        *len -= *dist;
        *dist += *dist;
    }
    return out;
}
#endif

/*
   Decode literal, length, and distance codes and write out the resulting
   literal and match bytes until either not enough input or output is
   available, an end-of-block is encountered, or a data error is encountered.
   When large enough input and output buffers are supplied to inflate(), for
   example, a 16K input buffer and a 64K output buffer, more than 95% of the
   inflate execution time is spent in this routine.

   Entry assumptions:

        state->mode == LEN
        strm->avail_in >= 6
        strm->avail_out >= 258
        start >= strm->avail_out
        state->bits < 8

   On return, state->mode is one of:

        LEN -- ran out of enough output space or enough available input
        TYPE -- reached end of block code, inflate() to interpret next block
        BAD -- error in block data

   Notes:

    - The maximum input bits used by a length/distance pair is 15 bits for the
      length code, 5 bits for the length extra, 15 bits for the distance code,
      and 13 bits for the distance extra.  This totals 48 bits, or six bytes.
      Therefore if strm->avail_in >= 6, then there is enough input to avoid
      checking for available input while decoding.

    - The maximum bytes that a single length/distance pair can output is 258
      bytes, which is the maximum length that can be coded.  inflate_fast()
      requires strm->avail_out >= 258 for each loop to avoid checking for
      output space.
 */
void ZLIB_INTERNAL inflate_fast(PREFIX3(stream) *strm, unsigned long start) {
    /* start: inflate()'s starting value for strm->avail_out */
    struct inflate_state *state;
    const unsigned char *in;    /* local strm->next_in */
    const unsigned char *last;  /* have enough input while in < last */
    unsigned char *out;         /* local strm->next_out */
    unsigned char *beg;         /* inflate()'s initial strm->next_out */
    unsigned char *end;         /* while out < end, enough space available */
#ifdef INFFAST_CHUNKSIZE
    unsigned char *safe;        /* can use chunkcopy provided out < safe */
#endif
#ifdef INFLATE_STRICT
    unsigned dmax;              /* maximum distance from zlib header */
#endif
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned wnext;             /* window write index */
    unsigned char *window;      /* allocated sliding window, if wsize != 0 */
    uint32_t hold;              /* local strm->hold */
    unsigned bits;              /* local strm->bits */
    code const *lcode;          /* local strm->lencode */
    code const *dcode;          /* local strm->distcode */
    unsigned lmask;             /* mask for first level of length codes */
    unsigned dmask;             /* mask for first level of distance codes */
    code here;                  /* retrieved table entry */
    unsigned op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    unsigned len;               /* match length, unused bytes */
    unsigned dist;              /* match distance */
    unsigned char *from;        /* where to copy match from */

    /* copy state to local variables */
    state = (struct inflate_state *)strm->state;
    in = strm->next_in;
    last = in + (strm->avail_in - 5);
    out = strm->next_out;
    beg = out - (start - strm->avail_out);
    end = out + (strm->avail_out - 257);

#ifdef INFFAST_CHUNKSIZE
    safe = out + (strm->avail_out - INFFAST_CHUNKSIZE);
#endif
#ifdef INFLATE_STRICT
    dmax = state->dmax;
#endif
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    window = state->window;
    hold = state->hold;
    bits = state->bits;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    do {
        if (bits < 15) {
            hold += load_short(in, bits);
            in += 2;
            bits += 16;
        }
        here = lcode[hold & lmask];
      dolen:
        DROPBITS(here.bits);
        op = here.op;
        if (op == 0) {                          /* literal */
            Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", here.val));
            *out++ = (unsigned char)(here.val);
        } else if (op & 16) {                     /* length base */
            len = here.val;
            op &= 15;                           /* number of extra bits */
            if (op) {
                if (bits < op) {
                    hold += (uint32_t)(*in++) << bits;
                    bits += 8;
                }
                len += BITS(op);
                DROPBITS(op);
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            if (bits < 15) {
                hold += load_short(in, bits);
                in += 2;
                bits += 16;
            }
            here = dcode[hold & dmask];
          dodist:
            DROPBITS(here.bits);
            op = here.op;
            if (op & 16) {                      /* distance base */
                dist = here.val;
                op &= 15;                       /* number of extra bits */
                if (bits < op) {
                    hold += (uint32_t)(*in++) << bits;
                    bits += 8;
                    if (bits < op) {
                        hold += (uint32_t)(*in++) << bits;
                        bits += 8;
                    }
                }
                dist += BITS(op);
#ifdef INFLATE_STRICT
                if (dist > dmax) {
                    strm->msg = (char *)"invalid distance too far back";
                    state->mode = BAD;
                    break;
                }
#endif
                DROPBITS(op);
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (unsigned)(out - beg);     /* max distance in output */
                if (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    if (op > whave) {
                        if (state->sane) {
                            strm->msg = (char *)"invalid distance too far back";
                            state->mode = BAD;
                            break;
                        }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        if (len <= op - whave) {
                            do {
                                *out++ = 0;
                            } while (--len);
                            continue;
                        }
                        len -= op - whave;
                        do {
                            *out++ = 0;
                        } while (--op > whave);
                        if (op == 0) {
                            from = out - dist;
                            do {
                                *out++ = *from++;
                            } while (--len);
                            continue;
                        }
#endif
                    }
#ifdef INFFAST_CHUNKSIZE
                    from = window;
                    if (wnext == 0) {           /* very common case */
                        from += wsize - op;
                    } else if (wnext >= op) {   /* contiguous in window */
                        from += wnext - op;
                    } else {                    /* wrap around window */
                        op -= wnext;
                        from += wsize - op;
                        if (op < len) {         /* some from end of window */
                            len -= op;
                            out = chunkcopysafe(out, from, op, safe);
                            from = window;      /* more from start of window */
                            op = wnext;
                            /* This (rare) case can create a situation where
                               the first chunkcopy below must be checked.
                             */
                        }
                    }
                    if (op < len) {             /* still need some from output */
                        len -= op;
                        out = chunkcopysafe(out, from, op, safe);
                        if (dist == 1) {
                            out = byte_memset(out, len);
                        } else {
                            out = chunkunroll(out, &dist, &len);
                            out = chunkcopysafe(out, out - dist, len, safe);
                        }
                    } else {
                        if (from - out == 1) {
                            out = byte_memset(out, len);
                        } else {
                            out = chunkcopysafe(out, from, len, safe);
                        }
                    }
#else
                    from = window;
                    if (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    } else if (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        if (op < len) {         /* some from end of window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = window;
                            if (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                do {
                                    *out++ = *from++;
                                } while (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    } else {                      /* contiguous in window */
                        from += wnext - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }

                    out = chunk_copy(out, from, (int) (out - from), len);
#endif
                } else {
#ifdef INFFAST_CHUNKSIZE
                    if (dist == 1 && len >= sizeof(uint64_t)) {
                        out = byte_memset(out, len);
                    } else {
                        /* Whole reference is in range of current output.  No
                           range checks are necessary because we start with room
                           for at least 258 bytes of output, so unroll and roundoff
                           operations can write beyond `out+len` so long as they
                           stay within 258 bytes of `out`.
                         */
                        out = chunkunroll(out, &dist, &len);
                        out = chunkcopy(out, out - dist, len);
                    }
#else
                    if (len < sizeof(uint64_t))
                      out = set_bytes(out, out - dist, dist, len);
                    else if (dist == 1)
                      out = byte_memset(out, len);
                    else
                      out = chunk_memset(out, out - dist, dist, len);
#endif
                }
            } else if ((op & 64) == 0) {          /* 2nd level distance code */
                here = dcode[here.val + BITS(op)];
                goto dodist;
            } else {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
        } else if ((op & 64) == 0) {              /* 2nd level length code */
            here = lcode[here.val + BITS(op)];
            goto dolen;
        } else if (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->mode = TYPE;
            break;
        } else {
            strm->msg = (char *)"invalid literal/length code";
            state->mode = BAD;
            break;
        }
    } while (in < last && out < end);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    /* update state and return */
    strm->next_in = in;
    strm->next_out = out;
    strm->avail_in = (unsigned)(in < last ? 5 + (last - in) : 5 - (in - last));
    strm->avail_out = (unsigned)(out < end ? 257 + (end - out) : 257 - (out - end));
    state->hold = hold;
    state->bits = bits;
    return;
}

/*
   inflate_fast() speedups that turned out slower (on a PowerPC G3 750CXe):
   - Using bit fields for code structure
   - Different op definition to avoid & for extra bits (do & for table bits)
   - Three separate decoding do-loops for direct, window, and wnext == 0
   - Special case for distance > 1 copies to do overlapped load and store copy
   - Explicit branch predictions (based on measured branch probabilities)
   - Deferring match copy and interspersed it with decoding subsequent codes
   - Swapping literal/length else
   - Swapping window/direct else
   - Larger unrolled copy loops (three is about right)
   - Moving len -= 3 statement into middle of loop
 */
