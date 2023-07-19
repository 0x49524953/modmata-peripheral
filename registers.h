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
static int _bsearch_addr_comparator(const void * addr, const void * reg) {
    return int(*(const uint16_t *)addr) - int((*(const Register **)reg)->address);
    // ( addr < reg.addr : ret -n)
    // ( addr > reg.addr : ret +n )
    // ( addr !< or !> reg.addr: ret 0)
}

// Comparator function for qsort
static int _qsort_addr_comparator(const void * reg0, const void * reg1) {
    return int((*(const Register **)reg0)->address) - int((*(const Register **)reg1)->address);
    // ( reg0->addr < reg1->addr : ret -n)
    // ( reg0->addr > reg1->addr : ret +n )
    // ( reg0->addr !< or !> reg1.addr: ret 0)
}

// Helper function for calloc() calls
static Register ** _genTableOfLen(const uint16_t len) {
    return (Register **)calloc(size_t(len), sizeof(Register*));
}

static Register * _allocateRegister(uint16_t address=0, uint16_t value=0) {
    Register * _temp = (Register*)malloc(sizeof(Register));
    _temp->address = address;
    _temp->value = value;
    return _temp;
}

static const bool validRegister(const Register ** ref) {
    return ref != NULL && *ref != NULL;
}

// Container type to store modbus registers and allow (simulated) "random" indexed access
// (using binary search so not log(1) but the best we have in this situation, log2(n))
class RegisterArray {
    protected:
        // lookupTable: list of Register pointers ( no stl :c )
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
            if (this->registerExists(address)) return (**getRegisterPtr(address)).value;
            else return 0u;
        }

        const bool registerExists(const uint16_t address) const {
            return getRegisterIndex(address) >= 0 ? true : false;
        }

        const void setRegister(const uint16_t address, const uint16_t value) {
            Register ** _temp = getRegisterPtr(address);
            if (validRegister(_temp)) (**_temp).value = value;
            else addRegister(address, value);
        }

        const bool verifySetRegister(const uint16_t address, const uint16_t value) {
            this->setRegister(address, value);
            const Register ** r = this->getRegisterPtr(address);
            if (validRegister(r)) return (**r).value == value;
            else return false;
        }

        const void sort() {
            // Sort using the AVR libc qsort
            if (lookupTable != nullptr && tableSize > 1)
                qsort(lookupTable, tableSize, sizeof(Register*), _qsort_addr_comparator);
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

        #ifdef EXPOSE_TESTS
        Register ** exposeTable() { return lookupTable; }
        const size_t exposeTableSize() { return tableSize; }
        #endif

        const void printRegisters() const {
            for (int i = 0; i < tableSize; i++) {
                Register r = *lookupTable[i];
                Serial.print("Address: ");
                Serial.print(r.address);
                Serial.print(" Value: ");
                Serial.println(r.value, HEX);
            }
        }
};

#ifdef EXPOSE_TESTS
const void test() {
    while (!Serial);
    delay(2);

    RegisterArray table;

    // Insert in reverse order to an empty table
    // Uses: _genTableOfLen, _allocateRegister, sort, _qsort_addr_comparator
    table.addRegister(5,5);
    table.addRegister(4,4);
    table.addRegister(3,3);
    table.addRegister(2,2);
    table.addRegister(1,1);

    // Set register vals to address * 11
    // Uses: setRegister, getRegisterPtr, validRegister, registerExists
    for (unsigned int i=0; i < 5; i++) {
        if (table.registerExists(i+1))
            table.setRegister(i+1, (i+1)*11);
    }


    // Swap values
    // Uses: swapByAddr, _bsearch_addr_comparator
    table.swapByAddr(1, 5);
    table.swapByAddr(2, 4);

    // Loop through table deleting values
    // Uses: getRegisterIndex, delRegister, getRegisterVal
    for (unsigned int i=0; i < 5; i++) {
        for (unsigned int j=0; j < table.exposeTableSize(); j++) {
            Register ** tr = table.exposeTable()+j;
            const uint16_t registerVal = table.getRegisterVal((**tr).address);

            Serial.print("Index ");
            Serial.print(j, DEC);
            Serial.print(" gives 0x");
            Serial.print((unsigned long)tr, HEX);
            Serial.print(" points to 0x");
            Serial.print((unsigned long)*tr, HEX);
            Serial.print(" points to Register Address ");
            Serial.print((*tr)->address, DEC);
            Serial.print(" with value ");
            Serial.print((*tr)->value, DEC);
            Serial.print(":");
            Serial.println(registerVal, DEC);
        }

        table.delRegister(i+1);
        Serial.println("");
    }

    // End result should look like this:
    //  Index 0 gives 0x205 points to 0x1FF points to Register Address 1 with value 55:55
    //  Index 1 gives 0x207 points to 0x1F3 points to Register Address 2 with value 44:44
    //  Index 2 gives 0x209 points to 0x1E7 points to Register Address 3 with value 33:33
    //  Index 3 gives 0x20B points to 0x1ED points to Register Address 4 with value 22:22
    //  Index 4 gives 0x20D points to 0x1E1 points to Register Address 5 with value 11:11
    //
    //  Index 0 gives 0x211 points to 0x1F3 points to Register Address 2 with value 44:44
    //  Index 1 gives 0x213 points to 0x1E7 points to Register Address 3 with value 33:33
    //  Index 2 gives 0x215 points to 0x1ED points to Register Address 4 with value 22:22
    //  Index 3 gives 0x217 points to 0x1E1 points to Register Address 5 with value 11:11
    //
    //  Index 0 gives 0x209 points to 0x1E7 points to Register Address 3 with value 33:33
    //  Index 1 gives 0x20B points to 0x1ED points to Register Address 4 with value 22:22
    //  Index 2 gives 0x20D points to 0x1E1 points to Register Address 5 with value 11:11
    //
    //  Index 0 gives 0x203 points to 0x1ED points to Register Address 4 with value 22:22
    //  Index 1 gives 0x205 points to 0x1E1 points to Register Address 5 with value 11:11
    //
    //  Index 0 gives 0x1DD points to 0x1E1 points to Register Address 5 with value 11:11
}
#endif // RUN_TESTS

#endif // REGISTERS_H
