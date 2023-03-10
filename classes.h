#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
#include <cmath>
using namespace std;

struct Header {
    uint8_t numSlots;
    uint8_t slots[5];
    uint8_t nextBlock;
};


// create and return a header struct
Header createHeader(char* buffer) {
    Header header = {}; // zero-initialize the struct

    header.numSlots = static_cast<uint8_t>(buffer[0]);

    for (int i = 0; i < 5; i++) {
        header.slots[i] = static_cast<uint8_t>(buffer[i + 1]);
    }
    header.nextBlock = static_cast<uint8_t>(buffer[6]);

    return header;
}

// conver the struct header back to a char array.
char* return_header(Header head){

    char* updated_header = new char[8];
    memset(updated_header,'\0',sizeof(updated_header));
    // cout << sizeof(updated_header) << "sizeof heaader" << endl;
    updated_header[0] = head.numSlots;

    for (int i = 0; i< 5;i++){
        updated_header[i+1] = head.slots[i];
    };

    updated_header[6] = head.nextBlock;
    return updated_header;

}

// created for increment of n.

int flip_msb( int flipped_number, int current_i){
    // cout << "FLIP IS NEEDED HERE TOO" << endl;
    
    int flip_mask = 1 << (current_i-1);

    // Flip the ith bit using XOR
    flipped_number = flipped_number ^ flip_mask;

    return flipped_number;

}

int hash_last_bits(int record_id, int i_bits){
    double temp = pow(2,8); 
    int result = temp;

    // compute the hash function.
    int hash_outcome = record_id  % result;
    int get_i_mask = (1 << i_bits) - 1; // create a mask of i bits set to 1
    // grab the last i bits from the hash outcome
    int last_ibits = hash_outcome & get_i_mask; // apply the mask to the number

    return last_ibits;
}


// compute the has and return the i last bits
int return_last_i_bits(int record_id, int i_bits, int num_buckets, bool &flipped){
    int last_ibits = hash_last_bits(record_id,i_bits);

    // check if the last i bits >= number of bucktets in the directory.
    if (last_ibits >= num_buckets){
        // cout << "FLIP IS NEEDED" << endl;
            int flip_mask = 1 << (i_bits-1);

        // Flip the ith bit using XOR
        last_ibits = last_ibits ^ flip_mask;

        flipped = true;
    }
    return last_ibits;
}


// convert and int to string to char array.

char* return_id(int id){
    char* id_array = new char[9];
    string temp_string = to_string( id);

    strcpy(id_array, temp_string.c_str());


    return id_array;

}


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
    const int HEADER_SIZE = 8;
    const int RECORD_SIZE = 718;

    vector<int> blockDirectory; // Map the least-significant-bits of h(id) to a bucket location in EmployeeIndex (e.g., the jth bucket)
                                // can scan to correct bucket using j*BLOCK_SIZE as offset (using seek function)
								// can initialize to a size of 256 (assume that we will never have more than 256 regular (i.e., non-overflow) buckets)
    int n;  // The number of indexes in blockDirectory currently being used
    int i;	// The number of least-significant-bits of h(id) to check. Will need to increase i once n > 2^i
    int numRecords;    // Records currently in index. Used to test whether to increase n
    int blockRecords;   // number of records in non overflow blocks
    int nextFreeBlock; // Next place to write a bucket. Should increment it by BLOCK_SIZE whenever a bucket is written to EmployeeIndex
    int total_buckets;  // total buckets in the Index file
    int block_size;
    string fName = "";
    fstream outfile;
    int overflowBlocks;
    
    // write a dynamic char array back to the file.
    void dynamic_to_file(int offset_spot, char *content_arary){
        outfile.seekp(offset_spot,ios::beg); 
        outfile.write(content_arary,sizeof(content_arary));

    }

    // write the record to the open spot
    void record_open_spot(Record record, int start_position){

        char * id_array = return_id(record.id);
        char* manager = return_id(record.manager_id);
       
        char name[201];
        memset(name,'\0',sizeof(name));
        char bio[501];
        memset(bio,'\0', sizeof(bio));
        strcpy(name, record.name.c_str());
        strcpy(bio, record.bio.c_str());

        // cout << "name " << sizeof(id_array) << "manager size" << sizeof(manager) << "name size " << sizeof(name) << " bio size " << sizeof(bio) << endl;



        // write record to the file.
        //write the id, which is 8 bytes.
        int offset_tracker = start_position;
        dynamic_to_file(offset_tracker,id_array);

        //write the manager id, which is also 8 bytes
        offset_tracker += 8;
        dynamic_to_file(offset_tracker, manager);
        offset_tracker += 8;

        // write the name, which is 200 bytes
        outfile.seekp(offset_tracker,ios::beg); 
        outfile.write(name,sizeof(name));

        offset_tracker += 201;
        // finally write the bio, which 500 bytes
        outfile.seekp(offset_tracker,ios::beg); 
        outfile.write(bio,sizeof(bio));
       
        // clear alloacted memory
        delete[] id_array;
        delete[] manager;


    }

    // start the process for inserting a record into a bucket
    void insert_into_bucket(Record record, Header header,int block_index, bool flipped){

        header.numSlots += 1;
        int open_index = 0;

        for(int i = 0; i < 5; i++){
            if((int)header.slots[i] == 0){
                if (flipped){
                    header.slots[i] = 1;
                }else{
                    header.slots[i] = 1;
                }
                
                open_index = i;
                break;

            }
        }

        // write updated header back to the file.
        char *updated_header = return_header(header);
        dynamic_to_file(block_index * BLOCK_SIZE, updated_header);
        delete[] updated_header;

        //find the position of the open slot
        int insert_offset = (RECORD_SIZE * open_index) + ((block_index * BLOCK_SIZE) + HEADER_SIZE);

        record_open_spot(record,insert_offset);
       
    }

    void add_to_bucket(Record record, int block_index, bool flipped){

        // loop until we find an open space to insert the record.
        bool inserting = true;
        bool overflow_flag = false;
        while(inserting){

            // grab the header of the current block
            char headerTxt[7];
            outfile.seekg(block_index * BLOCK_SIZE,ios::beg);
            outfile.read(headerTxt,HEADER_SIZE);
            Header bucket_header = createHeader(headerTxt);
            
            // check if there are blocks open in this bucket.

            if ((int)bucket_header.numSlots < 5){
                
                // call a function that insert the record into the open spot
                insert_into_bucket(record,bucket_header,block_index, flipped);
                inserting = false;

                // only cout records that have been inserted into non overflow blocks.
                if(!overflow_flag){
                    blockRecords++;
                }

            }// if there aren't any spots open, check for a over flow bucket. If one exist then iterate to it, otherwise create the overflow bucket.
            else{

                if((int)bucket_header.nextBlock == 0){

                    // update the overflow pointer in the header of the current bucket
                    bucket_header.nextBlock = nextFreeBlock;

                    // cout << record.name << "  OVERFLOW person " << endl;

                    // write the updated header back to the current bucket
                    char *new_header = return_header(bucket_header);
                    dynamic_to_file(block_index * BLOCK_SIZE,new_header);
                    delete[] new_header;

                    // set the block index to the next overflow bucket
                    block_index = nextFreeBlock;
                    
                    // outfile.seekp(block_index *BLOCK_SIZE,ios::beg);
                    outfile.seekp(nextFreeBlock * BLOCK_SIZE,ios::beg);

                    // write the new overflow bucket to the file.
                    char header[8];
                    memset(header,0,sizeof(header));
                    char emptyBuffer[4088];
                    memset(emptyBuffer, 45, sizeof(emptyBuffer));
                    outfile.write(header,sizeof(header));
                    outfile.write(emptyBuffer,sizeof(emptyBuffer));
                    nextFreeBlock++;
                    total_buckets++;

                    //set the overvlow flag.
                    overflow_flag = true;
                    overflowBlocks++;
                    

                }else{
                    // move the current page index to the over flow 
                    block_index = (int)bucket_header.nextBlock;
                }

            }
            
        }

    }


    string convert_char_string(int array_size, char* temp_array){

        string str = "";
        for (int h = 0; h < array_size; h++){
            if(temp_array[h] == '\0' ){
                break;
            }
            str = str + temp_array[h];
        }
        return str;
    }

    // read in the record from the page.
    Record readin_record(int current_offset){

        vector<std::string> temp_record;
        char emp_id[9];
        memset(emp_id,'\0',sizeof(emp_id));
        outfile.read(emp_id,sizeof(emp_id) -1);
        string str1 = convert_char_string(9,emp_id);
       
        

        char man_id[9];
        memset(man_id,'\0',sizeof(man_id));
        outfile.read(man_id,sizeof(man_id) -1);
        string str2 = convert_char_string(9,man_id);
        // cout << " man_id " << str2 << endl;
       

        char name[201];
        memset(name,'\0',sizeof(name));
        outfile.read(name,sizeof(name));
        string str3 = convert_char_string(201,name);
        

        char bio[501];
        memset(bio,'\0', sizeof(bio));
        outfile.read(bio,sizeof(bio));
        string str4 = convert_char_string(501,bio);
        

        temp_record.push_back(str1);
        temp_record.push_back(str3);
        temp_record.push_back(str4);
        temp_record.push_back(str2);
        Record new_record = Record(temp_record);
        

        return new_record;
        // convert 
        
    }


    //check for missed placed records in the file.
    void check_misplaced_records(int current_bucket, int new_bucket, int ptr_newBucket){

        bool checking = true;
        // iterate until all records in this bucket and it's overflow have been properlu
        while(checking){
            // read in the header of the current page.

            char headerTxt[7];
            outfile.seekg(current_bucket * BLOCK_SIZE,ios::beg);
            outfile.read(headerTxt,HEADER_SIZE);
            Header bucket_header = createHeader(headerTxt);
            int overflow = (int)bucket_header.nextBlock;

            int position_after_header = (current_bucket * BLOCK_SIZE) + HEADER_SIZE;

            for (int j = 0; j < 5; j++){

                // if a record exist in this spot
               
                if((int)bucket_header.slots[j] != 0){
                   

                    int record_spot = position_after_header + (j * RECORD_SIZE);
                    // find the record offset in the page
                    outfile.seekg(record_spot,ios::beg);
                    // read in the record id
                    char id_record[9];
                    memset(id_record, '\0', sizeof(id_record));
                    outfile.read(id_record,sizeof(id_record)-1);
                    string str = "";
                    for(int k = 0; k< 9;k++){
                        str = str + id_record[k];

                    } 

                    // convert char to int.
                    int id = stoi(str);
                    
                    // now check if the id belongs to this bucket.
                    bool flipped = false;
                    int last_bits = hash_last_bits(id,i);

                    // check if the record belongs in the 
                    // cout << last_bits << " = " << current_bucket << endl;
                    if(last_bits == new_bucket){
                        // cout << "move to it the new bucket " << id << endl;
   
                        // update current buckets header.
                        bucket_header.numSlots += -1;
                        bucket_header.slots[j] = 0;

                       // return the updated header,
                    
                        outfile.seekg(record_spot,ios::beg);
                        Record current_record = readin_record(record_spot);
                        // cout << "moved " << current_record.name << " " << endl;

                        // insert record in to new bucket
                        add_to_bucket(current_record,ptr_newBucket,flipped);
                        // overwrite the record with nothing.
                        outfile.seekp(record_spot,ios::beg);

                        char overwrite[718];
                        memset(overwrite,45, sizeof(overwrite));
                        outfile.write(overwrite,sizeof(overwrite));

                    
                    }

                }

            }
            // once we have iterated through each slot in the bucket, updatet header.
            char * new_header = return_header(bucket_header);
            outfile.seekp(current_bucket * BLOCK_SIZE,ios::beg);
            outfile.write(new_header,sizeof(new_header));
            delete [] new_header;

            if(overflow == 0){
                checking = false;

            }
            // go to the overflow bucket.
            current_bucket = overflow;

        }
    }

   
    // Insert new record into index
    void insertRecord(Record record) {

        // create the first 4 buckets
        if (numRecords == 0) {
            outfile.open(fName, ios::out | ios::app);
            // Initialize index with first blocks (start with 4)

            char header[8];
            memset(header,0,sizeof(header));
            char emptyBuffer[4088];
            memset(emptyBuffer, 45, sizeof(emptyBuffer));

            // create the first four buckets and their headers.
            for(int i = 0; i < 4; i++){
                // add the bucket index tot the block directory
                blockDirectory.push_back(i);
                
                // set the file pointer to the start of the bucket
                outfile.seekp(i*BLOCK_SIZE,ios::beg);
                // write the header, then the rest of the space in the bucket. 7 bytes for the header, 4089 for the records
                outfile.write(header,sizeof(header));
                outfile.write(emptyBuffer,sizeof(emptyBuffer));

            }
            // set the offset to the next open bucket space.
            nextFreeBlock = 4;

            total_buckets = 4;
            outfile.close();

        }
        
        outfile.open(fName, ios::binary | ios::in | ios::out);

        // compute the hash function and return the last i bits.
        bool flipped_flag = false;
        int last_ibits = return_last_i_bits(record.id,i,n,flipped_flag);
        int block_index = blockDirectory[last_ibits];

      
        add_to_bucket(record, block_index, flipped_flag);
        // outfile.flush();
        numRecords++;
    

        double current_capacity = numRecords / (n * 5.0); // five is the capacity for a bucket
        // cout << numRecords << " capacity: " << current_capacity << endl;

        if( current_capacity > 0.70){

            // create a new bucket 
            // create a spot in the directory.
            // // set the file pointer to the open space
            outfile.seekp(nextFreeBlock *BLOCK_SIZE,ios::beg);
            blockDirectory.push_back(nextFreeBlock);
            
            total_buckets++;
            n++;


            char header[8];
            memset(header,0,sizeof(header));
            char emptyBuffer[4088];
            memset(emptyBuffer, 45, sizeof(emptyBuffer));
            outfile.write(header,sizeof(header));
            outfile.write(emptyBuffer,sizeof(emptyBuffer));

            if((double)n > pow(2,i)){
                
                i++;
            }
            // increment to the next blcok spot
            nextFreeBlock++;

            // cout << "next_free block " << nextFreeBlock << " i = " << i << " n = " << n << " records "<< numRecords << endl;
            int review_bucket = flip_msb(n -1,i);
            int review_index = blockDirectory[review_bucket];
            int new_bucket = n-1;
            int ptr_newBucket = blockDirectory[n-1];
            // cout << "buckets checked " << review_bucket << " bucket inserted into " << new_bucket << endl;
            check_misplaced_records(review_index, new_bucket, ptr_newBucket);
        }       
      
            
        outfile.close();



    }

public:
    LinearHashIndex(string indexFileName) {
        n = 4;
        i = 2;
        numRecords = 0;
        block_size = 0;
        fName = indexFileName;
        blockRecords = 0;
        total_buckets = 0;
        overflowBlocks = 0;

        // open the file and truncate it if it exist.
    }


    // Read csv file and add records to the index
    void createFromFile(string csvFName) {
   
        // open the input file.
        fstream input_file;
        input_file.open(csvFName, ios::in);
        int check = 0;

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
                // cout << "end of the file"  << line << endl;
                flag = false;
            }
       
            
         
        }

        input_file.close();


    }

   

    // Given an ID, find the relevant record and print it
    Record findRecordById(int id) {
        outfile.open(fName, ios::binary | ios::in | ios::out);


        int outcome = hash_last_bits(id,i);

        // cout << "outcome = " << outcome << endl;

        // check if the outcome is greater than the number of buckets

        if(outcome >= n){
            // flipe the bits
            outcome = flip_msb(outcome,i);

        }

        int block_offset = blockDirectory[outcome];

      

        // search the buckets for the records with this id.
        int atleast_one = 0;
        bool searching = true;
        while(searching){

            char headerTxt[7];
            outfile.seekg(block_offset * BLOCK_SIZE,ios::beg);
            outfile.read(headerTxt,HEADER_SIZE);
            Header bucket_header = createHeader(headerTxt);
            // cout << "header read in = ";

            // for(int j = 0; j < 7; j++){
            //     cout << (int)headerTxt[j] << " ";
            // }
            // cout << endl;
            int block_postition = (block_offset * BLOCK_SIZE);

            for(int k = 0; k < 5 ;k++){
                // check if the slot has record
                if ((int)bucket_header.slots[k] != 0){


                    // move file pointer to the current record.
                    int rec_offset = (k * RECORD_SIZE) + (HEADER_SIZE + block_postition);
                    outfile.seekg(rec_offset,ios::beg);
                    
                    Record current_record = readin_record(rec_offset);

                    // cout << current_record.name << " " <<current_record.id <<  " == " << id << endl;
                    // cout << endl;

                    // check if the id's match
                    if(current_record.id == id){
                        atleast_one++;
                        // print out the records with a matching id.
                        current_record.print();
                    }
                }
            }

            // check if there is an overflow block;

            if ((int)bucket_header.nextBlock == 0){
                searching = false;
            }else{
                
                cout << endl;
                block_offset = (int)bucket_header.nextBlock;
            }

        }

        if (atleast_one == 0){
            cout << "sorry " << atleast_one << " records with the id of " << id << " exist "<< endl;
        } 



        

        // // return a dummy record.
        string str = "12314215";
        vector<std::string> temp_record;
        temp_record.push_back(str);
        temp_record.push_back(str);
        temp_record.push_back(str);
        temp_record.push_back(str);
        Record null_record = Record(temp_record);
        outfile.close();
        return null_record;
    }

 
};

