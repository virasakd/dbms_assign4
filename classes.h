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
    updated_header[0] =(int)head.numSlots;

    for (int i = 0; i< 5;i++){
        updated_header[i+1] =(int)head.slots[i];
    };

    updated_header[6] = (int)head.nextBlock;
    return updated_header;

}

// compute the has and return the i last bits
int return_last_i_bits(int record_id, int i_bits, int num_buckets){
    double temp = pow(2,4); 
    int result = temp;

    // compute the hash function.
    int hash_outcome = record_id  % result;
    int get_i_mask = (1 << i_bits) - 1; // create a mask of i bits set to 1
    // grab the last i bits from the hash outcome
    int last_ibits = hash_outcome & get_i_mask; // apply the mask to the number


    // check if the last i bits >= number of bucktets in the directory.
    if (last_ibits >= num_buckets){
        cout << "flip is needed." << endl;
            int flip_mask = 1 << (i_bits-1);

        // Flip the ith bit using XOR
        last_ibits = last_ibits ^ flip_mask;
    }
    return last_ibits;

}


// convert and int to string to char array.

char* return_id(int id){
    char* id_array = new char[9];
    string temp_string = to_string( id);

    strcpy(id_array, temp_string.c_str());

    for(int i = 0; i < 8;i++){
        cout << id_array[i];
    }
    cout << endl;

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
    const int HEADER_SIZE = 7;
    const int RECORD_SIZE = 716;

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
    
    // write a dynamic char array to the file.
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

        offset_tracker += 200;
        // finally write the bio, which 500 bytes
        outfile.seekp(offset_tracker,ios::beg); 
        outfile.write(bio,sizeof(bio));
       
        // clear alloacted memory
        delete[] id_array;
        delete[] manager;


    }

    // start the process for inserting a record into a bucket
    void insert_into_bucket(Record record, Header header,int block_index){

        header.numSlots += 1;
        int open_index = 0;

        // find the first open slot in the page header.
        for(int i = 0; i < 5; i++){
            if((int)header.slots[i] == 0){
                header.slots[i] = 1;
                open_index = i;
                break;

            }
            
        }
        cout << "id: " << record.id << " bucket -> " << block_index << " slot: " << open_index << endl;

        // write updated header back to the file.
        char *updated_header = return_header(header);
        dynamic_to_file(block_index * BLOCK_SIZE, updated_header);
        delete[] updated_header;

        //find the position of the open slot
        int insert_offset = (RECORD_SIZE * open_index) + ((block_index * BLOCK_SIZE) + HEADER_SIZE);

        // cout << insert_offset << ": initial offset position." << endl;
        record_open_spot(record,insert_offset);
       
    }
   
    // Insert new record into file
    void insertRecord(Record record) {

        // create the first 4 buckets
        if (numRecords == 0) {
            outfile.open(fName, ios::out | ios::app);
            // Initialize index with first blocks (start with 4)

            char header[7];
            memset(header,0,sizeof(header));
            char emptyBuffer[4089];
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
        int last_ibits = return_last_i_bits(record.id,i,n);
        int block_index = blockDirectory[last_ibits];

        
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
                // if so, find the first open spot in the bucket.

                // write the block header first.
                insert_into_bucket(record,bucket_header,block_index);
                inserting = false;

                
                // only cout records that have been inserted into non overflow blocks.
                if(!overflow_flag){
                    blockRecords++;
                }
                

            }// if there aren't any spots open, check for a over flow bucket. If one exist then iterate to it, otherwise create the overflow bucket.
            else{
                // need to modularize this later
                if((int)bucket_header.nextBlock == 0){

                    // update the overflow pointer in the header of the current bucket
                    bucket_header.nextBlock = nextFreeBlock;

                    cout << record.name << " overflow person " << endl;

                    // write the updated header back to the current bucket
                    char *new_header = return_header(bucket_header);
                    dynamic_to_file(block_index * BLOCK_SIZE,new_header);
                    delete[] new_header;

                    // set the block index to the next overflow bucket
                    block_index = nextFreeBlock;
                    
                    // cout << "next spot: " << block_index << " offset bytes = " << block_index * BLOCK_SIZE << endl;

                    outfile.seekp(block_index *BLOCK_SIZE,ios::beg);

                    // write the new overflow bucket to the file.
                    char header[7];
                    memset(header,0,sizeof(header));
                    char emptyBuffer[4089];
                    memset(emptyBuffer, 45, sizeof(emptyBuffer));
                    outfile.write(header,sizeof(header));
                    outfile.write(emptyBuffer,sizeof(emptyBuffer));
                    nextFreeBlock++;
                    total_buckets++;

                    //set the overvlow flag.
                    overflow_flag = true;
                    

                }else{
                    // move the current page index to the over flow 
                    block_index = (int)bucket_header.nextBlock;
                }

            }


            
        }

        // outfile.flush();
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
    }


    // Read csv file and add records to the index
    void createFromFile(string csvFName) {
        cout << csvFName << endl;
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
                cout << "end of the file"  << line << endl;
                flag = false;
            }
            
            if(check >= 22){
                break;
            }
            check++;
            
            

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

 
};
