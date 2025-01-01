# Linear Hash Index

## Overview

The `LinearHashIndex` class manages the linear hash index data structure. It provides methods to insert records, rehash the index, handle overflow pages, and lookup records by ID.

### Methods

- `insertRecord(Record record)`: Inserts a new record into the index.
- `rehash()`: Handles the rehashing process when the number of records exceeds a certain threshold.
- `getPage(int pageOffset, fstream &IndexFile, bool overflow = false)`: Reads a page from the index file.
- `writePage(Page page, int pageOffset, fstream &IndexFile, bool overflow = false)`: Writes a page to the index file.
- `createEmptyPage(fstream &IndexFile, int pageOffset = 0, bool overflow = false)`: Creates an empty page in the index.
- `bitHash(int id, bool bitFlip = false)`: Generates a hash of the record ID.
- `findRecordById(int id)`: Finds a record by its ID.

## Inserting Records

When a new record is inserted, the `insertRecord` method determines the appropriate page to store the record based on its hash value. If the page is full, it checks if the bit-flipped location can hold the record. If not, it creates an overflow page. If the number of records exceeds a certain threshold, the index is rehashed.

## Rehashing

The `rehash` method increases the number of buckets in the index and redistributes the records. It creates a new empty page and updates the block directory. If necessary, it increases the number of bits used to address the buckets.

## Handling Overflow Pages

If a page is full and the bit-flipped location cannot hold the record, an overflow page is created. The overflow page is linked to the original page, and records are stored in the overflow page.

## Hashing Mechanism

The `bitHash` method generates a hash of the record ID and returns the least significant bits. If bit flipping is enabled, it flips the least significant bit. The hash value is used to determine the appropriate page to store the record.

## Index File Structure

The index file consists of pages, each with a fixed size. Each page contains records and an overflow offset. If a page is full, an overflow page is created and linked to the original page.

## Compiling and Running the Code

To compile the code, run the following command:

```sh
g++ -std=c++11 -g main.cpp -o main.out
```

To run the code, execute the following command:

```sh
./main.out
```

## Example Usage

The following example demonstrates how to use the `LinearHashIndex` class to create an index and lookup records:

```cpp
#include <string>
#include <iostream>
#include "classes.h"

int main() {
    // Create the index
    LinearHashIndex emp_index("EmployeeIndex.bin");
    emp_index.createFromFile("Employee.csv");
    
    // Lookup a record by ID
    int id = 11432121;
    Record record = emp_index.findRecordById(id);
    if (record.id == -1) {
        std::cout << "Employee not found" << std::endl;
    } else {
        record.print();
    }

    return 0;
}
```
