/*
 * THIS SOFTWARE IS PROVIDED BY RAJESH JAYAPRAKASH ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL RAJESH JAYAPRAKASH OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CLASS_FILE_
#define _CLASS_FILE_

#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

using namespace std ;

#include "ziplib.h"


//this is a dummy exception class to
//trap errors in read_bytes()
//and return VERIFY_ERROR
class ReadException
{
} ;

class constant_pool
{
	public:
		static const int CONSTANT_Class	;
		static const int CONSTANT_Fieldref ;
		static const int CONSTANT_Methodref	;
		static const int CONSTANT_InterfaceMethodref ;
		static const int CONSTANT_String ;
		static const int CONSTANT_Integer ;
		static const int CONSTANT_Float	;
		static const int CONSTANT_Long ;
		static const int CONSTANT_Double ;
		static const int CONSTANT_NameAndType ;
		static const int CONSTANT_Utf8 ;

		bool valid_flag ;
		int tag ;
		int name_index ;
		int class_index ;
		int name_and_type_index ;
		int string_index ;
		int int_bytes ;
		int float_bytes[4] ;
		int long_high_bytes[4] ;
		int long_low_bytes[4] ;
		int double_high_bytes[4] ;
		int double_low_bytes[4] ;
		int descriptor_index ;
		int length ;
		vector<int> Utf8_bytes ;

		//code added 15/9/2005
		//to prevent uninitialised booleans
		//from screwing things up
		constant_pool()
		{
			valid_flag = false ;
		}
		//end of code added 15/9/2005

} ;


class exception_table
{
	public:
		int start_pc ;
		int end_pc ;
		int handler_pc ;
		int catch_type ;
} ;

class line_number_table
{
	public:
		int start_pc ;
		int line_number ;

} ;

class local_variable_table
{
	public:
		int start_pc ;
		int length ;
		int name_index ;
		int descriptor_index ;
		int index ;

} ;
class class_info
{
	public:
		int inner_class_info_index ;    
		int outer_class_info_index ;    
		int inner_name_index ;    
		int inner_class_access_flags ;

} ;

class attribute_info
{
	public:


		static const int SOURCE_FILE ;
		static const int CONSTANT_VALUE	;
		static const int CODE ;
		static const int EXCEPTIONS	;
		static const int INNER_CLASSES ;
		static const int LINE_NUMBER_TABLE ;
		static const int LOCAL_VARIABLE_TABLE ;
		static const int DEPRECATED	;
		static const int SYNTHETIC ;

		//common to all attributes
		int attribute_name_flag ;	//introduced for efficiency
		int attribute_name_index ;
		int attribute_length ;

		//SourceFile
		int source_file_index ;

		//Constantvalue
		int constantvalue_index ;

		//Code
		int max_stack ;
		int max_locals ;
		int code_length ;
		vector<int> code ;
		int exception_table_length ;
		vector<exception_table *> ex_tbl ;
		int attributes_count ;
		vector<attribute_info *> attributes ;

		//Exceptions
		int number_of_exceptions ;
		vector<int> exception_index_table ;

		//InnerClasses
		int number_of_classes;
		vector<class_info *> classes ;

		//LineNumberTable
		int line_number_table_length ;
		vector<line_number_table *> lnt_info ;

		//LocalVariableTable
		int local_variable_table_length ;
		vector<local_variable_table *> lvt_info ;

		//Deprecated and Synthetic attributes do not require
		//additional fields (only attribute_name_index
		//and attribute_length)

		//for other attributes
		vector<int> info ;

		~attribute_info()
		{
			//free memory associated with ex_tbl vector
			for(int i=0; i < ex_tbl.size(); i++)
				delete ex_tbl.at(i) ;

			for(int i=0; i < attributes.size(); i++)
				delete attributes.at(i) ;

			for(int i=0; i < classes.size(); i++)
				delete classes.at(i) ;

			for(int i=0; i < lnt_info.size(); i++)
				delete lnt_info.at(i) ;

			for(int i=0; i < lvt_info.size(); i++)
				delete lvt_info.at(i) ;
		}


} ;


class field_info
{
	public:
		int access_flags ;
		int name_index ;
		int descriptor_index ;
		int attributes_count ;
		vector<attribute_info *> attributes ;

		~field_info()
		{
			for(int i=0; i<attributes.size(); i++)
				delete attributes.at(i) ;
		}
} ;

class method_info
{
	public:
		int access_flags ;
		int name_index ;
		int descriptor_index ;
		int attributes_count ;
		vector<attribute_info *> attributes ;

		~method_info()
		{
			for(int i=0; i<attributes.size(); i++)
				delete attributes.at(i) ;
		}

} ;

/** commented out because we are including zipfile.cpp
directly in the project, as compared to using the ziplib.lib
static libary in Windows


//this class is used in the extraction
//of class files from JAR files
class ZipFileEntry {
 private:
    static const unsigned short HAS_DATA_DESCRIPTOR = 0x8;
    static const unsigned int METHOD_UNCOMPRESSED = 0;
    static const unsigned int METHOD_DEFLATED = 8;

    void parse();
    int doInflate(char *dest);
    
 protected:
    string _name;
    string _crc;
    string _extra;
    short _method;
    string _comment;
    size_t _size;
    size_t _csize; 		// compressed size
    long _time;
    int _lhoff;			// local header off
    ifstream &_is;
    bool _initialized;
    off_t _off;
    char *_cdata;
    char *_data;

 public:
    ZipFileEntry(string& name, ifstream& input, int lhoff, int csize);
    ~ZipFileEntry();

    const string& getComment() const { return _comment;};
    const string& getCrc() const { return _crc; };
    const string& getExtra() const { return _extra; };
    const short getMethod() const { return _method;};
    const string& getName() const { return _name;};
    size_t getSize() const { return _size; };
    long getTime() const { return _time; };
    bool isDirectory() const { return _name.find_last_of("/") == _name.length()-1;};

    // Get the uncompressed data for this file
    char *getData();
};

**/

class ClassFile
{

	public:

		static const int NO_CLASS_DEF_FOUND ;
		static const int FORMAT_ERROR ;
		static const int VERIFY_ERROR ;

		ClassFile(const ClassFile&) ;
		ClassFile() ;
		~ClassFile() ;

		ClassFile& operator=(const ClassFile&) ;

		//code changed 6/8/2005
		//void load(string class_name) ;
		void load(string class_name, string classpath) ;
		//end of code changed 6/8/2005
	
		string get_class_name() ;
		string get_superclass_name() ;
		string get_parent_class_name(int) ;
		string get_symbol_name(int) ;
	
		int magic ;
		int minor_version ;
		int major_version ;
		int constant_pool_count ;
		vector<constant_pool *> cp ;
		int access_flags ;
		int this_class ;
		int super_class ;
		int interfaces_count ;
		vector<int> interfaces ;
		int fields_count ;
		vector<field_info *> fi ;
		int methods_count ;
		vector<method_info *> mi ;
		int attributes_count ;
		vector<attribute_info *> ai ;

		//this method is for loading a class
		//directly from a char buffer
		//code changed 16/4/2004
		//as part of refactoring the two load() methods
		//void load(char *, int) ;	
		void load(char *, int, string) ;	
		//end of code changed 16/5/2004

	private:

		int bytes_read ;

		int ptr ;	//identifies the next character to be
				//read from the data array


		char *data ;	//this contains the data obtained from
				//extracting the class file from the JAR file



		int size ;	//size of the extracted data array


		FILE *out ;



		int read_bytes(int n) ;
		attribute_info * populate_attribute_info() ;
		//void free_attribute_memory(attribute_info *a) ;
		//void free_memory() ;
		//attribute_info *copy_attribute(attribute_info *a) ;
		
} ;
#endif
