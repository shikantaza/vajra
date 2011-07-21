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

#include "ClassFile.h"

#include "util.h"
#include "logger.h"
#include "exceptions.h"

#include <iostream>
#include <cassert>

const int constant_pool::CONSTANT_Class	= 7 ;
const int constant_pool::CONSTANT_Fieldref = 9 ;
const int constant_pool::CONSTANT_Methodref	= 10 ;
const int constant_pool::CONSTANT_InterfaceMethodref = 11 ;
const int constant_pool::CONSTANT_String = 8 ;
const int constant_pool::CONSTANT_Integer =	3 ;
const int constant_pool::CONSTANT_Float	= 4 ;
const int constant_pool::CONSTANT_Long = 5 ;
const int constant_pool::CONSTANT_Double = 6 ;
const int constant_pool::CONSTANT_NameAndType =	12 ;
const int constant_pool::CONSTANT_Utf8 = 1 ;

const int attribute_info::SOURCE_FILE =	1 ;
const int attribute_info::CONSTANT_VALUE = 2 ;
const int attribute_info::CODE = 3 ;
const int attribute_info::EXCEPTIONS = 4 ;
const int attribute_info::INNER_CLASSES	= 5 ;
const int attribute_info::LINE_NUMBER_TABLE	= 6 ;
const int attribute_info::LOCAL_VARIABLE_TABLE = 7 ;
const int attribute_info::DEPRECATED = 8 ;
const int attribute_info::SYNTHETIC = 9 ;


const int ClassFile::NO_CLASS_DEF_FOUND = 1 ;
const int ClassFile::FORMAT_ERROR = 2 ;
const int ClassFile::VERIFY_ERROR = 3 ;

//code added 24/5/2004, changed 6/8/2005
//extern string classpath ;
//end of code added 24/5/2004, changed 6/8/2005

//code added 14/11/2004
extern bool blnOutfileInitialised ;
//end of code added 14/11/2004


string ClassFile::get_symbol_name(int name_index)
{

#if DEBUG_LEVEL >= 3
	assert(name_index > 0 && name_index < constant_pool_count) ;

	if(cp.at(name_index-1)->tag != constant_pool::CONSTANT_Utf8)
	{
		printf("%d %d\n", name_index, cp.at(name_index-1)->tag) ;
		assert(false) ;
	}
#endif

	int i ;

	string ret = "" ;
	
	for(i=0 ; i<cp.at(name_index-1)->length ; i++)
		ret += cp.at(name_index-1)->Utf8_bytes.at(i) ;

	//looks like some Utf8 items can have zero length
	//(e.g. java.lang.Class)
	/*
	if(ret.length() == 0)
	{
		cout << "get_symbol_name() failed" << "\n" ;
		exit(1) ;
	}
	*/
	//

	return ret ;
}

int ClassFile::read_bytes(int n)
{

#if DEBUG_LEVEL >= 3
	assert(n > 0 && n <= 4) ;
#endif
	int retval = 0 ;

	bytes_read += n ;

	char s[20] ;

	unsigned char c1, c2[2], c4[4] ;
	
	switch(n)
	{

		case 1:
			if(n == 1 && ptr > size-1) throw ReadException() ;
			c1 = data[ptr] ;
			ptr++ ;
			sprintf(s,"%02X", c1) ;
			break ;
		case 2:
			if(n == 2 && ptr > size-2) throw ReadException() ;
			c2[0] = data[ptr] ; c2[1] = data[ptr+1] ;
			ptr += 2 ;
			sprintf(s,"%02X%02X", c2[0], c2[1]) ;
			break ;
		case 4:
			if(n == 4 && ptr > size-4) throw ReadException() ;
			c4[0] = data[ptr] ; c4[1] = data[ptr+1] ;
			c4[2] = data[ptr+2] ; c4[3] = data[ptr+3] ;
			ptr += 4 ;
			sprintf(s,"%02X%02X%02X%02X", c4[0],c4[1], c4[2], c4[3]) ;
			break ;
	} //end switch

	retval = util::convert_from_hex(s) ;

	return retval ;

}

attribute_info * ClassFile::populate_attribute_info()
{

#if DEBUG_LEVEL >= 4
        //chokes for some reason
	//FunctionCallLogger fcl("ClassFile::populate_attribute_info") ;
#endif

	int i ;

	attribute_info *a = new attribute_info() ;

	string buf ;
	
	a->attribute_name_index = read_bytes(2) ;

#if DEBUG_LEVEL >= 3
	assert(a->attribute_name_index > 0 && a->attribute_name_index < constant_pool_count) ;
#endif
	util::debugprint(out,"attribute_name_index = %d\n",a->attribute_name_index) ;

	a->attribute_length = read_bytes(4) ;

	util::debugprint(out,"attribute_length = %d\n",a->attribute_length) ;
	
	buf = get_symbol_name(a->attribute_name_index) ;

#if DEBUG_LEVEL >= 3
	assert(buf.length() > 0) ;
#endif

	//code added 28/2/2005
	a->attribute_name_flag = -1 ;
	//end of code added 28/2/2005

	if(buf == "SourceFile")
		a->attribute_name_flag = attribute_info::SOURCE_FILE ;

	if(buf == "ConstantValue")
		a->attribute_name_flag = attribute_info::CONSTANT_VALUE ;

	if(buf == "Code")
		a->attribute_name_flag = attribute_info::CODE ;

	if(buf == "Exceptions")
		a->attribute_name_flag = attribute_info::EXCEPTIONS ;

	if(buf == "InnerClasses")
		a->attribute_name_flag = attribute_info::INNER_CLASSES ;

	if(buf == "LineNumberTable")
		a->attribute_name_flag = attribute_info::LINE_NUMBER_TABLE ;

	if(buf == "LocalVariableTable")
		a->attribute_name_flag = attribute_info::LOCAL_VARIABLE_TABLE ;

	if(buf == "Deprecated")
		a->attribute_name_flag = attribute_info::DEPRECATED ;

	if(buf == "Synthetic")
		a->attribute_name_flag = attribute_info::SYNTHETIC ;

	//code added 28/2/2005
#if DEBUG_LEVEL >= 3
	//"EnclosingMethod" attribute seems to be a Classpath attribute
	if(buf != "EnclosingMethod" && a->attribute_name_flag == -1)
	  assert(false) ;
#endif
	//end of code added 28/2/2005

	if(a->attribute_name_flag == attribute_info::SOURCE_FILE)
	{
		a->source_file_index = read_bytes(2) ;
		util::debugprint(out,"source_file_index = %d\n",a->source_file_index) ;
	}

	else if(a->attribute_name_flag == attribute_info::CONSTANT_VALUE)
	{
		a->constantvalue_index = read_bytes(2) ;
		util::debugprint(out,"constantvalue_index = %d\n",a->constantvalue_index) ;
	}

	else if(a->attribute_name_flag == attribute_info::CODE)
	{
		a->max_stack = read_bytes(2) ;
		a->max_locals = read_bytes(2) ;
		a->code_length = read_bytes(4) ;

		util::debugprint(out,"max_stack = %d\n",a->max_stack) ;
		util::debugprint(out,"max_locals = %d\n",a->max_locals) ;
		util::debugprint(out,"code_length = %d\n",a->code_length) ;

		a->code.reserve(a->code_length) ;

		util::debugprint(out,"code:") ;

		for(i=0 ; i<a->code_length ; i++)
		{
			a->code.insert(a->code.begin() + i,read_bytes(1)) ;
			util::debugprint(out,"%d ",a->code.at(i)) ;
		}

		util::debugprint(out,"\n") ;

		a->exception_table_length = read_bytes(2) ;

		util::debugprint(out,"exception_table_length = %d\n",a->exception_table_length) ;

		a->ex_tbl.reserve(a->exception_table_length) ;

		if(a->exception_table_length > 0)
			util::debugprint(out,"exception_table:\n") ;			

		for(i=0; i<a->exception_table_length; i++)
		{

			exception_table *e = new exception_table() ;

			e->start_pc = read_bytes(2) ;
			e->end_pc = read_bytes(2) ;
			e->handler_pc = read_bytes(2) ;
			e->catch_type = read_bytes(2) ;

			util::debugprint(out,"start_pc = %d ",e->start_pc) ;
			util::debugprint(out,"end_pc = %d ",e->end_pc) ;
			util::debugprint(out,"handler_pc = %d ",e->handler_pc) ;
			util::debugprint(out,"catch_type = %d\n",e->catch_type) ;

			a->ex_tbl.insert(a->ex_tbl.begin() + i, e) ;
		}


		a->attributes_count = read_bytes(2) ;
		util::debugprint(out,"attributes_count = %d\n",a->attributes_count) ;

		a->attributes.reserve(a->attributes_count) ;

		if(a->attributes_count > 0)
			util::debugprint(out,"attributes:\n") ;

		for(i=0; i<a->attributes_count; i++)
			a->attributes.insert(a->attributes.begin() + i, populate_attribute_info()) ;

	}

	else if(a->attribute_name_flag == attribute_info::EXCEPTIONS)
	{
		a->number_of_exceptions = read_bytes(2) ;
		util::debugprint(out,"number_of_exceptions = %d\n",a->number_of_exceptions) ;

		a->exception_index_table.reserve(a->number_of_exceptions) ;

		util::debugprint(out,"exception_index_table:\n") ;
		for(i=0; i<a->number_of_exceptions; i++)
		{
			a->exception_index_table.insert(a->exception_index_table.begin() + i,read_bytes(2)) ;
			util::debugprint(out,"%d",a->exception_index_table.at(i)) ;
		}
		util::debugprint(out,"\n") ;

	}

	else if(a->attribute_name_flag == attribute_info::INNER_CLASSES)
	{
		a->number_of_classes = read_bytes(2) ;
		util::debugprint(out,"number_of_classes = %d\n",a->number_of_classes) ;

		a->classes.reserve(a->number_of_classes) ;
		
		util::debugprint(out,"classes:\n") ;
		for(i=0; i<a->number_of_classes; i++)
		{
			class_info *c = new class_info() ;

			c->inner_class_info_index = read_bytes(2) ;
			c->outer_class_info_index = read_bytes(2) ;
			c->inner_name_index = read_bytes(2) ;
			c->inner_class_access_flags = read_bytes(2) ;

			util::debugprint(out,"inner_class_info_index = %d\n",c->inner_class_info_index) ;
			util::debugprint(out,"outer_class_info_index = %d\n",c->outer_class_info_index) ;
			util::debugprint(out,"inner_name_index = %d\n",c->inner_name_index) ;
			util::debugprint(out,"inner_class_access_flags = %02X\n",c->inner_class_access_flags) ;

			a->classes.insert(a->classes.begin() + i, c) ;

		}

	}

	else if(a->attribute_name_flag == attribute_info::LINE_NUMBER_TABLE)
	{
		a->line_number_table_length = read_bytes(2) ;
		util::debugprint(out,"line_number_table_length = %d\n",a->line_number_table_length) ;

		a->lnt_info.reserve(a->line_number_table_length) ;

		util::debugprint(out,"line_number_table info:\n") ;

		for(i=0; i<a->line_number_table_length; i++)
		{
			line_number_table *l = new line_number_table() ;

			l->start_pc = read_bytes(2) ;
			l->line_number = read_bytes(2) ;
			util::debugprint(out,"start_pc = %d\n",l->start_pc) ;
			util::debugprint(out,"line_number = %d\n",l->line_number) ;

			a->lnt_info.insert(a->lnt_info.begin() + i, l) ;
		}

	}

	else if(a->attribute_name_flag == attribute_info::LOCAL_VARIABLE_TABLE)
	{
		a->local_variable_table_length = read_bytes(2) ;
		util::debugprint(out,"local_variable_table_length = %d\n",a->local_variable_table_length) ;

		a->lvt_info.reserve(a->local_variable_table_length) ;

		util::debugprint(out,"local_variable_table info:\n") ;

		for(i=0; i<a->local_variable_table_length; i++)
		{

			local_variable_table *l = new local_variable_table() ;

			l->start_pc = read_bytes(2) ;
			l->length = read_bytes(2) ;
			l->name_index = read_bytes(2) ;
			l->descriptor_index = read_bytes(2) ;
			l->index = read_bytes(2) ;

			util::debugprint(out,"start_pc = %d\n",l->start_pc) ;
			util::debugprint(out,"length = %d\n",l->length) ;
			util::debugprint(out,"name_index = %d\n",l->name_index) ;
			util::debugprint(out,"descriptor_index = %d\n",l->descriptor_index) ;
			util::debugprint(out,"index = %d\n",l->index) ;


			a->lvt_info.insert(a->lvt_info.begin() + i, l) ;
		}

	}

	else if(a->attribute_name_flag == attribute_info::DEPRECATED ||
		    a->attribute_name_flag == attribute_info::SYNTHETIC)
	{
		//do nothing
		;
	}


	else
	{

		if(a->attribute_length > 0)
		{
			//commented out because Classpath
			//has an extra attribute called EnclosingMethod
			//which has zero bytes of information
			//though attribute_length is set to 4

			//a->info.reserve(a->attribute_length) ;

			//util::debugprint(out,"default info:\n") ;
			//for(i=0; i<a->attribute_length; i++)
			//{
			//	a->info.at(i) = read_bytes(1) ;
			//	util::debugprint(out,"%d",a->info.at(i)) ;
			//}
			//util::debugprint(out,"\n") ;
		}
	}

	return a ;

}

ClassFile::ClassFile()
	:bytes_read(0),ptr(0),data(NULL)
{
	//dummy constructor; required for satisying pair template
}

//assignment operator
ClassFile& ClassFile::operator=(const ClassFile& rhs)
{

	if(this == &rhs)
		return *this ;

	magic = rhs.magic;
	minor_version = rhs.minor_version;
	major_version = rhs.major_version;
	constant_pool_count = rhs.constant_pool_count;

	cp = rhs.cp ;
	
	access_flags = rhs.access_flags;
	this_class = rhs.this_class;
	super_class = rhs.super_class;
	interfaces_count = rhs.interfaces_count;

	interfaces = rhs.interfaces ;

	fields_count = rhs.fields_count;

	fi = rhs.fi ;

	methods_count = rhs.methods_count;

	mi = rhs.mi ;

	attributes_count = rhs.attributes_count;

	ai = rhs.ai ;	

	out = rhs.out;

	return *this ;
}

ClassFile::ClassFile(const ClassFile& rhs)
{

	magic = rhs.magic;
	minor_version = rhs.minor_version;
	major_version = rhs.major_version;
	constant_pool_count = rhs.constant_pool_count;

	cp = rhs.cp ;
	
	access_flags = rhs.access_flags;
	this_class = rhs.this_class;
	super_class = rhs.super_class;
	interfaces_count = rhs.interfaces_count;

	interfaces = rhs.interfaces ;

	fields_count = rhs.fields_count;

	fi = rhs.fi ;

	methods_count = rhs.methods_count;

	mi = rhs.mi ;

	attributes_count = rhs.attributes_count;

	ai = rhs.ai ;	

	out = rhs.out;

}

void ClassFile::load(string class_name, string classpath)
{

	int i, j ;

#if DEBUG_LEVEL >= 3
	assert(class_name.length() > 0) ;
#endif
#if DEBUG_LEVEL >= 4
	
	//code added 11/11/2004
	//Logger::LogFunctionEntry("ClassFile::load(string): " + class_name) ;
	FunctionCallLogger fcl("ClassFile::load(string): " + class_name) ;
	//end of code added 11/11/2004

	//Logger::Log("loading class " + class_name) ; ;
#endif

	string class_name_with_ext = class_name + ".class" ;

	//code changed 24/5/2004
	//this is now being handled in java.cpp
	//string classpath = getenv("CLASSPATH") ;
	//end of code changed 24/5/2004

	if(classpath.length() == 0)
	{
		//code added 11/11/2004
		//Logger::LogFunctionExit("ClassFile::load(string) - NoClassDefFound 1") ;
		//end of code added 11/11/2004		
		throw NoClassDefFound() ;		
	}

#ifdef WIN32
	vector<string> *vcp = util::tokenise(classpath,';') ;
#else
        vector<string> *vcp = util::tokenise(classpath,':') ;
#endif
	bool blnClassFound = false ;

	for(i=0; i<vcp->size() && blnClassFound==false ; i++)
	{
		if(vcp->at(i) == ".")
		{
			ifstream f(class_name_with_ext.data(), ios_base::binary) ;

			if(f) //file exists in current directory
			{
				vector<char> v ;

				char c ;
				while(f.get(c))
					v.push_back(c) ;

				f.close() ;

				size = v.size() ;

				data = new char[size] ;

				for(int i=0; i<v.size(); i++)
					data[i] = v.at(i) ;

				blnClassFound = true ;

			} //end of if(f)
		} //end of if(vcp->...
		else if(vcp->at(i).substr(vcp->at(i).length()-4,4) == ".jar" ||
			vcp->at(i).substr(vcp->at(i).length()-4,4) == ".zip")
		{

		  //code added 13/5/2004 to handle non-existent jar/zip files in CLASSPATH
		  ifstream f1(vcp->at(i).data(),ios_base::binary) ;

		  if(!f1) //i.e. file doesn't exist
		    continue ;
		  // end of code added 13/5/2004

			ZipFile *z = new JarFile(vcp->at(i).data()) ;

			list<ZipFileEntry *> v = z->entries();

			list<ZipFileEntry *>::iterator it;

			for (it = v.begin(); it != v.end() && blnClassFound==false; it++)
			{
				ZipFileEntry *ze = *it;

				if(ze->getName() == class_name_with_ext)
				{
				  data = ze->getData() ;
				  size = ze->getSize() ;
				  blnClassFound = true ;
				}

				//TO DO: for some reason, deleting ze crashes the program
				//delete ze ;
			}
			//TO DO: for some reason, deleting z crashes the program
			//delete z ;

		} //end of else if
		else
		{
		  //we assume that the directory name will not end with a "/"
		  string class_name_with_full_path = vcp->at(i) + "/" + class_name_with_ext ;
		  ifstream f(class_name_with_full_path.data(), ios_base::binary) ;

		  if(f) //file exists in a directory in the class path
		  {
		    vector<char> v ;

		    char c ;
		    while(f.get(c))
		      v.push_back(c) ;

		    f.close() ;

		    size = v.size() ;

		    data = new char[size] ;

		    for(int i=0; i<v.size(); i++)
		      data[i] = v.at(i) ;

		    blnClassFound = true ;
		  }

		}
	} //end of for

	delete vcp ;

	if(blnClassFound == false)
	{
		//code added 11/11/2004
		//Logger::LogFunctionExit("ClassFile::load(string) - NoClassDefFound 2") ;
		//end of code added 11/11/2004		
		throw NoClassDefFound() ;
	}

	//code changed 16/5/2004
	//as part of refactoring the two load methods
	//(a lot of code has been moved to the other load() method
	//to improve readability, this code has been physically 
	//removed instead of being just commented out
	try
	  {
	    load(data, size, class_name) ;
	  }
	catch(...)
	  {
	    //code added 11/11/2004
	    //Logger::LogFunctionExit("ClassFile::load(string) - unspecified error") ;
	    //end of code added 11/11/2004	  	
	    throw ;
	  }
	//end of code changed 16/5/2004

#if DEBUG_LEVEL >= 3
	//Logger::Log("loaded class " + class_name) ;

	//code added 11/11/2004
	//Logger::LogFunctionExit("ClassFile::load(string): " + class_name) ;
	//end of code added 11/11/2004
	
	
#endif
	return ;
}

ClassFile::~ClassFile()
{
	//free_memory() ;

	for(int i=0; i < cp.size(); i++)
		delete cp.at(i) ;

	for(int i=0; i < fi.size(); i++)
		delete fi.at(i) ;

	for(int i=0; i < mi.size(); i++)
		delete mi.at(i) ;

	for(int i=0; i < ai.size(); i++)
		delete ai.at(i) ;

        if(data != NULL)
                delete data ;
}

string ClassFile::get_class_name()
{

	string ret ;

	for(int i=0;i < cp.at(cp.at(this_class - 1)->name_index - 1)->length; i++)
		ret += cp.at(cp.at(this_class - 1)->name_index - 1)->Utf8_bytes.at(i) ;

	return ret ;

}

string ClassFile::get_superclass_name()
{

	string ret ;

	for(int i=0;i < cp.at(cp.at(super_class - 1)->name_index - 1)->length; i++)
		ret += cp.at(cp.at(super_class - 1)->name_index - 1)->Utf8_bytes.at(i) ;

	return ret ;

}

string ClassFile::get_parent_class_name(int index)
{

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index < constant_pool_count-1) ;

	assert(cp.at(index)->tag == constant_pool::CONSTANT_Fieldref ||
	       cp.at(index)->tag == constant_pool::CONSTANT_InterfaceMethodref ||
	       cp.at(index)->tag == constant_pool::CONSTANT_Methodref) ;
#endif

	int class_index = cp.at(index)->class_index ;
	int name_of_class_index = cp.at(class_index-1)->name_index ;
	return(get_symbol_name(name_of_class_index)) ;

}

//this method does the same thing as load(string), but 
//starts with the buffer containing the .class file buffer
//we need to make load(string) call this method internally
//code changed 16/5/2004
//as part of refactoring the two load methods
//a) when this load() method is called from the other load() method,
//   the class_name parameter will be populated with the name of the class
//   that the other load() method has been asked to load
//b) when it is called from DefineClass(), the 'class_name'
//   parameter will be empty
// void ClassFile::load(char *buf, int len)
void ClassFile::load(char *buf, int len, string class_name)
//end of code changed 16/5/2004
{

//code added 11/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("ClassFile::load(char *, int, string):" + class_name) ;
	FunctionCallLogger fcl("ClassFile::load(char *, int, string):" + class_name) ;
#endif
//end of code added 11/11/2004

	int i, j ;
	
	//code changed 16/5/2004 
	//if this load method is being called from the
	//other load method, no need to populate the 
	//'data' buffer
	if(buf != data)
	  {

	    data = new char[len] ;

	    for(i=0; i<len; i++)
	      data[i] = *(buf + i) ;
	  }
	//end of code changed 16/5/2004


	//16/5/2004: the original code that existed below
	//has been completely replaced by the code moved
	//from the other load() method.
	//to maintain readability, the old code has not
	//been simply commented out (but knocked off altogether)

#if DEBUG_LEVEL >= 3

	//code changed 14/11/2004
	//out = fopen("vajra_2.log","a") ;

	if(blnOutfileInitialised == false)
	{
		out = fopen("vajra_2.log", "w") ;		
		blnOutfileInitialised = true ;
	}
	else
		out = fopen("vajra_2.log", "a") ;
	//end of code changed 14/11/2004
	
#endif
	//coded added 11/11/2004
	if(class_name.length() == 0)
		util::debugprint(out,"----------------Begin load class: %s----------------\n",buf) ;
	else
		util::debugprint(out,"----------------Begin load class: %s----------------\n",class_name.data()) ;	
	//end of code added 11/11/2004

	try {	

		magic = read_bytes(4) ;

#ifdef WIN32
		if(magic != -889275714) //this is 0xCAFEBABE
#else
		if(magic != -1963017538) //this is 0xCAFEBABE
#endif
		{

#if DEBUG_LEVEL >= 3
			fclose(out) ;
#endif

			Logger::Log("Magic number wrong") ;
//code added 11/11/2004
#if DEBUG_LEVEL >= 3
			//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
#endif
//end of code added 11/11/2004
			throw FormatError() ;
		}

		minor_version = read_bytes(2) ;
		major_version = read_bytes(2) ;
		constant_pool_count = read_bytes(2) ;

		util::debugprint(out,"magic: %d\n",magic) ;
		util::debugprint(out,"minor version: %d\n",minor_version) ;
		util::debugprint(out,"major version: %d\n",major_version) ;
		util::debugprint(out,"constant_pool_count: %d\n",constant_pool_count) ;

		cp.reserve(constant_pool_count-1) ;

		util::debugprint(out,"constant pool:\n") ;

		bool last_entry_was_long_or_double = false ;

		for(i=0; i < constant_pool_count - 1; i++)
		{
			constant_pool *c = new constant_pool() ;

			if(last_entry_was_long_or_double == true)
			{
				last_entry_was_long_or_double = false ;
				c->valid_flag = false ;
				cp.insert(cp.begin()+i,c) ;
				continue ;
			}

			c->valid_flag = true ;

			c->tag = read_bytes(1) ;

#if DEBUG_LEVEL >= 3
			assert(c->tag == constant_pool::CONSTANT_Class ||
			   c->tag == constant_pool::CONSTANT_Fieldref ||
			   c->tag == constant_pool::CONSTANT_Methodref ||
			   c->tag == constant_pool::CONSTANT_InterfaceMethodref ||
			   c->tag == constant_pool::CONSTANT_String ||
			   c->tag == constant_pool::CONSTANT_Integer ||
			   c->tag == constant_pool::CONSTANT_Float	||
			   c->tag == constant_pool::CONSTANT_Long ||
			   c->tag == constant_pool::CONSTANT_Double ||
			   c->tag == constant_pool::CONSTANT_NameAndType ||
			   c->tag == constant_pool::CONSTANT_Utf8) ;
#endif
		
			util::debugprint(out,"%d. ",i+1) ;

			if(c->tag == constant_pool::CONSTANT_Class)
			{
				c->name_index = read_bytes(2) ;
#if DEBUG_LEVEL >= 3
				assert(c->name_index > 0 && c->name_index < constant_pool_count) ;

				if(c->name_index < i+1)
				{
					if(cp.at(c->name_index-1)->tag 
						!= constant_pool::CONSTANT_Utf8)
					{
						printf("%d %d\n", i+1, c->name_index) ;
						//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
						//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				util::debugprint(out,"CONSTANT_Class\n") ;
				util::debugprint(out,"name_index = %d\n", c->name_index) ;
			}
			else if(c->tag == constant_pool::CONSTANT_Fieldref)
			{
				c->class_index = read_bytes(2) ;
#if DEBUG_LEVEL >= 3
				assert(c->class_index > 0 && c->class_index < constant_pool_count) ;

				if(c->class_index < i+1)
				{

					if(cp.at(c->class_index-1)->tag 
						!= constant_pool::CONSTANT_Class)
					{
						printf("%d %d\n", i+1, c->class_index) ;
						//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
						//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				c->name_and_type_index = read_bytes(2);
#if DEBUG_LEVEL >= 3
				assert(c->name_and_type_index > 0 && c->name_and_type_index < constant_pool_count) ;


				if(c->name_and_type_index < i+1)
				{

					if(cp.at(c->name_and_type_index-1)->tag 
						!= constant_pool::CONSTANT_NameAndType)
					{
						printf("%d %d\n", i+1, c->name_and_type_index) ;
						//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
						//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				util::debugprint(out,"CONSTANT_Fieldref\n") ;
				util::debugprint(out,"class_index = %d\n", c->class_index) ;
				util::debugprint(out,"name_and_type_index = %d\n", c->name_and_type_index) ;
			}
			else if(c->tag == constant_pool::CONSTANT_Methodref)
			{
				c->class_index = read_bytes(2) ;
#if DEBUG_LEVEL >= 3
				assert(c->class_index > 0 && c->class_index < constant_pool_count) ;

				if(c->class_index < i+1)
				{

					if(cp.at(c->class_index-1)->tag 
						!= constant_pool::CONSTANT_Class)
					{
						printf("%d %d\n", i+1, c->class_index) ;
//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				c->name_and_type_index = read_bytes(2);
#if DEBUG_LEVEL >= 3
				assert(c->name_and_type_index > 0 && c->name_and_type_index < constant_pool_count) ;

				if(c->name_and_type_index < i+1)
				{

					if(cp.at(c->name_and_type_index-1)->tag 
						!= constant_pool::CONSTANT_NameAndType)
					{
						printf("%d %d\n", i+1, c->name_and_type_index) ;
						//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
						//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				util::debugprint(out,"CONSTANT_Methodref\n") ;
				util::debugprint(out,"class_index = %d\n", c->class_index) ;
				util::debugprint(out,"name_and_type_index = %d\n", c->name_and_type_index) ;
			}
			else if(c->tag == constant_pool::CONSTANT_InterfaceMethodref)
			{
				c->class_index = read_bytes(2) ;
#if DEBUG_LEVEL >= 3
				assert(c->class_index > 0 && c->class_index < constant_pool_count) ;

				if(c->class_index < i+1)
				{

					if(cp.at(c->class_index-1)->tag 
						!= constant_pool::CONSTANT_Class)
					{
						printf("%d %d\n", i+1, c->class_index) ;
						//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
						//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				c->name_and_type_index = read_bytes(2);
#if DEBUG_LEVEL >= 3
				assert(c->name_and_type_index > 0 && c->name_and_type_index < constant_pool_count) ;

				if(c->name_and_type_index < i+1)
				{

					if(cp.at(c->name_and_type_index-1)->tag 
						!= constant_pool::CONSTANT_NameAndType)
					{
						printf("%d %d\n", i+1, c->name_and_type_index) ;
						//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
						//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				util::debugprint(out,"CONSTANT_InterfaceMethodref\n") ;
				util::debugprint(out,"class_index = %d\n", c->class_index) ;
				util::debugprint(out,"name_and_type_index = %d\n", c->name_and_type_index) ;
			}
			else if(c->tag == constant_pool::CONSTANT_String)
			{
				c->string_index = read_bytes(2) ;
#if DEBUG_LEVEL >= 3
				assert(c->string_index > 0 && c->string_index < constant_pool_count) ;

				//this assert commented out because it
				//doesn't seem necessary in Classpath
				//if(c->string_index < i+1)
				//{

				//	if(cp.at(c->string_index-1)->tag 
				//		!= constant_pool::CONSTANT_String)
				//	{
				//		printf("%d %d\n", i+1, c->string_index) ;
				//		assert(false) ;
				//	}
				//}
#endif
				util::debugprint(out,"CONSTANT_String\n") ;
				util::debugprint(out,"string_index = %d\n", c->string_index) ;
			}
			else if(c->tag == constant_pool::CONSTANT_Integer)
			{
				c->int_bytes = read_bytes(4) ;
				util::debugprint(out,"CONSTANT_Integer\n") ;
				util::debugprint(out,"bytes = %d\n", c->int_bytes) ;
			}
			else if(c->tag == constant_pool::CONSTANT_Float)
			{
				c->float_bytes[0] = read_bytes(1) ;
				c->float_bytes[1] = read_bytes(1) ;
				c->float_bytes[2] = read_bytes(1) ;
				c->float_bytes[3] = read_bytes(1) ;
				util::debugprint(out,"CONSTANT_Float\n") ;
				util::debugprint(out,"bytes = %d %d %d %d \n", c->float_bytes[0],c->float_bytes[1],c->float_bytes[2],c->float_bytes[3]) ;
			}
			else if(c->tag == constant_pool::CONSTANT_Long)
			{
			
				c->long_high_bytes[0] = read_bytes(1) ;
				c->long_high_bytes[1] = read_bytes(1) ;
				c->long_high_bytes[2] = read_bytes(1) ;
				c->long_high_bytes[3] = read_bytes(1) ;
			
				c->long_low_bytes[0] = read_bytes(1) ;
				c->long_low_bytes[1] = read_bytes(1) ;
				c->long_low_bytes[2] = read_bytes(1) ;
				c->long_low_bytes[3] = read_bytes(1) ;
			
	
				last_entry_was_long_or_double = true ;

				util::debugprint(out,"CONSTANT_Long\n") ;

				//TO DO: debugprint has some problems here;
				//high_bytes and low_bytes are printed wrongly
				//util::debugprint(out,"high_bytes = %d %d %d %d\n", c->long_high_bytes[0],c->long_high_bytes[1],c->long_high_bytes[2],c->long_high_bytes[3]) ;
				//util::debugprint(out,"low_bytes = %d %d %d %d\n", c->long_low_bytes[0],c->long_low_bytes[1],c->long_low_bytes[2],c->long_low_bytes[3]) ;
#if DEBUG_LEVEL >= 3
				fprintf(out,"high_bytes = %d %d %d %d\n", c->long_high_bytes[0],c->long_high_bytes[1],c->long_high_bytes[2],c->long_high_bytes[3]) ;
				fprintf(out,"low_bytes = %d %d %d %d\n", c->long_low_bytes[0],c->long_low_bytes[1],c->long_low_bytes[2],c->long_low_bytes[3]) ;
#endif

			}
			else if(c->tag == constant_pool::CONSTANT_Double)
			{
				c->double_high_bytes[0] = read_bytes(1) ;
				c->double_high_bytes[1] = read_bytes(1) ;
				c->double_high_bytes[2] = read_bytes(1) ;
				c->double_high_bytes[3] = read_bytes(1) ;

				c->double_low_bytes[0] = read_bytes(1) ;
				c->double_low_bytes[1] = read_bytes(1) ;
				c->double_low_bytes[2] = read_bytes(1) ;
				c->double_low_bytes[3] = read_bytes(1) ;

				last_entry_was_long_or_double = true ;

				util::debugprint(out,"CONSTANT_Double\n") ;
				util::debugprint(out,"high_bytes = %d %d %d %d\n", c->double_high_bytes[0],c->double_high_bytes[1],c->double_high_bytes[2],c->double_high_bytes[3]) ;
				util::debugprint(out,"low_bytes = %d %d %d %d\n", c->double_low_bytes[0],c->double_low_bytes[1],c->double_low_bytes[2],c->double_low_bytes[3]) ;

			}
			else if(c->tag == constant_pool::CONSTANT_NameAndType)
			{
				c->name_index = read_bytes(2) ;
#if DEBUG_LEVEL >= 3
				assert(c->name_index > 0 && c->name_index < constant_pool_count) ;

				if(c->name_index < i+1)
				{

					if(cp.at(c->name_index-1)->tag 
						!= constant_pool::CONSTANT_Utf8)
					{
						printf("%d %d\n", i+1, c->name_index) ;
						//code added 11/11/2004
						//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
						//end of code added 11/11/2004
						assert(false) ;
					}
				}
#endif
				c->descriptor_index = read_bytes(2) ;
#if DEBUG_LEVEL >= 3
				assert(c->descriptor_index > 0 && c->descriptor_index < constant_pool_count) ;
#endif
				util::debugprint(out,"CONSTANT_NameAndType\n") ;
				util::debugprint(out,"name_index = %d\n", c->name_index) ;
				util::debugprint(out,"descripter_index = %d\n", c->descriptor_index) ;
			}
			else if(c->tag == constant_pool::CONSTANT_Utf8)
			{
				c->length = read_bytes(2) ;
				util::debugprint(out,"CONSTANT_Utf8\n") ;
				util::debugprint(out,"length = %d\n", c->length) ;

				c->Utf8_bytes.reserve(c->length) ;

				util::debugprint(out,"bytes\n") ;
				for(j=0;j<c->length;j++)
				{
					c->Utf8_bytes.insert(c->Utf8_bytes.begin()+j,read_bytes(1)) ;
					util::debugprint(out,"%c",c->Utf8_bytes.at(j)) ;
				}
				util::debugprint(out,"\n") ;
			}

			cp.insert(cp.begin() + i, c) ;

		}

		access_flags = read_bytes(2) ;
		util::debugprint(out,"access_flags: 0x%04x\n",access_flags) ;

		this_class = read_bytes(2) ;
		util::debugprint(out,"this_class: %d\n",this_class) ;

		//verify that the class name as mentioned
		//in the class file is the same as the 
		//class file name
		if(get_symbol_name(cp.at(this_class-1)->name_index) != class_name)
		{

#if DEBUG_LEVEL >= 3
			fclose(out) ;
#endif
//code added 11/11/2004
#if DEBUG_LEVEL >= 3
			//Logger::LogFunctionExit("ClassFile::load(char *, int, string):" + class_name) ;
#endif
//end of code added 11/11/2004
			throw NoClassDefFound() ;
		}

		super_class = read_bytes(2) ;
		util::debugprint(out,"super_class: %d\n",super_class) ;

		interfaces_count = read_bytes(2) ;
		util::debugprint(out,"interfaces_count: %d\n",interfaces_count) ;

		interfaces.reserve(interfaces_count) ;

		util::debugprint(out,"interfaces:\n") ;
		for(i=0;i<interfaces_count;i++)
		{
			interfaces.insert(interfaces.begin() + i, read_bytes(2)) ;
			util::debugprint(out,"%d\n",interfaces.at(i)) ;
		}

		fields_count = read_bytes(2) ;
		util::debugprint(out,"fields_count: %d\n",fields_count) ;

		fi.reserve(fields_count) ;

		for(i=0;i<fields_count;i++)
		{

			field_info *f = new field_info() ;

			f->access_flags = read_bytes(2) ;
			util::debugprint(out,"access_flags: 0x%04x\n",f->access_flags) ;

			f->name_index = read_bytes(2) ;
			util::debugprint(out,"name_index: %d\n",f->name_index) ;

			f->descriptor_index = read_bytes(2) ;
			util::debugprint(out,"descriptor_index: %d\n",f->descriptor_index) ;

			f->attributes_count = read_bytes(2) ;

			f->attributes.reserve(f->attributes_count) ;

			for(j=0;j<f->attributes_count;j++)
				f->attributes.insert(f->attributes.begin() + j, populate_attribute_info()) ;
		
			fi.insert(fi.begin() + i, f) ;
		}

		methods_count = read_bytes(2) ;
		util::debugprint(out,"methods_count: %d\n",methods_count) ;

		mi.reserve(methods_count) ;

		for(i=0;i<methods_count;i++)
		{
		
			util::debugprint(out,"*******method #%d*******\n",i+1) ;
		
			method_info *m = new method_info() ;

			m->access_flags = read_bytes(2) ;
			util::debugprint(out,"access_flags: 0x%04x\n",m->access_flags) ;

			m->name_index = read_bytes(2) ;
			util::debugprint(out,"name_index: %d\n",m->name_index) ;

			m->descriptor_index = read_bytes(2) ;
			util::debugprint(out,"descriptor_index: %d\n",m->descriptor_index) ;

			m->attributes_count = read_bytes(2) ;
			util::debugprint(out,"attributes_count:%d\n",m->attributes_count) ;

			m->attributes.reserve(m->attributes_count) ;

			for(j=0;j<m->attributes_count;j++)
				m->attributes.insert(m->attributes.begin() + j, populate_attribute_info()) ;
			
			mi.insert(mi.begin() + i, m) ;

			util::debugprint(out,"***end of method #%d*******\n",i+1) ;
		}


		attributes_count = read_bytes(2) ;
		util::debugprint(out,"attributes_count: %d\n",attributes_count) ;

		ai.reserve(attributes_count) ;

		for(j=0;j<attributes_count;j++)
			ai.insert(ai.begin() + j, populate_attribute_info()) ;

		util::debugprint(out,"total_bytes read = %d\n",bytes_read) ;

	//code added 11/11/2004
	if(class_name.length() == 0)
		util::debugprint(out,"----------------End load class: %s----------------\n",buf) ;
	else
		util::debugprint(out,"----------------End load class: %s----------------\n",class_name.data()) ;	
	//end of code added 11/11/2004

#if DEBUG_LEVEL >= 3
		fclose(out) ;
#endif

	}
	catch(ReadException)
	{

#if DEBUG_LEVEL >= 3
		fclose(out) ;
#endif

//code added 11/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
#endif
//end of code added 11/11/2004

		throw FormatError() ;
	}

	//this is to verify that the class file
	//does not contain any extra characters
	if(bytes_read != size)
	{
		
		//commented out because of Classpath quirkiness
		//wrt EnclosingMethod attribute
		
//code added 11/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
#endif
//end of code added 11/11/2004		
		//throw FormatError() ;
	}

//code added 11/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("ClassFile::load(char *, int, string)") ;
#endif
//end of code added 11/11/2004

	return ;


}
