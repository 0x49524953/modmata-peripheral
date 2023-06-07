#include <stdint.h>
#include <stdlib.h>

#ifndef REGISTERS_H
#define REGISTERS_H

// Struct to represent any kind of modbus register in memory
typedef struct Register {
    uint16_t address;
    uint16_t value;
};

// Comparator function for binary searching
int _bsearch_addr_comparator(const void * addr, const void * reg) {
    return int(*(const uint16_t *)addr) - int((*(const Register **)reg)->address);
    // ( addr < reg.addr : ret -n)
    // ( addr > reg.addr : ret +n )
    // ( addr !< or !> reg.addr: ret 0)
}

// Comparator function for qsort
int _qsort_addr_comparator(const void * reg0, const void * reg1) {
    return int((*(const Register **)reg0)->address) - int((*(const Register **)reg1)->address);
    // ( reg0->addr < reg1->addr : ret -n)
    // ( reg0->addr > reg1->addr : ret +n )
    // ( reg0->addr !< or !> reg1.addr: ret 0)
}

// Helper function for calloc() calls
Register ** _genTableOfLen(const uint16_t len) {
    return (Register **)calloc(size_t(len), sizeof(Register*));
}

Register * _allocateRegister(uint16_t address=0, uint16_t value=0) {
    Register * _temp = (Register*)malloc(sizeof(Register));
    _temp->address = address;
    _temp->value = value;
    return _temp;
}

const bool validRegister(const Register ** ref) {
    return ref != NULL && *ref != NULL;
}

// Container type to store modbus registers and allow (simulated) "random" indexed access
// (using binary search so not log(1) but the best we have in this situation, log2(n))
class RegisterArray {
    protected:
        // lookupTable: list of Register pointers ( no stl :c )
        // It actually is less data rewritten per size adjustment if 
        // we copy every register and the contents rather than every ptr
        // (more data on AMD64) but I already rewrote it to be this way
        // sizeof(Register) = 4 bytes
        // sizeof(Register*) = 2 bytes
        // On AMD64: sizeof(Register*) = 8 bytes
        Register ** lookupTable = nullptr;
        size_t tableSize = 0;
        
    public:
        RegisterArray() {}

        Register ** getRegisterPtr(const uint16_t address) const {
            return static_cast<Register **>(bsearch(
                static_cast<const void *>(&address),
                static_cast<const void *>(lookupTable),
                tableSize, sizeof(Register*), _bsearch_addr_comparator
            ));
        }

        const signed int getRegisterIndex(const uint16_t address) const {
            // signed int because:
            //  - max value of possible addresses for (signed short) < (unsigned short (aka uint16_t))
            //  - 24-bit signed int type does not and will not exist afaik

            signed int offset {-1}; // index of -1 == register DNE
            
            const Register ** ref = static_cast<Register **>(bsearch(
                static_cast<const void *>(&address),
                static_cast<const void *>(lookupTable),
                tableSize, sizeof(Register*), _bsearch_addr_comparator
            ));

            if (validRegister(ref)) {
                // Pointer arithmetic:
                // since (member_adddress >= base_address),
                // [num of bytes before member aka:] relative_offset_of_member = member_address - base_address
                // relative_offset / sizeof(member)
                offset = (Register**)ref - lookupTable;
            }

            return offset;
        }

        const uint16_t getRegisterVal(const uint16_t address) const {
            const Register ** _temp = getRegisterPtr(address);
            if (validRegister(_temp)) return (*_temp)->value;
            else return 0;
        }

        const bool registerExists(const uint16_t address) const {
            return validRegister(getRegisterPtr(address));
        }

        const void setRegister(const uint16_t address, const uint16_t value) {
            Register ** _temp = getRegisterPtr(address);
            if (validRegister(_temp)) (*_temp)->value = value;
        }

        const void sort() {
            // Sort using the AVR libc qsort
            if (lookupTable != nullptr && tableSize > 1)
                qsort(lookupTable, tableSize, sizeof(Register*), _qsort_addr_comparator);
        }

        const void addRegister(const uint16_t address, const uint16_t initial_value=0) {
            // Allocate space
            Register ** grownTable = _genTableOfLen(tableSize + 1);
            
            if (lookupTable == NULL) { // add new register to empty table
                lookupTable = grownTable;
                lookupTable[0] = _allocateRegister(address, initial_value);
                tableSize++;
            }   // Trivially sorted

            else {  
                // If the 'old' table was non-empty, copy data & delete old stuff, reassign ptr
                memcpy(grownTable, lookupTable, sizeof(Register*) * (tableSize + 1));
                free(lookupTable);
                lookupTable = grownTable;

                // Populate data of empty register at the end of array
                lookupTable[tableSize] = _allocateRegister(address, initial_value);
                tableSize++;
                this->sort();
            }
        }

        const void swapByAddr(const uint16_t address0, const uint16_t address1) {
            // Swap values of registers at address0 and address1
            Register ** ptr0 = getRegisterPtr(address0);
            Register ** ptr1 = getRegisterPtr(address1);
            if (ptr0 != nullptr && ptr1 != nullptr) {
                // I am so fucking sorry for using XOR swap
                (*ptr0)->value ^= (*ptr1)->value ^= (*ptr0)->value ^= (*ptr1)->value;
            }
        }

        const void delRegister(const uint16_t address) {
            if (!validRegister(lookupTable)) return;        // Empty lookup table

            signed int index = getRegisterIndex(address);   
            if (index == -1) return;                        // Register not found

            bool high =     (index != 0 && index == tableSize);
            bool low =      (index == 0 && tableSize > 0);
            bool onePartition = (high || low);
            bool twoPartition = (tableSize > 0 && !(high || low));

            Register ** shrunkTable = _genTableOfLen(tableSize-1);

            if (onePartition) { // One partition, just shrunk by 1 since we can just copy everything else contiguously 
                memcpy(shrunkTable, lookupTable + (low ? 1 : 0),    sizeof(Register *) * (tableSize-1));
            }

            else if (twoPartition) {  // Two sections to copy for anything not at the edges
                memcpy(shrunkTable,         lookupTable,                sizeof(Register *) * (index));
                memcpy(shrunkTable+index,   lookupTable + (index+1),    sizeof(Register *) * (tableSize-index-1));
            }

            free(lookupTable[index]);   // deallocate register since we used double ptrs
            free(lookupTable);          // deallocate old table
            lookupTable = shrunkTable;
            tableSize--;
            // no need to sort elements that have not changed order relative to deleted register
        }

};

#endif
