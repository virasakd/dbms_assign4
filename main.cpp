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

    //Create the index
    LinearHashIndex emp_index("EmployeeIndex");
    emp_index.createFromFile("Employee.csv");

  

    // Loop to lookup IDs until user is ready to quit
    // while (1) {
    //     string choice;
    //     int id;
    //     printf("Please enter either desired ID or 'quit' to end program.\n");
    //     cin >> choice;
    //     if (choice == "quit") {
    //         break;
    //     }
    //     id = stoi(choice);
    //     Record desired_record = emp_index.findRecordById(id);
    //     // check if desired_record has null value for ID (-1)
    //     //      print error if so
    //     if (desired_record.id == -1) {
    //         printf("ID: %d NOT FOUND\n", id);
    //         // desired_record.print();
    //     }
    //     else {
    //         desired_record.print();
    //     }
    //     // printf("desired_record.id: %d \n", desired_record.id);
    //     // findRecordById(id);
    // }
    

    return 0;
}
