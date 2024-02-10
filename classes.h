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

    Record() {}

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

class Page {
    public:
    int overflowOffset = -1; // offset of overflow page ( likely not next to eachother )
    int pageSize = 0;       // # of records in the page
    Record records[2];
    string buffer;         // used to padd the Page until it's 4096 bytes 
    Page() {}

    //returns size of all objects including buffer
    int size() {
        return (sizeof(overflowOffset) + sizeof(pageSize) + records[0].size() + records[1].size() + buffer.size());
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

    // Insert new record into index
    void insertRecord(Record record) {
        fstream IndexFile(IndexFileName, ios::binary | ios::in | ios::trunc);
        // No records written to index yet
        if (numRecords == 0) {
            for(int j = 0; j < i; j++) {
                createPage(IndexFile);
            }
            // Initialize index with first blocks (start with 4)

        }

        // Add record to the index in the correct block, creating a overflow block if necessary


        // Take neccessary steps if capacity is reached:
		// increase n; increase i (if necessary); place records in the new bucket that may have been originally misplaced due to a bit flip


    }
    
    int createPage(fstream &IndexFile) {
        Page page;                      // initialize page struct (basically header) 
                                        // to keep track of records next to this struct in binary file
        page.overflowOffset = -1;
        page.pageSize = 0;

        IndexFile.write(reinterpret_cast<char *>(&page), 4096);

        blockDirectory.push_back(nextFreeBlock);

        nextFreeBlock++;
        numRecords++;
        
        return nextFreeBlock; // return next block available to use
    }

    // Find a page by ID
    int findPage(int id)
    {
        int result, hashed;
        hashed = getHash(id, n, false); // get hash of id (using least significant bits of id
        result = hashed % (int)(pow(2, i));

        if (result >= n) {
            result = hashed % static_cast<int>(pow(2, i - 1));
        }

        printf("Found page: %d \n", result);
        return blockDirectory[result];
    }

    int getHash(int id, int n, bool flip) {
        return (id % (int)pow(2, 8)% n);
    }

    int getHashBits(int hash, int numBits) {
        return (hash & ((1 << i)) -1);
    }

    string addBuffer(Page page) {
        page.buffer = "";

        while (page.size() < (BLOCK_SIZE - 2)) {
            page.buffer.append("~");
        }
        page.buffer.append("\n");
        printf("buffer size: %d \n", page.buffer.size());

        return page.buffer;
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
        fstream IndexFile(IndexFileName, ios::binary | ios::out | ios::trunc);
        printf("Created EmployeeIndex\n");

        for (int j = 0; j < n; j++) {
            Page page;
            page.overflowOffset = -1;
            page.pageSize = 0;
            // add record to page by hash
            // assign the first 4 records to a page by getting their hash to create a page in the linear hash index
            page.buffer = addBuffer(page);

            printf("Creating page\n");
            IndexFile.write(reinterpret_cast<char *>(&page), sizeof(page));

            blockDirectory.push_back(nextFreeBlock); // add the next block to the blockDirectory
            numRecords++;
            nextFreeBlock += BLOCK_SIZE;
        }
        // make sure to account for the created buckets by incrementing nextFreeBlock appropriately
        IndexFile.close();
        printf("Closing Index File\n");
      
    }

    // Read csv file and add records to the index
    void createFromFile(string csvFName) {
        fstream EmployeeCSV(csvFName, ios::in);

        string line;
        while(getline(EmployeeCSV, line)) {
            Record newRecord = stringToRecord(line);
            // insertRecord(newRecord);
        }
        EmployeeCSV.close();
    }

    // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {
        
    }
};
