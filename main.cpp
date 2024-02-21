/*
Skeleton code for linear hash indexing
*/

#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "classes.h"
using namespace std;


int main(int argc, char* const argv[]) {

    // Create the index
    LinearHashIndex emp_index("EmployeeIndex.bin");
    emp_index.createFromFile("Employee.csv");
    
    // Loop to lookup IDs until user is ready to quit
    string input;
    while (true) {
        cout << "Enter an employee ID to look up: ";
        cin >> input;
        if (input == "quit") {
            break;
        }
        int id = stoi(input);
        Record record = emp_index.findRecordById(id);
        if (record.id == -1) {
            cout << "Employee not found" << endl;
        } else {
            // cout << "Employee found: " << record.id << ", " << record.name << ", " << record.bio << endl;
            record.print();
        }
    }
    

    return 0;
}
