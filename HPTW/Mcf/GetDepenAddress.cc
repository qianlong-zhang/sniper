#include<iostream>
#include<vector>
#include<map>
#include<unordered_map>
#include<fstream>
#include<string>
#include<assert.h>
using namespace std;
const int  PPW_SIZE = 128;
const int  CT_SIZE = 256;
const int  PRQ_SIZE = 32;

class correlation_entry {
private:
    string ProducerPC;
    string ConsumerPC;
    string opcode;
public:
    correlation_entry()
    {
        ProducerPC = "";
        ConsumerPC = "";
        opcode = "";
    }
    ~correlation_entry()
    {
    }

    void SetCT(string Producer, string Consumer, string opc)
    {
        ProducerPC = Producer;
        ConsumerPC = Consumer;
        opcode = opc;
    }
};

int main(int argc, char * argv[])
{
    int linenum = 0;
    ifstream in_file;
    ofstream out_file;
    string line;

    string    PR;
    string    CN;
    unordered_map<string, string>  ppw;                              //AddressValue, Producer
    vector <correlation_entry> ct(CT_SIZE);
    unordered_map<string, string> prq;                                 //ProgramCounter, AddressValue
    correlation_entry temp_entry;

    string pc;
    string inst;
    string opcode;
    int index;

    if (argc != 3)
    {
        cout<<"Error: ./this sift.dump out_file"<<endl;
        return 0;
    }
    in_file.open(argv[1]);
    out_file.open(argv[2]);
    if(!in_file.is_open() || !out_file.is_open())
    {
        cout<<"Error: opening file failed!"<<endl;
        return 0;
    }

    string last_line;
    bool physical_flag=false;
    int i = 0;
    string physical_address;
    while(getline(in_file, line))
    {
        //cout<<line<<endl;
        //this line is the additional information of last line, cause it has "--"
        if( line.find_first_of("--",0) != string::npos )
        {
            physical_flag = true;
            cout<<line<<endl;
            // this line is an physical address
            if (line.find("addr",0) != string::npos)
            {
                //get real physical address
                physical_address = line.substr(line.find("addr",0)+5, line.size());
                cout << "  physical_address = "<<physical_address<<endl;
                //cout<<hex<<atoi(physical_address.c_str())<<endl;
            }


            assert(ppw.count(physical_address) <= 1);
            //step 1
            if (ppw.count(physical_address))
            {
                for (unordered_map<string, string>::iterator ppw_it = ppw.begin();
                        ppw_it != ppw.end();
                        ppw_it ++)
                {
                    if (ppw_it->first.compare(physical_address) == 0)
                        PR = ppw_it->second;    //producer
                }
                //step 2 put PR/CN/TMPL into CT
                //ct.insert(make_pair<uint64_t, unordered_map<uint64_t, string > > (PR, pc, opcode);
                temp_entry.SetCT(PR, pc, inst);
                ct.push_back(temp_entry);
                if (ct.size() >= CT_SIZE)
                    ct.erase(ct.end() );

                //step3
                ppw.insert(unordered_map<string, string>::value_type (physical_address, pc) );
                if (ppw.size() >=  PPW_SIZE )
                    ppw.erase(ppw.end() );
            }
        }
        //that is a normal line
        else
        {
            pc=line.substr(0,16);  //get instruction address, PC
            inst=line.substr(17,43);  //get instruction string
            opcode=line.substr(60,line.size());  //get opcode
            index=0;

            //get rid of space in opcode
            while( (index = opcode.find(' ', index)) != string::npos )
            {
                opcode.erase(index,1);
            }

            cout <<" pc "<<pc<<endl;
            cout <<" inst: "<<inst<<endl;
            cout<<" opcode: "<<opcode<<endl;
        }
        last_line=line;
        linenum++;
    }



    in_file.close();
    out_file.close();
    return 0;
}
