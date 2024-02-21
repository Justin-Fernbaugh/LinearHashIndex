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

// create a class page which takes in the page string and creates variables to work with the data
// then use the variables to return it to the string format
class Page {
    public: 
        string page;
        int pageSize;
        int overflowOffset; // 0 if no overflow page
        vector<Record> records;

        Page(string page) {
            this->page = page;
            this->pageSize = 0;
            this->overflowOffset = 0;
            this->records = vector<Record>();
            parsePage();
            // printf("Page() returned\n");
        }
        Page() {
            this->page = "";
            this->pageSize = 0;
            this->overflowOffset = 0;
            this->records = vector<Record>();
        }

        void parsePage() {
            // parse the page string into the variables
            string delimiter = ";";
            size_t pos = 0;
            string token;
            while ((pos = page.find(delimiter)) != string::npos) {
                token = page.substr(0, pos);
                page.erase(0, pos + delimiter.length());

                vector<string> recordFields;
                string csvField;
                stringstream streamTuple(token);
                // printf("page() token: %s\n", token.c_str());
                for(int i=0; i < 4; i++) {
                    getline(streamTuple, csvField, ',');
                    if(csvField.find("Record") != string::npos) {
                        csvField = csvField.substr(csvField.find(":") + 1);
                    }
                    recordFields.push_back(csvField);
                    // printf("page() found record %s\n", csvField.c_str());
                }
                Record tmpRecord(recordFields);
                records.push_back(tmpRecord);
                    
            }
        }

        string pageToString() {
            // string pageString = "overflowOffset:" + to_string(overflowOffset) + ";pageSize:" + to_string(pageSize) + ";";
            string pageString;
            for(int i = 0; i < records.size(); i++) {
                pageString += "Record:" + to_string(records[i].id) + "," + records[i].name + "," + records[i].bio + "," + to_string(records[i].manager_id) + ";";
            }
            return pageString;
        }
};

class LinearHashIndex {

private:
    const int BLOCK_SIZE = 4097;
    const int EXTENSION_LIMIT = BLOCK_SIZE * 0.7; // modify the decimal to raise the cap to whatever % of the BLOCK_SIZE
    const int OVERFLOW_OFFSET = BLOCK_SIZE * 128; // 256 is the number of blocks in the index
    const int MAX_PAGE_SIZE = 2; // Maximum number of records in a page

    vector<int> blockDirectory; // Map the least-significant-bits of h(id) to a bucket location in EmployeeIndex (e.g., the jth bucket)
                                // can scan to correct bucket using j*BLOCK_SIZE as offset (using seek function)
								// can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int n;  // The number of indexes in blockDirectory currently being used
    int i;	// The number of least-significant-bits of h(id) to check. Will need to increase i once n > 2^i
    int numRecords;    // Records currently in index. Used to test whether to increase n
    int nextFreeBlock; // Next place to write a bucket. Should increment it by BLOCK_SIZE whenever a bucket is written to EmployeeIndex
    string IndexFileName;      // Name of index file

    // Insert new record into index
    Page insertRecord(Record record) {
        printf("insertRecord() \n");
        // printf("insertRecord() n: %d & i: %d\n", n, i); // print size of n and i
        fstream IndexFile(IndexFileName, ios::binary | ios::in | ios ::out);

        // Add record to the index in the correct block, creating a overflow block if necessary
        // if page is full check if bit flip location can hold the record. 
        // if bit flip location can't hold the record, place it in the overflow page
        // if bit flip location can hold the record, place it there
        // if page is not full, place the record in the page

        int pageOffset = blockDirectory[bitHash(record.id)] - 1; // get the page offset from the blockDirectory
        // printVector(blockDirectory);
        // printf("insertRecord() blockDirectory: %d\n", blockDirectory[bitHash(record.id)] - 1);
        // printf("insertRecord() pageOffset: %d\n", pageOffset);
        Page page = getPage(pageOffset, IndexFile); // have this return the Page class instead of vector<string>
        // printf("insertRecord() page: %s\n", page.pageToString().c_str());

        // if the page isn't full then add the record to the page
        if(page.pageSize < 2) {
            // If page isn't full place record in page
            // printf("insertRecord() normal page\n");
            page.records.push_back(record);
            page.pageSize++;
            numRecords++;
            writePage(page, pageOffset, IndexFile);

            IndexFile.close();
            // printf("insertRecord() ret\n\n");
            return page;
        }
        // printf("Page is full\n");
        
        // if page is full check if bit flip location can hold the record.
        int bitFlipOffset = blockDirectory[bitHash(record.id, true)] - 1;
        Page bitFlipPage = getPage(bitFlipOffset, IndexFile);
        if(bitFlipPage.pageSize < 2) {
            // If bit flip location can hold the record, place it there
            // printf("insertRecord() bitFlipPage\n");
            bitFlipPage.records.push_back(record);
            bitFlipPage.pageSize++;
            numRecords++;
            writePage(bitFlipPage, bitFlipOffset, IndexFile);

            IndexFile.close();
            // printf("insertRecord() ret\n\n");
            return bitFlipPage;
        }

        // Page overflow logic
        // Create overflow page if it doesn't already exist
        if(page.overflowOffset == 0) { // when page currently has no overflow page
            // printf("insertRecord() page.overflowOffset == 0 \n");
            //Create overflow page and place record in it
            int overflowOffset = createEmptyPage(IndexFile, pageOffset, true); // create an empty page at the original page index + overflow offset
            Page overflowPage = getPage(pageOffset, IndexFile, true); // get the overflow page located at the original page index + overflow offset
            overflowPage.records.push_back(record);
            overflowPage.pageSize++;
            numRecords++;
            writePage(overflowPage, pageOffset, IndexFile, true); // use original pageOffset and overflow = true then writePage will determine the correct location

            // Update overflowOffset in original page
            page.overflowOffset = 1;
            writePage(page, pageOffset, IndexFile); // write OG page to update overflowOffset

            
        } else if(page.overflowOffset == 1) { // If page currently has an overflow page
            // printf("insertRecord() page.overflowOffset > 0 \n");
            // Place record in existing overflow page
            Page overflowPage = getPage(pageOffset, IndexFile, true); // get the overflow page located at the original page index + overflow offset
            if(overflowPage.pageSize > 1) { // Overflow page is full then, create new overflow page and place record in it
                // printf("insertRecord() overflowPage max capacity\n");
                // If overflow page is full, create new overflow page and place record in it
                int overflowOffset = createEmptyPage(IndexFile, overflowOffset, true); // create empty page at overflowOffset;
                Page newOverflowPage = getPage(pageOffset, IndexFile, true); // get the overflow page located at the original page index + overflow offset
                newOverflowPage.records.push_back(record);
                newOverflowPage.pageSize++;
                numRecords++;
                writePage(newOverflowPage, overflowOffset, IndexFile, true);

                overflowPage.overflowOffset = 1;
                writePage(overflowPage, pageOffset, IndexFile, true); // write OG page to update overflowOffset (not sure if this is necessary, might be able to remove it

            } else {
                // continue with adding page to overflow page
                overflowPage.records.push_back(record);
                overflowPage.pageSize++;
                numRecords++;
                writePage(overflowPage, pageOffset, IndexFile, true); // use original pageOffset and overflow = true then writePage will determine the correct location
            }
            
        }

        if( (float)numRecords > (float)n * 5 * 0.7) {
            // printf("insertRecord() rehash n: %d, i: %d\n", n, i);
            rehash();
        }

        IndexFile.close();
        // printf("insertRecord() ret\n\n");
        return page;
    }

    void rehash() {
        int old_n = n;
        int old_i = i;
        n++; // Increase the number of buckets by 1

        fstream IndexFile(IndexFileName, ios::binary | ios::in | ios ::out);
        createEmptyPage(IndexFile); // Create a new empty page
        IndexFile.close();

        blockDirectory.push_back(nextFreeBlock); // Add nextFreeblock (index) to back of blockDirectory
        // printf("rehash() nextFreeBlock: %d\n", nextFreeBlock);
        // printf("rehash() n increased\n");

        if (n > pow(2, i)) {
            i++; // Increase i by 1 if necessary
            // printf("rehash() i increased\n");
        }

        for (int j = 0; j < old_n; j++) {
            // printf("rehash() j: %d\n", j);
            fstream IndexFile(IndexFileName, ios::binary | ios::in | ios ::out);            
            vector<Record> tempRecords; // Temporary structure to hold records during rehashing
            
            Page primaryPage = getPage(j, IndexFile); // Get primary page

            int pageOffset = j; // Get page offset from blockDirectory
            int recordOffset = j;
            // Check if each record in the primary page is in the correct location
            for (auto it = primaryPage.records.begin(); it != primaryPage.records.end(); ++it) {
                int recordOffset = blockDirectory[bitHash(it->id)] - 1; // Get page offset from blockDirectory
                if(recordOffset != pageOffset) {
                    // printf("rehash() move id: %d, pageOffset: %d oldPageOffset: %d\n", it->id, pageOffset, blockDirectory[bitHash(it->id), false, old_i] - 1);
                    tempRecords.push_back(*it);
                    it = primaryPage.records.erase(it); // Erase the record from the primary page's records vector
                    primaryPage.pageSize--;
                    writePage(primaryPage, pageOffset, IndexFile); // Write the cleared primary page
                    --it; // Decrement the iterator to account for the erased element
                }
            }

            // Check if the records in any existing overflow pages are in the correct location
            if (primaryPage.overflowOffset > 0  && primaryPage.pageSize > 0) {
                // printf("rehash() primaryPage.overflowOffset: %d\n", primaryPage.overflowOffset);
                Page overflowPage = getPage(recordOffset, IndexFile, true); // Get overflow page

                // Add records from the overflow page to tempRecords
                for (auto it = overflowPage.records.begin(); it != overflowPage.records.end(); ++it) {
                    int recordOffset = blockDirectory[bitHash(it->id)] - 1; // Get page offset from blockDirectory
                    if (recordOffset != pageOffset) {
                        // printf("rehash() move overflow id: %d, pageOffset: %d oldPageOffset: %d\n", it->id, pageOffset, blockDirectory[bitHash(it->id), false, old_i] - 1);
                        tempRecords.push_back(*it);
                        it = overflowPage.records.erase(it); // Erase the record from the overflow page's records vector
                        overflowPage.pageSize--;
                        writePage(overflowPage, pageOffset, IndexFile, true); // Write the cleared overflow page
                        --it; // Decrement the iterator to account for the erased element
                    }
                }
                if(overflowPage.pageSize == 0) {
                    // printf("rehash() overflowPage.pageSize: %d\n", overflowPage.pageSize);
                    primaryPage.overflowOffset = 0;
                    writePage(overflowPage, pageOffset, IndexFile, true); // Write the cleared overflow page
                }
            }
            IndexFile.close();

            for (const auto& record : tempRecords) {
                // printf("rehash() inserting record: %d\n", record.id);
                insertRecord(record); // Reinsert each record
            }
        }
    }

    Page getPage(int pageOffset, fstream &IndexFile, bool overflow = false) {
        // printf("getPage()\n");
        char temp[4098];
        if(overflow) {
            pageOffset = (pageOffset * BLOCK_SIZE) + OVERFLOW_OFFSET;
            // printf("getPage() overflow pageOffset: %d\n", pageOffset);
        } else {
            pageOffset = pageOffset * BLOCK_SIZE;
            // printf("getPage() normal pageOffset: %d\n", pageOffset);
        }
        
        IndexFile.seekg(pageOffset);
        Page page;
        IndexFile.read(reinterpret_cast<char*>(&page.overflowOffset), sizeof(page.overflowOffset));
        IndexFile.read(reinterpret_cast<char*>(&page.pageSize), sizeof(page.pageSize));
        IndexFile.read(temp, BLOCK_SIZE - 4 - (sizeof(int) * 2));
        // printf("getPage() reading: %s\n", temp);
        // print the number of chars in temp
        // printf("getPage() temp size: %d\n", strlen(temp));

        // remove buffer from the page
        string line(temp);
        // printf("getPage() char to string\n");
        line = line.erase(line.find("~"), string::npos);
        // printf("getPage() line: %s\n", line.c_str());
        // turn the string into the Page class 
        // printf("getPage() creating page\n");
        page.page = line;
        // printf("getPage() parsing \n");
        page.parsePage();

        // printf("getPage() returned \n");
        return page;
    }

    void writePage(Page page, int pageOffset, fstream &IndexFile, bool overflow = false) {
        // printf("writePage()\n");
        if (overflow) {
            pageOffset = (pageOffset * BLOCK_SIZE) + OVERFLOW_OFFSET;
        } else {
            pageOffset = pageOffset * BLOCK_SIZE;
        }
        // printf("writePage() pageOffset: %d\n", pageOffset);

        // write the bytes of the page to the file
        string pageString = page.pageToString();
        // printf("writePage() pageString: %s\n", pageString.c_str());
        pageString = addBuffer(pageString);
        // printf("writePage() addBuffer() size: %d\n", pageString.size());

        const char *pageChar = pageString.c_str();
        // printf("writePage() pageChar: %s\n", pageChar);
        IndexFile.seekg(pageOffset, ios::beg);
        IndexFile.write(reinterpret_cast<const char*>(&page.overflowOffset), sizeof(page.overflowOffset));
        IndexFile.write(reinterpret_cast<const char*>(&page.pageSize), sizeof(page.pageSize));
        IndexFile.write(pageChar, BLOCK_SIZE - (sizeof(int) * 2));
        
    }
    

    // Create an empty page in the index and return location
    // This only create pages and not overflow pages
    // Because when finding an index we might go to it by index * 4096 and then add the overflow offset(implement overflow offset)
    int createEmptyPage(fstream &IndexFile, int pageOffset = 0, bool overflow = false) {
        // printf("Creating page\n");
        // If it's an overflow page then pageOffset passes the index of the page which we use to store the overflow page 
        // at the pageOffset + OVERFLOW_OFFSET (this way we can find the overflow page by adding the overflow offset to the index of the page)
        if(overflow) {
            pageOffset = (pageOffset * BLOCK_SIZE) + OVERFLOW_OFFSET;
        } else {
            pageOffset = nextFreeBlock * BLOCK_SIZE;
        }
        // printf("createEmptyPage() pageOffset: %d\n", pageOffset);

        // string page = "overflowOffset:0;pageSize:0;";
        string page;
        int overflowOffset = 0;
        int pageSize = 0;
        page = addBuffer(page);
        const char *pageChar = page.c_str();
        // printf("createEmptyPage: %s\n\n", pageChar);


        IndexFile.seekg(pageOffset); // move to empty page location or empty overflow page location
        IndexFile.write(reinterpret_cast<const char*>(&overflowOffset), sizeof(overflowOffset));
        IndexFile.write(reinterpret_cast<const char*>(&pageSize), sizeof(pageSize));
        IndexFile.write(pageChar, BLOCK_SIZE - (sizeof(int) * 2));
        // IndexFile.write(reinterpret_cast<char *>(&page), 4096);

        if(!overflow) {
            nextFreeBlock++;
        }
        numRecords++;
        
        return pageOffset; // return the location of the page
    }

    // Creates a hash of the id and returns the least significant bits
    // If bitFlip is true then it will flip the least significant bit (known as bit flipping)
    int bitHash(int id, bool bitFlip = false) {
        int result, hashed;
        hashed = id % static_cast<int>(pow(2, 8));
        result = hashed % static_cast<int>(pow(2, i));
        // printf("bitHash() n: %d, lsbBits: %d\n", n, result);

        if (bitFlip || result >= n) {
            result = hashed % static_cast<int>(pow(2, i - 1));
                // printf("bitHash() bitFlip: %d\n", result);
        }
        return result;
    }

    Record stringToRecord(string line) {
        vector<string> recordFields;
        string csvField;
        stringstream streamTuple(line);

        for(int i=0; i < 4; i++) {
            getline(streamTuple, csvField, ',');
            recordFields.push_back(csvField);
        }
        Record tmpRecord(recordFields);
        csvField.clear();
        return tmpRecord;
    }

    string addBuffer(string record) { // adds padding to string
        string bufferRecord = record;

        while (bufferRecord.size() < BLOCK_SIZE - (sizeof(int) * 2)) {
            bufferRecord.append("~");
        }
        bufferRecord.append("\n");

        return bufferRecord;
    }

    // create a function called printVector which prints each index of a vector
    void printVector(vector<int> vec) {
        for(int i = 0; i < vec.size(); i++) {
            printf("printVector() vec[%d]: %d\n", i, vec[i]);
        }
    }

public:
    LinearHashIndex(string indexFileName) {
        n = 4; // Start with 4 buckets in index
        i = 2; // Need 2 bits to address 4 buckets
        numRecords = 0;
        nextFreeBlock = 0;
        IndexFileName = indexFileName;
        fstream IndexFile(IndexFileName, ios::binary | ios::out | ios::trunc);

        // Create your EmployeeIndex file and write out the initial 4 buckets
        // make sure to account for the created buckets by incrementing nextFreeBlock appropriately
        // printf("Created EmployeeIndex\n");

        for (int j = 0; j < n; j++) {
            createEmptyPage(IndexFile); // Create a new empty page
            blockDirectory.push_back(nextFreeBlock); // Add nextFreeblock (index) to back of blockDirectory
            // printf("constructor() nextFreeBlock: %d\n", nextFreeBlock);
        }
        // make sure to account for the created buckets by incrementing nextFreeBlock appropriately
        IndexFile.close();
        // printf("Closing Index File\n");

        return;
    }

    // Read csv file and add records to the index
    void createFromFile(string csvFName) {
        fstream EmployeeCSV(csvFName, ios::in);
        printf("createFromFile() \n");

        //insert record one by into linear hash index
        string line;
        while(getline(EmployeeCSV, line)) {
            Record newRecord = stringToRecord(line);
            //write the bytes of the page to the file
            // printf("createFromFile() line:  %s \n", line.c_str());
            insertRecord(newRecord);
            // Page page = insertRecord(newRecord);
        }
        EmployeeCSV.close();
        printf("createFromFile() close\n\n");
    }

    // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {
        fstream IndexFile(IndexFileName, ios::binary | ios::in | ios ::out);
        int pageOffset = blockDirectory[bitHash(id)] - 1;
        Page page = getPage(pageOffset, IndexFile);
        for (auto it = page.records.begin(); it != page.records.end(); ++it) {
            if (it->id == id) {
                IndexFile.close();
                return *it;
            }
        }
        if (page.overflowOffset > 0) {
            Page overflowPage = getPage(pageOffset, IndexFile, true);
            for (auto it = overflowPage.records.begin(); it != overflowPage.records.end(); ++it) {
                if (it->id == id) {
                    IndexFile.close();
                    return *it;
                }
            }
        }
        IndexFile.close();
        Record record;
        record.id = -1;
        return record;
        
    }
};
