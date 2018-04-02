#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
/*
 * CS300 - Program #1
 * Trevor Ozbun (Q243B644)
 * DUE: Tuesday, August 25, 2015
 * COMPLETED: 12:17 AM 8/24/2015
 * No errors or warnings under -Wall      -Wmissing-delclarations however provides a warning due to the template instances not having prototypes.
 */

using namespace std;

//our stock item structure
struct StockItem{
    string desc;//description of our stock item
    int quantity;
    float unitCost;//price per item
    float totalCost;//since this is calculating on the fly this is not really necessary, but required by the assignment.
};

//function prototypes
string StockItem_serialize(StockItem& item);//Convert the struct to a record string - we could read/write directly from streams, but this provides the entry in a standard format - binary options require complicated alignments
void StockItem_unserialize(StockItem& item_out, string entry);//Convert a string record to struct data, given the memory to fill
void StockItem_input(StockItem&);//perform a prompt for each field of StockItem struct
void StockItem_outputRow(StockItem&);//print a StockItem's fields as a table row with fixed-width cells
void StockItems_input(fstream&);//perform the input-prompt loop for multiple StockItem_input calls
void StockItems_outputTable(fstream&);//print a table of all the records from a file using fixed-width cells/rows and display the Grand Total
void printPrice(float);

// I/O automatic conversion templates for getline prompts
template <typename T>
void StringTo(string s, T& data){//perform a quick conversion with stringstream, which terminates at the first invalid char (NOTE: does not support spaces)
    stringstream ss;
    ss<<s;//prepare our stream with the input string
    ss>>data;//output to the specified type using our stream
}
void StringTo(string s, string& data);//overload the template function for 'string' to prevent conversion (NOTE: allows spaces)

template <typename T>
void PromptLineEntry(string prompt,T& output,bool allowblank){//Perform a prompt for a variable needing input, with forced flush
    string input;
	do{
		cout<<prompt<<": ";//print out prompt line to tell the user what to enter
		getline(cin,input);//retrieve the user's input line (NOTE: requires an <ENTER> at the end of the input)
		cin.clear();//clear any errors from the input to prevent breaking future prompts.
		//cin.ignore(65536,'\n');//flush any waiting data in the input buffer to prevent filling future prompts without user interaction
	}while(input.length()==0 && !allowblank);
    StringTo(input, output);//convert and save our input line as the desired field
}



int main(int argc, char* args[]){
    fstream file;// set up our the objects we need
    StockItem entry;
    char cExit;

	//begin the I/O operations
    file.open("inventory.txt", ios::in | ios::out | ios::app);//open our database/record-file for reading and writing in Append mode -- TODO: file error handling
    cout<<"Enter new stock item data below:"<<endl;
    StockItems_input(file);//prompt for each field of new StockItem records

    file.clear();//clear any file errors before continuing
    file.seekg(0,file.beg);//set the stream position to the beginning of the file to prepare for reading, reusing the same file stream

    StockItems_outputTable(file);//print a table of saved records and the final total
    file.close();//close the file, we're done reading and writing here


    PromptLineEntry("Press Enter to exit",cExit,true);//flush input and wait for the user to hit enter before closing the program
    return 0;
}

void StockItems_outputTable(fstream& file){
    StockItem entry;
    string record;
    float total=0.0f;
    cout<<"+----------------------+------------+----------------------+----------------------+"<<endl;//print our table header
    cout<<"| DESCRIPTION          | QTY        | UNIT COST            | TOTAL COST           |"<<endl;
    cout<<"|----------------------+------------+----------------------+----------------------|"<<endl;
    while(getline(file,record)){//get a line at a time from our record file
        StockItem_unserialize(entry, record);//convert string records to structs (NOTE: saving binary records is possible, but not suggested due to implementation-defined struct alignment)
        total+=entry.totalCost;//accumulate our total cost as we iterate records
        StockItem_outputRow(entry);//output the stock record as a row in table format
    }
    cout<<"|----------------------+------------+----------------------+----------------------|"<<endl;//print our table footer
    cout<<"|                                     GRAND  TOTAL         | ";
	printPrice(total);
	cout<<" |"<<endl;//print our final total
    cout<<"+----------------------------------------------------------+----------------------+"<<endl;
}


void StockItems_input(fstream& file){
    StockItem entry;
    char yn;
    do{
        StockItem_input(entry);
        file<<StockItem_serialize(entry)<<endl;//serialize entry and append to file
        do{
                PromptLineEntry("Enter another record? (Y/N)",yn,false);//prompt for a char Y or N
                yn=toupper(yn);//make the input uppercase so that the entry is case-insensitive
        }while(yn!='Y' && yn!='N');//keep prompting for Y/N until the user enters a valid option
    }while(yn=='Y');//as long as the user enters Y, keep prompting entries
}

void StockItem_input(StockItem& entry){
    PromptLineEntry("   Item Description",entry.desc,false);
    PromptLineEntry("   Item Quantity (total units)",entry.quantity,false);
	if(entry.quantity<0) entry.quantity=0;
    cout<<"   Item Quantity (total units): "<<entry.quantity<<endl;
    PromptLineEntry("   Unit Cost (price per unit)",entry.unitCost,false);
	if(entry.unitCost<0.0f) entry.unitCost=0;
    cout<<"   Unit Cost (price per unit): ";
	printPrice(entry.unitCost);
	cout<<endl;
    entry.totalCost=entry.unitCost*entry.quantity;//calculate the total cost of the stock entry
    cout<<"   Total Stock Entry Cost: ";
	printPrice(entry.totalCost);
	cout<<endl;
}

void StockItem_outputRow(StockItem& entry){
    string desc=entry.desc;
    if(desc.length()>20){//trim descriptions longer than 20 chars
        desc=desc.substr(0,20-3)+"...";//suffix with ... to decode the trimmed text.
    }
    cout<<"| "<<setw(20)<<desc<<" | "<<setw(10)<<entry.quantity<<" | ";//output each stock item record field as a formatted width cell in the table
    printPrice(entry.unitCost);//prices need special formatting for decimal places
    cout<<" | ";
    printPrice(entry.totalCost);
    cout<<" | "<<endl;
}

string StockItem_serialize(StockItem& item){//covert struck fields to file record string line
    ostringstream stream;

	//record format is:
	// <qty> <cost> <total> <description string>
	//having the description LAST permits reading it in with spaces (NOTE: adding additional string fields will require more complicated parsing)

    stream<<" "<<item.quantity<<" "<<item.unitCost<<" "<<item.totalCost<<" "<<item.desc;//convert all the values quickly to a string using stringstream
    return stream.str();//read the entry stream to a string
}

void StockItem_unserialize(StockItem& item_out, string entry){
    stringstream stream;
    stream<<entry<<endl;//write file record string into stringstream
    stream>>item_out.quantity;//read+convert each record field out to their types (note: does some minor terminating validation)
    stream>>item_out.unitCost;
    stream>>item_out.totalCost;
    //after reading in the previous fields delimited by spaces, we have a string (with spaces) and a newline, to read in the entry thing we have to use getline():
	getline(stream,item_out.desc);
}

void printPrice(float f){
    streamsize z=cout.precision();//push the current precision for later
    cout.setf( std::ios::fixed, std:: ios::floatfield );//set fixed-decimal-point and float formatting flags
    cout.precision(2);//set the decimal precision to 2 places
    cout<<setw(20)<<f;//print our price in a fixed-width field
    cout.unsetf (ios::floatfield );//clear the flags we just set earlier to prevent formatting leak
    cout.unsetf (ios::fixed);//clear the flags we just set earlier to prevent formatting leak
    cout.precision(z);//pop the precision back to the earlier value
}

void StringTo(string s, string& data){ data=s; }//overload the template function for 'string' to prevent conversion (NOTE: allows spaces)
