#include "MemPool.h"
#include "GenericKit/Debug.h"

namespace GenericKit
{
    const int MemPool::table[256] = {
        -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
        4, 4,
        4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5,
        5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6,
        6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6,
        6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7,
        7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7,
        7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7,
        7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7,
        7, 7, 7, 7, 7, 7, 7
    };
    
    MemPool::MemPool(size_t size)
    : total_size(align_up(size))
    , mem_pool(new char[total_size])
    , area_head(nullptr)
    , fl_bitmap(0)
    {
        MemSet(mem_pool, 0, total_size);
        MemSet(sl_bitmap, 0, sizeof(sl_bitmap));
        MemSet(matrix, 0, sizeof(matrix));
        
        bhdr_t* ib = process_area(mem_pool, size);
        bhdr_t* b = get_next_block(ib->ptr.buffer, ib->size & BLOCK_SIZE);
        free(b->ptr.buffer);
        area_head = (area_info_t*)ib->ptr.buffer;
        
        available_size = b->size & BLOCK_SIZE;
        used_size = 0;
    }
    
    MemPool::~MemPool()
    {
        if (nullptr != mem_pool)
            delete[] mem_pool;
        
        mem_pool = nullptr;
    }
    
    void* MemPool::alloc(size_t size)
    {
        void* ret = nullptr;
        
        int fl, sl;
        size = (size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : align_up(size);
        
        /* Rounding up the requested size and calculating fl and sl */
        mapping_search(&size, &fl, &sl);
        
        /* Searching a free block, recall that this function changes the values of fl and sl,
         so they are not longer valid when the function fails */
        bhdr_t* b = find_suitable_block(&fl, &sl);
        
        if (!b)
            return ret;     /* Not found */
        
        extract_block_hdr(b, fl, sl);
        
        bhdr_t* b2 = nullptr;
        /*-- found: */
        bhdr_t* next_b = get_next_block(b->ptr.buffer, b->size & BLOCK_SIZE);
        /* Should the block be split? */
        size_t tmp_size = (b->size & BLOCK_SIZE) - size;
        if (tmp_size >= sizeof(bhdr_t)) {
            tmp_size -= BHDR_OVERHEAD;
            b2 = get_next_block(b->ptr.buffer, size);
            b2->size = tmp_size | FREE_BLOCK | PREV_USED;
            next_b->prev_hdr = b2;
            mapping_insert(tmp_size, &fl, &sl);
            insert_block(b2, fl, sl);
            
            b->size = size | (b->size & PREV_STATE);
        } else {
            next_b->size &= (~PREV_FREE);
            b->size &= (~FREE_BLOCK);       /* Now it's used */
        }
        
        used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
        ret = b->ptr.buffer;
        
        return ret;
    }
    
    void* MemPool::realloc(void* ptr, size_t new_size)
    {
        void* ret = nullptr;
        
        if (!ptr)
        {
            if (new_size)
                return alloc(new_size);
            else
                return nullptr;
        }
        else if (!new_size)
        {
            free(ptr);
            return nullptr;
        }
        
        new_size = (new_size < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : align_up(new_size);
        
        int fl, sl;
        bhdr_t* b = (bhdr_t*)((char*)ptr - BHDR_OVERHEAD);
        bhdr_t* next_b = get_next_block(b->ptr.buffer, b->size & BLOCK_SIZE);
        bhdr_t* tmp_b = nullptr;
        
        size_t tmp_size = (b->size & BLOCK_SIZE);
        if (new_size <= tmp_size)
        {
            used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
            if (next_b->size & FREE_BLOCK)
            {
                mapping_insert(next_b->size & BLOCK_SIZE, &fl, &sl);
                extract_block(next_b, fl, sl);
                tmp_size += (next_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
                next_b = get_next_block(next_b->ptr.buffer, next_b->size & BLOCK_SIZE);
                /* We allways reenter this free block because tmp_size will
                 be greater then sizeof (bhdr_t) */
            }
            tmp_size -= new_size;
            if (tmp_size >= sizeof(bhdr_t)) {
                tmp_size -= BHDR_OVERHEAD;
                tmp_b = get_next_block(b->ptr.buffer, new_size);
                tmp_b->size = tmp_size | FREE_BLOCK | PREV_USED;
                next_b->prev_hdr = tmp_b;
                next_b->size |= PREV_FREE;
                mapping_insert(tmp_size, &fl, &sl);
                insert_block(tmp_b, fl, sl);
                b->size = new_size | (b->size & PREV_STATE);
            }
            used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
            ret = b->ptr.buffer;
            
            return ret;
        }
        if ((next_b->size & FREE_BLOCK)) {
            if (new_size <= (tmp_size + (next_b->size & BLOCK_SIZE))) {
                used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
                
                mapping_insert(next_b->size & BLOCK_SIZE, &fl, &sl);
                extract_block(next_b, fl, sl);
                b->size += (next_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
                next_b = get_next_block(b->ptr.buffer, b->size & BLOCK_SIZE);
                next_b->prev_hdr = b;
                next_b->size &= ~PREV_FREE;
                tmp_size = (b->size & BLOCK_SIZE) - new_size;
                if (tmp_size >= sizeof(bhdr_t)) {
                    tmp_size -= BHDR_OVERHEAD;
                    tmp_b = get_next_block(b->ptr.buffer, new_size);
                    tmp_b->size = tmp_size | FREE_BLOCK | PREV_USED;
                    next_b->prev_hdr = tmp_b;
                    next_b->size |= PREV_FREE;
                    mapping_insert(tmp_size, &fl, &sl);
                    insert_block(tmp_b, fl, sl);
                    b->size = new_size | (b->size & PREV_STATE);
                }
                used_size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
                ret = b->ptr.buffer;
                
                return ret;
            }
        }
        
        if (!(ret = alloc(new_size))){
            return nullptr;
        }
        
        size_t cpsize = ((b->size & BLOCK_SIZE) > new_size) ? new_size : (b->size & BLOCK_SIZE);
        
        MemCopy(ret, ptr, cpsize);
        
        free(ptr);
        return ret;
    }
    
    void MemPool::free(void* ptr)
    {
        if (ptr)
        {
            int fl = 0, sl = 0;
            
            bhdr_t* b = (bhdr_t*)((char*)ptr - BHDR_OVERHEAD);
            b->size |= FREE_BLOCK;
            
            used_size -= (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
            
            b->ptr.free_ptr.prev = b->ptr.free_ptr.next = nullptr;
            bhdr_t* tmp_b = get_next_block(b->ptr.buffer, b->size & BLOCK_SIZE);
            if (tmp_b->size & FREE_BLOCK)
            {
                mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);
                extract_block(tmp_b, fl, sl);
                b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
            }
            
            if (b->size & PREV_FREE)
            {
                tmp_b = b->prev_hdr;
                mapping_insert(tmp_b->size & BLOCK_SIZE, &fl, &sl);
                extract_block(tmp_b, fl, sl);
                tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
                b = tmp_b;
            }
            mapping_insert(b->size & BLOCK_SIZE, &fl, &sl);
            insert_block(b, fl, sl);
            
            tmp_b = get_next_block(b->ptr.buffer, b->size & BLOCK_SIZE);
            tmp_b->size |= PREV_FREE;
            tmp_b->prev_hdr = b;
        }
    }
    
    void MemPool::print_all_blocks()
    {
        area_info_t *ai = area_head;
        bhdr_t *next = nullptr;
        
        DebugPrintf("\nTLSF at %p\nALL BLOCKS\n\n", mem_pool);
        
        while (ai) {
            next = (bhdr_t *) ((char *) ai - BHDR_OVERHEAD);
            while (next) {
                print_block(next);
                if ((next->size & BLOCK_SIZE))
                    next = get_next_block(next->ptr.buffer, next->size & BLOCK_SIZE);
                else
                    next = NULL;
            }
            ai = ai->next;
        }
    }
    
    MemPool::bhdr_t* MemPool::process_area(void *area, size_t size)
    {
        bhdr_t* ib = (bhdr_t*)area;
        ib->size = (sizeof(area_info_t) < MIN_BLOCK_SIZE) ? MIN_BLOCK_SIZE : align_up(sizeof(area_info_t)) | USED_BLOCK | PREV_USED;
        
        bhdr_t* b = (bhdr_t*)get_next_block(ib->ptr.buffer, ib->size & BLOCK_SIZE);
        b->size = align_down(size - 3 * BHDR_OVERHEAD - (ib->size & BLOCK_SIZE)) | USED_BLOCK | PREV_USED;
        b->ptr.free_ptr.prev = b->ptr.free_ptr.next = nullptr;
        
        bhdr_t* lb = get_next_block(b->ptr.buffer, b->size & BLOCK_SIZE);
        lb->prev_hdr = b;
        lb->size = 0 | USED_BLOCK | PREV_FREE;
        
        area_info_t* ai = (area_info_t*)ib->ptr.buffer;
        ai->next = nullptr;
        ai->end = lb;
        
        return ib;
    }
    
    void MemPool::print_block(bhdr_t *b)
    {
        if (!b)
            return;
        DebugPrintf(">> [%p] (", b);
        if ((b->size & BLOCK_SIZE))
            DebugPrintf("%lu bytes, ", (unsigned long) (b->size & BLOCK_SIZE));
        else
            DebugPrintf("sentinel, ");
        if ((b->size & BLOCK_STATE) == FREE_BLOCK)
            DebugPrintf("free [%p, %p], ", b->ptr.free_ptr.prev, b->ptr.free_ptr.next);
        else
            DebugPrintf("used, ");
        if ((b->size & PREV_STATE) == PREV_FREE)
            DebugPrintf("prev. free [%p])\n", b->prev_hdr);
        else
            DebugPrintf("prev used)\n");
    }
}
