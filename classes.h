#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
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
};


class LinearHashIndex {

private:
    const int BLOCK_SIZE = 4096;
    const int HEADER_SIZE = 12;

    vector<int> blockDirectory; // Map the least-significant-bits of h(id) to a bucket location in EmployeeIndex (e.g., the jth bucket)
                                // can scan to correct bucket using j*BLOCK_SIZE as offset (using seek function)
								// can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int n;  // The number of indexes in blockDirectory currently being used
    int i;	// The number of least-significant-bits of h(id) to check. Will need to increase i once n > 2^i
    int numRecords;    // Records currently in index. Used to test whether to increase n
    int nextFreeBlock; // Next place to write a bucket. Should increment it by BLOCK_SIZE whenever a bucket is written to EmployeeIndex
    int block_size;
    string fName = "";
    fstream outfile;

    // Insert new record into index
    void insertRecord(Record record) {

        // No records written to index yet
        outfile.open(fName, ios::out | ios::app);
        if (numRecords == 0) {
            // Initialize index with first blocks (start with 4)
            blockDirectory.push_back(0);
            blockDirectory.push_back(1);
            blockDirectory.push_back(2);
            blockDirectory.push_back(3);

            // initialize the header for the first blocks.
            char emptyBuffer[4084];
            memset(emptyBuffer, '0', sizeof(emptyBuffer));
            // set the file point to the first page, and write the string bitmap.
            outfile.seekp(0*BLOCK_SIZE,ios::beg);
            outfile << "-,-,-,-,-," << "0" << ":";
            outfile.write(emptyBuffer,sizeof(emptyBuffer));

            // set the file pointer to the second page, and write the string bitmap.
            outfile.seekp(1*BLOCK_SIZE,ios::beg);
            outfile << "+,+,+,+,+," << "0" << ":";
            outfile.write(emptyBuffer,sizeof(emptyBuffer));

            // set the file pointer to the second page, and write the string bitmap.
            outfile.seekp(2*BLOCK_SIZE,ios::beg);
            outfile << "?,?,?,?,?," << "0" << ":";
            outfile.write(emptyBuffer,sizeof(emptyBuffer));

            // set the file pointer to the second page, and write the string bitmap.
            outfile.seekp(3*BLOCK_SIZE,ios::beg);
            outfile << "<,<,<,<,<,"<< 0 << ":";
            outfile.write(emptyBuffer,sizeof(emptyBuffer));

            nextFreeBlock = BLOCK_SIZE * 4;

            

        }
         outfile.close();


        // find hashkey: ID % 2^8
        int hashkey = findHashKey(record.id);
        // cout << "hashkey: " << hashkey << "= " << record.id << " (record.id) % " << (pow((double)2, (double)8)) << "(2^8)" 
        //         << endl;

        // first check if the 
        // first check if the _bit maps match?_
        
        // these are the number of bits we want to compare
        int bit_mask = (1 << i) - 1;
        // int bit_mask = (1 << i);

        // check if last i bits of hashkey are >= to n, flip if so
        // importantBits = last i bits of hashKey
        // if ((record.id & bit_mask) >= (n & bit_mask)) {
        if ((hashkey & bit_mask) >= n) {
            // cout << "last two bits are equal " <<  bit_mask << " " << hashkey << endl;
            cout << "last two bits are equal or greater " <<  bit_mask << " " << hashkey << ": hashkey & bit_mask(" << 
                    (hashkey & bit_mask) << "), n(" << n << ")" <<  endl;
            // for (int idx = 0; idx < count; ++idx)
            // {
                // 'flip the MSB of importantBits' (ie hashkey)
                hashkey = (hashkey & bit_mask) ^ (1<<i);
            // }

                // cout << "last two bits are equal or greater " <<  bit_mask << " " << hashkey << ": hashkey & bit_mask(" << 
                //     (hashkey & bit_mask) << "), n(" << n << ")" <<  endl;

        }else{
            cout << "last are not equal " 
                        << "bit_mask: "<<  bit_mask << " " 
                        << "hashkey: "<< hashkey 
                        << ", i: " << i
                        << "; hashkey & bit_mask(" << (hashkey & bit_mask) 
                        << "), n(" << n << ")" <<  endl;



                    // hashkey = hashkey ^ (1<<bit_mask);
                    // hashkey = (hashkey & bit_mask) ^ (1<<n);
                    // cout << "last are not equal " 
                    //     << "bit_mask: "<<  bit_mask << " " 
                    //     << "hashkey: "<< hashkey 
                    //     << ", i: " << i
                    //     << ", (1 << i): " << (1 << i)
                    //     << ", (1 << i) -1: " << (1 << i) - 1
                    //     << "; hashkey & bit_mask(" << (hashkey & bit_mask) 
                    //     << "), n(" << n << ")" <<  endl;

        }

        
        outfile.open(fName, ios::in);

        for(int i = 0; i < 4;i++){
            outfile.seekg(i*BLOCK_SIZE,ios::beg);
            string line;
            getline(outfile, line,':');
            cout << line << endl;

            

        }
        outfile.close();

        // Add record to the index in the correct block, creating a overflow block if necessary

        
        // outfile.open(fName, ios::out | ios::app);
        // outfile.seekp(0*BLOCK_SIZE,ios::beg);
        // outfile.close();


        // Take neccessary steps if capacity is reached:
		// increase n; increase i (if necessary); place records in the new bucket that may have been originally misplaced due to a bit flip


    }

public:
    LinearHashIndex(string indexFileName) {
        n = 4;
        i = 2;
        numRecords = 0;
        block_size = 0;
        fName = indexFileName;

        
    }

    // Read csv file and add records to the index
    void createFromFile(string csvFName) {
        cout << csvFName << endl;
        // open the input file.
        fstream input_file;
        input_file.open(csvFName, ios::in);



        // begin reading the contents of the file.
        bool flag = true;
        while(flag){
            string line, word;

            // read a line until the end of the file.
            if(getline(input_file,line,'\n')){
                vector<std::string> temp_record;
                stringstream s(line);
                // retrieve the id
                getline(s, word, ',');
                temp_record.push_back(word);
                //retrieve the name,
                getline(s, word, ',');
                temp_record.push_back(word);
                // retrieve the bio,
                getline(s, word, ',');
                temp_record.push_back(word);
                // retrieve the manager id
                getline(s, word, ',');
                temp_record.push_back(word);
                // cout << temp_record[0] << " " << temp_record[1]<< endl;

                // now create a record and insert it.
                Record new_record = Record(temp_record);
                insertRecord(new_record);
                

            }
            else{
                cout << "end of the file"  << line << endl;
                flag = false;
            }
            break;

        }
        input_file.close();


    }

    // Record(vector<std::string> fields) {
    //     id = stoi(fields[0]);
    //     name = fields[1];
    //     bio = fields[2];
    //     manager_id = stoi(fields[3]);
    // }

    // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {

        // if record not found, give record null (-1) values
        Record null_record = Record({"-1", "-1", "-1", "-1"});
        return null_record;
    }

    int findHashKey(int id) {
        return id % ((int)pow((double)2, (double)8));
    }

 
};
