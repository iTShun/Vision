#pragma once

#include "GenericKit/General.h"

namespace GenericKit
{
    class MemPool
    {
    protected:
        enum
        {
            BLOCK_ALIGN     = 2 * sizeof(void*),
            MAX_FLI         = 30,
            MAX_LOG2_SLI    = 5,
            MAX_SLI         = 1 << MAX_LOG2_SLI,
            FLI_OFFSET      = 6,
            SMALL_BLOCK     = 128,
            REAL_FLI        = MAX_FLI - FLI_OFFSET,
            PTR_MASK        = sizeof(void *) - 1,
            BLOCK_SIZE      = 0xFFFFFFFF - PTR_MASK,
            
            BLOCK_STATE     = 1 << 0,
            PREV_STATE      = 1 << 1,
            
            FREE_BLOCK      = 1 << 0,
            USED_BLOCK      = 0,
            PREV_FREE       = 1 << 1,
            PREV_USED       = 0,
        };
        
        struct bhdr_t;
        
        struct free_ptr_t
        {
            struct bhdr_t *prev;
            struct bhdr_t *next;
        };
        
        struct bhdr_t
        {
            /* This pointer is just valid if the first bit of size is set */
            struct bhdr_t *prev_hdr;
            /* The size is stored in bytes */
            size_t size;                /* bit 0 indicates whether the block is used and */
            /* bit 1 allows to know whether the previous block is free */
            union {
                struct free_ptr_t free_ptr;
                unsigned char buffer[1];         /*sizeof(struct free_ptr_struct)]; */
            } ptr;
        };
        
        struct area_info_t
        {
            bhdr_t *end;
            struct area_info_t *next;
        };
        
        enum
        {
            MIN_BLOCK_SIZE      = sizeof(free_ptr_t),
            BHDR_OVERHEAD       = sizeof (bhdr_t) - MIN_BLOCK_SIZE,
        };
        
    protected:
        void* mem_pool;
        size_t total_size;
        size_t available_size;
        size_t used_size;
        
        /* A linked list holding all the existing areas */
        area_info_t *area_head;
        
        /* the first-level bitmap */
        /* This array should have a size of REAL_FLI bits */
        unsigned int fl_bitmap;
        
        /* the second-level bitmap */
        unsigned int sl_bitmap[REAL_FLI];
        
        bhdr_t *matrix[REAL_FLI][MAX_SLI];
        
        static const int table[256];
    public:
        MemPool(size_t size);
        MemPool(const MemPool&) = default;
        ~MemPool();
        
        void* alloc(size_t size);
        
        void* realloc(void* ptr, size_t new_size);
        
        void free(void* ptr);
        
        void print_all_blocks();
    protected:
        void operator=(const MemPool&);
        
        bhdr_t *process_area(void *area, size_t size);
        
        void print_block(bhdr_t *b);
        
    protected:
        inline size_t align_up(size_t bytes, size_t align = BLOCK_ALIGN)
        {
            assert(0 == (align & (align - 1)) && "must align to a power of two");
            return (bytes + (align - 1)) & ~(align - 1);
        }
        
        inline size_t align_down(size_t bytes, size_t align = BLOCK_ALIGN)
        {
            assert(0 == (align & (align - 1)) && "must align to a power of two");
            return bytes - (bytes & (align - 1));
        }
        
        inline int ls_bit(int i)
        {
            unsigned int a;
            unsigned int x = i & -i;
            
            a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
            return table[x >> a] + a;
        }
        
        inline int ms_bit(int i)
        {
            unsigned int a;
            unsigned int x = (unsigned int) i;
            
            a = x <= 0xffff ? (x <= 0xff ? 0 : 8) : (x <= 0xffffff ? 16 : 24);
            return table[x >> a] + a;
        }
        
        inline void set_bit(int nr, unsigned int * addr)
        {
            addr[nr >> 5] |= 1 << (nr & 0x1f);
        }
        
        inline void clear_bit(int nr, unsigned int * addr)
        {
            addr[nr >> 5] &= ~(1 << (nr & 0x1f));
        }
        
        inline bhdr_t* get_next_block(const void* ptr, size_t size)
        {
            return (bhdr_t*)((char*)ptr + size);
        }
        
        inline void mapping_search(size_t * _r, int *_fl, int *_sl)
        {
            int _t;
            
            if (*_r < SMALL_BLOCK) {
                *_fl = 0;
                *_sl = *_r / (SMALL_BLOCK / MAX_SLI);
            } else {
                _t = (1 << (ms_bit(*_r) - MAX_LOG2_SLI)) - 1;
                *_r = *_r + _t;
                *_fl = ms_bit(*_r);
                *_sl = (*_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
                *_fl -= FLI_OFFSET;
                /*if ((*_fl -= FLI_OFFSET) < 0) // FL wil be always >0!
                 *_fl = *_sl = 0;
                 */
                *_r &= ~_t;
            }
        }
        
        inline void mapping_insert(size_t _r, int *_fl, int *_sl)
        {
            if (_r < SMALL_BLOCK) {
                *_fl = 0;
                *_sl = _r / (SMALL_BLOCK / MAX_SLI);
            } else {
                *_fl = ms_bit(_r);
                *_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
                *_fl -= FLI_OFFSET;
            }
        }
        
        inline bhdr_t *find_suitable_block(int *_fl, int *_sl)
        {
            unsigned int _tmp = sl_bitmap[*_fl] & (~0 << *_sl);
            bhdr_t *_b = nullptr;
            
            if (_tmp) {
                *_sl = ls_bit(_tmp);
                _b = matrix[*_fl][*_sl];
            } else {
                *_fl = ls_bit(fl_bitmap & (~0 << (*_fl + 1)));
                if (*_fl > 0) {         /* likely */
                    *_sl = ls_bit(sl_bitmap[*_fl]);
                    _b = matrix[*_fl][*_sl];
                }
            }
            return _b;
        }
        
        inline void extract_block_hdr(bhdr_t* _b, int _fl, int _sl)
        {
            matrix[_fl][_sl] = _b->ptr.free_ptr.next;
            if (matrix[_fl][_sl])
                matrix[_fl][_sl]->ptr.free_ptr.prev = nullptr;
            else {
                clear_bit(_sl, &sl_bitmap[_fl]);
                if (!sl_bitmap [_fl])
                    clear_bit(_fl, &fl_bitmap);
            }
            _b->ptr.free_ptr.prev = _b->ptr.free_ptr.next = nullptr;
        }
        
        inline void extract_block(bhdr_t* _b, int _fl, int _sl)
        {
            bhdr_t *prev = _b->ptr.free_ptr.prev;
            bhdr_t *next = _b->ptr.free_ptr.next;
            
            if (next)
                next->ptr.free_ptr.prev = _b->ptr.free_ptr.prev;
            if (prev)
                prev->ptr.free_ptr.next = _b->ptr.free_ptr.next;
            
            if (matrix[_fl][_sl] == _b) {
                matrix[_fl][_sl] = _b->ptr.free_ptr.next;
                if (!matrix[_fl][_sl]) {
                    clear_bit(_sl, &sl_bitmap[_fl]);
                    if (!sl_bitmap[_fl])
                        clear_bit(_fl, &fl_bitmap);
                }
            }
            _b->ptr.free_ptr.prev = _b->ptr.free_ptr.next = nullptr;
        }
        
        inline void insert_block(bhdr_t* _b, int _fl, int _sl)
        {
            _b->ptr.free_ptr.prev = nullptr;
            _b->ptr.free_ptr.next = matrix[_fl][_sl];
            if (matrix[_fl][_sl])
                matrix[_fl][_sl]->ptr.free_ptr.prev = _b;
            matrix[_fl][_sl] = _b;
            set_bit(_sl, &sl_bitmap[_fl]);
            set_bit(_fl, &fl_bitmap);
        }
    };
}
