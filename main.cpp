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
    LinearHashIndex emp_index("EmployeeIndex");
    emp_index.createFromFile("Employee.csv");
    
    // Loop to lookup IDs until user is ready to quit
    while (1) {
        string choice;
        int id;
        printf("Please enter either desired ID or 'quit' to end program.\n");
        cin >> choice;
        if (choice == "quit") {
            break;
        }
        id = stoi(choice);
        // findRecsordById(id);
    }
    

    return 0;
}
