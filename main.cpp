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
    // int hash_score = 0;
    // cout << sizeof(hash_score) << endl;

  

    // Loop to lookup IDs until user is ready to quit
    while (1) {
        string choice;
        int id;
        cout << "Please enter either desired ID or 'quit' to end program."<< endl;
        cin >> choice;
        if (choice == "quit") {
            break;
        }

        // check and make sure the length of the string they provide is 8.
        if(choice.length() == 8){
            id = stoi(choice);
            Record desired_record = emp_index.findRecordById(id);

        }
        else{
            cout << "sorry the we only accept 8 digits id's: TRY AGAIN" << endl;
        }
       
        // check if desired_record has null value for ID (-1)
       
    }
    

    return 0;
}
