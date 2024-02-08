#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bits/stdc++.h>
#include <bitset>
#include <fstream>

using namespace std;

class Record {
public:
    int id, manager_id;
    std::string bio, name;

    Record(vector<std::string> fields) {
        id = stoi(fields[0]);
        name = fields[1];
        bio = fields[2];
        manager_id = stoi(fields[3]);
    }

    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }

    int size() {
        //  8 * 2 for the size of each int 
        return (16 + name.size() + bio.size());
    }
};

class LinearHashIndex {

private:
    const int BLOCK_SIZE = 4096;
    const int EXTENSION_LIMIT = BLOCK_SIZE * 0.7; // modify the decimal to raise the cap to whatever % of the BLOCK_SIZE

    vector<int> blockDirectory; // Map the least-significant-bits of h(id) to a bucket location in EmployeeIndex (e.g., the jth bucket)
                                // can scan to correct bucket using j*BLOCK_SIZE as offset (using seek function)
								// can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int n;  // The number of indexes in blockDirectory currently being used
    int i;	// The number of least-significant-bits of h(id) to check. Will need to increase i once n > 2^i
    int numRecords;    // Records currently in index. Used to test whether to increase n
    int nextFreeBlock; // Next place to write a bucket. Should increment it by BLOCK_SIZE whenever a bucket is written to EmployeeIndex
    string IndexFileName;      // Name of index file

    struct PageData {
        int overflowOffset = -1; // offset of overflow page ( likely not next to eachother )
        int pageSize = 0;       // # of records next to page
        int recordOneBytes = 0; // used to keep track of size in bytes of record one
        int recordTwoBytes = 0; // used to keep track of size in bytes of record two
    };

    // Insert new record into index
    void insertRecord(Record record) {
        fstream IndexFile(IndexFileName, ios::binary | ios::in | ios::trunc);
        // No records written to index yet
        if (numRecords == 0) {
            for(int j = 0; j < i; j++) {

            }
            // Initialize index with first blocks (start with 4)

        }

        // Add record to the index in the correct block, creating a overflow block if necessary


        // Take neccessary steps if capacity is reached:
		// increase n; increase i (if necessary); place records in the new bucket that may have been originally misplaced due to a bit flip


    }

    int createPage(fstream &IndexFile) {
        PageData page;                  // initialize page struct (basically header) 
                                        // to keep track of records next to this struct in binary file
        page.overflowOffset = -1;
        page.pageSize = 0;
        page.recordOneBytes = 0;
        page.recordTwoBytes = 0;

        IndexFile.write(reinterpret_cast<char *>(&page), sizeof(page));

        nextFreeBlock++;
        numRecords++;
        n++;

        blockDirectory.push_back(nextFreeBlock);
        return nextFreeBlock; // return next block available to use
    }

    int getHash(int id, int n, bool flip) {
        return (id % (int)pow(2, 16)% n);
    }

    int getHashBits(int hash, int numBits) {
        return (hash & ((1 << i)) -1);
    }

    Record stringToRecord(string line) {
        vector<string> recordFields;
        string csvField;
        stringstream streamTuple(line);

        for(int i=0; i < 4; i++) {
            getline(streamTuple, csvField, ',');
            recordFields.push_back(csvField);
            // cout << csvField << endl;
        }
        Record tmpRecord(recordFields);
        csvField.clear();
        return tmpRecord;
    }

public:
    LinearHashIndex(string indexFileName) {
        n = 4; // Start with 4 buckets in index
        i = 2; // Need 2 bits to address 4 buckets
        numRecords = 0;
        nextFreeBlock = 0;
        IndexFileName = indexFileName;

        // Create your EmployeeIndex file and write out the initial 4 buckets
        // make sure to account for the created buckets by incrementing nextFreeBlock appropriately
      
    }

    // Read csv file and add records to the index
    void createFromFile(string csvFName) {
        fstream EmployeeCSV(csvFName, ios::in);

        string line;
        while(getline(EmployeeCSV, line)) {
            Record newRecord = stringToRecord(line);
            insertRecord(newRecord);
        }
        EmployeeCSV.close();
    }

    // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {
        
    }
};
