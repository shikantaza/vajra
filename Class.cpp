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

#include <string>
#include <iostream>
#include <assert.h>
#include <bitset>

#include "vajra.h"
#include "util.h"

const int runtime_constant_pool::CLASS = 1 ;
const int runtime_constant_pool::INTERFACE = 2 ;
const int runtime_constant_pool::FIELD = 3 ;
const int runtime_constant_pool::CLASS_METHOD = 4 ;
const int runtime_constant_pool::INTERFACE_METHOD = 5 ;
const int runtime_constant_pool::STRING_LITERAL = 6 ;
const int runtime_constant_pool::INTEGER_CONSTANT = 7 ;
const int runtime_constant_pool::FLOAT_CONSTANT = 8 ;
const int runtime_constant_pool::LONG_CONSTANT = 9 ;
const int runtime_constant_pool::DOUBLE_CONSTANT = 10 ;


const int Class::PRIMITIVE_BYTE = 1;
const int Class::PRIMITIVE_CHAR = 2 ;
const int Class::PRIMITIVE_DOUBLE = 3 ;
const int Class::PRIMITIVE_FLOAT = 4 ;
const int Class::PRIMITIVE_INT = 5;
const int Class::PRIMITIVE_LONG = 6 ;
const int Class::PRIMITIVE_SHORT = 7 ;
const int Class::PRIMITIVE_BOOLEAN = 8 ;

//code added 13/11/2004
#include "logger.h"
//end of coded added 13/11/2004

Class::Class()
{
	blnVerified = false ;
	blnPrepared = false ;
	blnInitialised = false ;

	//code changed 15/5/2004
	//native_library_handle = NULL ;
	//end of code changed 15/5/2004

	//code added 28/11/2004
	super_class = NULL ;
	//end of code added 28/11/2004
	
	//code added 16/2/2005
	blnArrayClass = false ;
	//end of code 16/2/2005

	//code added 9/8/2005
	class_object = NULL ;
	//end of code added 9/8/2005	

	//code added 9/8/2005
	_class_object = NULL ;
	//end of code added 9/8/2005
        
        //code added 26/8/2005
        blnInterface = false ;
        //end of code added 26/8/2005

	//code added 15/9/2005
	blnPublic = false ;
	blnFinal = false ;
	blnSuper = false ;
	blnAbstract = false ;
	blnPrimitiveArray = false ;
	blnPrimitiveClass = false ;
	blnVoid = false ;
	//end of code added 15/9/2005
}

//assignment operator
Class& Class::operator=(const Class& rhs)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("Class::operator=()") ;
	FunctionCallLogger fcl("Class::operator=()") ;
#endif
//end of code added 13/11/2004	

	if(this == &rhs)
		return *this ;

	name = rhs.name ;
	super_class_name = rhs.super_class_name ;

	super_class = rhs.super_class ;

	blnVerified = rhs.blnVerified ;
	blnPrepared = rhs.blnPrepared ;
	blnInitialised = rhs.blnInitialised ;

	blnPublic = rhs.blnPublic ;
	blnFinal = rhs.blnFinal ;
	blnSuper = rhs.blnSuper ;
	blnInterface = rhs.blnInterface ;
	blnAbstract = rhs.blnAbstract ;


	initiating_loader = rhs.initiating_loader ;
	defining_loader = rhs.defining_loader ;

	interfaces = rhs.interfaces ;

	constant_pool_count = rhs.constant_pool_count ;
	rcp = rhs.rcp;

	fields = rhs.fields ;
	methods = rhs.methods ;

	blnArrayClass = rhs.blnArrayClass ;

	blnPrimitiveArray = rhs.blnPrimitiveArray ; 

	nodims = rhs.nodims ;
	array_class_ref = rhs. array_class_ref ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("Class::Operator=()") ;
#endif
//end of code added 13/11/2004

	return *this ;
}

Class::Class(const Class& rhs)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("Class::Class(Class&)") ;
	FunctionCallLogger fcl("Class::Class(Class&)") ;
#endif
//end of code added 13/11/2004	

	name = rhs.name ;

	super_class_name = rhs.super_class_name ;

	super_class = rhs.super_class ;

	blnVerified = rhs.blnVerified ;
	blnPrepared = rhs.blnPrepared ;
	blnInitialised = rhs.blnInitialised ;

	blnPublic = rhs.blnPublic ;

	blnFinal = rhs.blnFinal ;
	blnSuper = rhs.blnSuper ;
	blnInterface = rhs.blnInterface ;
	blnAbstract = rhs.blnAbstract ;

	initiating_loader = rhs.initiating_loader ;
	defining_loader = rhs.defining_loader ;

	interfaces = rhs.interfaces ;

	constant_pool_count = rhs.constant_pool_count ;

	rcp = rhs.rcp;

	fields = rhs.fields ;
	methods = rhs.methods ;

	blnArrayClass = rhs.blnArrayClass ;

	blnPrimitiveArray = rhs.blnPrimitiveArray ; 

	nodims = rhs.nodims ;
	array_class_ref = rhs. array_class_ref ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("Class::Class(Class&)") ;
#endif
//end of code added 13/11/2004

}

Class::~Class()
{
	for(int i=0; i<cp.size() ; i++)
		delete cp.at(i) ;

	for(int i=0; i<rcp.size() ; i++)
		delete rcp.at(i) ;

	for(int i=0; i<fields.size() ; i++)
		delete fields.at(i) ;

	for(int i=0; i<interfaces.size() ; i++)
		delete interfaces.at(i) ;

	//code changed 17/11/2004
	//typedef map<method_key, method *>::const_iterator CI ;
	typedef map<string, method *>::const_iterator CI ;
	//end of code changed 17/11/2004

	for(CI p=methods.begin(); p!= methods.end(); ++p)
		delete methods[p->first];
}


/*
string Class::get_method_name(int index)
{

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index < constant_pool_count-1) ;
	assert(cp.at(index)->tag == constant_pool::CONSTANT_Methodref) ;
#endif

	string ret ;

	int i, j ;

	for(i=0; i < methods.size(); i++)
	{
		if(index == i)
		{
			ret = get_symbol_name(mi[i]->name_index) ;
			break ;
		}
	}

	return ret ;

}
*/

string Class::get_descriptor(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_descriptor()") ;
	FunctionCallLogger fcl("Class::get_descriptor()") ;
#endif
//end of code added 13/11/2004	

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index < constant_pool_count-1) ;

	assert(cp.at(index)->tag == constant_pool::CONSTANT_Fieldref ||
		   cp.at(index)->tag == constant_pool::CONSTANT_InterfaceMethodref ||
		   cp.at(index)->tag == constant_pool::CONSTANT_Methodref) ;
#endif

	int name_and_type_index = cp.at(index)->name_and_type_index ;
	return(get_symbol_name(cp.at(name_and_type_index-1)->descriptor_index)) ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_descriptor()") ;
#endif
//end of code added 13/11/2004
}

string Class::get_symbol_name(int name_index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_symbol_name()") ;
	FunctionCallLogger fcl("Class::get_symbol_name()") ;
#endif
//end of code added 13/11/2004	

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

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_symbol_name()") ;
#endif
//end of code added 13/11/2004

	return ret ;
}

//this function gets the name of the entry
//corresponding to the passed index
//it takes the name_index/name_and_type_index/
//string_index value of the CP
//entry; finds the CP entry corresponding to
//this index and returns the Utf8_bytes value
//(by calling get_symbol_name)
string Class::get_name(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_name()") ;
	FunctionCallLogger fcl("Class::get_name()") ;
#endif
//end of code added 13/11/2004	

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index < constant_pool_count-1) ;

	assert(cp.at(index)->tag == constant_pool::CONSTANT_Class ||
		cp.at(index)->tag == constant_pool::CONSTANT_Fieldref ||
		cp.at(index)->tag == constant_pool::CONSTANT_Methodref ||
		cp.at(index)->tag == constant_pool::CONSTANT_InterfaceMethodref ||
		cp.at(index)->tag == constant_pool::CONSTANT_String) ;
#endif

	string retval = "" ;

	if(cp.at(index)->tag == constant_pool::CONSTANT_Class)
	{
		retval = get_symbol_name(cp.at(index)->name_index) ;
	}
	else if(cp.at(index)->tag == constant_pool::CONSTANT_Fieldref)
	{
		int name_and_type_index = cp.at(index)->name_and_type_index ;
		retval = get_symbol_name(cp.at(name_and_type_index-1)->name_index) ;
	}
	else if(cp.at(index)->tag == constant_pool::CONSTANT_Methodref)
	{
		int name_and_type_index = cp.at(index)->name_and_type_index ;
		retval = get_symbol_name(cp.at(name_and_type_index-1)->name_index) ;
	}
	else if(cp.at(index)->tag == constant_pool::CONSTANT_InterfaceMethodref)
	{
		int name_and_type_index = cp.at(index)->name_and_type_index ;
		retval = get_symbol_name(cp.at(name_and_type_index-1)->name_index) ;
	}
	else if(cp.at(index)->tag == constant_pool::CONSTANT_String)
	{
		int string_index = cp.at(index)->string_index ;
		retval = get_symbol_name(string_index) ;
	}

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_name()") ;
#endif
//end of code added 13/11/2004

    return retval ;
}

string Class::get_string_value(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_string_value()") ;
	FunctionCallLogger fcl("Class::get_string_value()") ;
#endif
//end of code added 13/11/2004	

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index <= constant_pool_count-1) ;
	assert(cp.at(index)->tag == constant_pool::CONSTANT_String) ;
#endif

	int string_index = cp.at(index)->string_index ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_string_value()") ;
#endif
//end of code added 13/11/2004

	return(get_symbol_name(string_index)) ;
}

int Class::get_int_value(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_int_value()") ;
	FunctionCallLogger fcl("Class::get_int_value()") ;
#endif
//end of code added 13/11/2004	

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index <= constant_pool_count-1) ;
	assert(cp.at(index)->tag == constant_pool::CONSTANT_Integer) ;
#endif

	int retval = 0 ;

	retval = cp.at(index)->int_bytes ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_int_value()") ;
#endif
//end of code added 13/11/2004

	return retval ;
}

float Class::get_float_value(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_float_value()") ;
	FunctionCallLogger fcl("Class::get_float_value()") ;
#endif
//end of code added 13/11/2004	

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index <= constant_pool_count-1) ;
	assert(cp.at(index)->tag == constant_pool::CONSTANT_Float) ;
#endif

	float retval = 0 ;

	string a = util::convert_to_binary(cp.at(index)->float_bytes[0]) +
					util::convert_to_binary(cp.at(index)->float_bytes[1]) +
					util::convert_to_binary(cp.at(index)->float_bytes[2]) +
					util::convert_to_binary(cp.at(index)->float_bytes[3]) ;

	bitset<32> b(a) ;

	int s, e, m ;

	if((b >> 31) == 0)
		s = 1 ;
	else
		s = -1 ;

	//string a1 = util::convert_to_binary(16) + util::convert_to_binary(16) ;
	string a1 = "11111111" ;

	bitset<32> t1(a1) ;

	//bitset<32> temp1 = (b >> 23) & 0xff ;
	bitset<32> temp1 = (b >> 23) & t1 ;

	e = temp1.to_ulong() ;

	/*
	string a2 = util::convert_to_binary(7) +
				util::convert_to_binary(16) +
				util::convert_to_binary(16) +
				util::convert_to_binary(16) +
				util::convert_to_binary(16) +
				util::convert_to_binary(16) ;
	*/
	string a2 = "11111111111111111111111" ;

	bitset<32> t2(a2) ;

	/*
	string a3 = util::convert_to_binary(8) +
				util::convert_to_binary(0) +
				util::convert_to_binary(0) +
				util::convert_to_binary(0) +
				util::convert_to_binary(0) +
				util::convert_to_binary(0) ;
	*/
	string a3 = "100000000000000000000000" ;
                           
	bitset<32> t3(a3) ;

	bitset<32> temp2 ;
          
	/*
	if(e == 0)
		temp2 = (b & 0x7fffff) << 1 ;
	else
		temp2 = (b & 0x7fffff) | 0x800000 ;
	*/

	if(e == 0)
		temp2 = (b & t2) << 1 ;
	else
		temp2 = (b & t2) | t3 ;

	m = temp2.to_ulong() ;
		
	retval = s * m * pow((double)2,(double)(e - 150)) ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_float_value()") ;
#endif
//end of code added 13/11/2004
		
	return retval ;
}

__int64 Class::get_long_value(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_long_value()") ;
	FunctionCallLogger fcl("Class::get_long_value()") ;
#endif
//end of code added 13/11/2004	

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index <= constant_pool_count-1) ;
	assert(cp.at(index)->tag == constant_pool::CONSTANT_Long) ;
#endif

	__int64 retval = 0 ;

	string a1 = util::convert_to_binary(cp.at(index)->long_high_bytes[0]) +
					 util::convert_to_binary(cp.at(index)->long_high_bytes[1]) +
					 util::convert_to_binary(cp.at(index)->long_high_bytes[2]) +
					 util::convert_to_binary(cp.at(index)->long_high_bytes[3]) ;

	string a2 = util::convert_to_binary(cp.at(index)->long_low_bytes[0]) +
					 util::convert_to_binary(cp.at(index)->long_low_bytes[1]) +
					 util::convert_to_binary(cp.at(index)->long_low_bytes[2]) +
					 util::convert_to_binary(cp.at(index)->long_low_bytes[3]) ;	
/*
		bitset<32> h(a1) ;

		bitset<32> l(a2) ;

		bitset<32> temp = h << 32 ;

		retval = (long)(temp.to_ulong() + l.to_ulong()) ;
*/

	retval = util::convert_to_long(a1+a2) ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_long_value()") ;
#endif
//end of code added 13/11/2004

	return retval ;
}

double Class::get_double_value(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::get_double_value()") ;
	FunctionCallLogger fcl("Class::get_double_value()") ;
#endif
//end of code added 13/11/2004	

#if DEBUG_LEVEL >= 3
	assert(index >= 0 && index <= constant_pool_count-1) ;
	assert(cp.at(index)->tag == constant_pool::CONSTANT_Double) ;
#endif

	double retval = 0 ;

	string a1 = util::convert_to_binary(cp.at(index)->double_high_bytes[0]) +
					 util::convert_to_binary(cp.at(index)->double_high_bytes[1]) +
					 util::convert_to_binary(cp.at(index)->double_high_bytes[2]) +
					 util::convert_to_binary(cp.at(index)->double_high_bytes[3]) ;

	string a2 = util::convert_to_binary(cp.at(index)->double_low_bytes[0]) +
					 util::convert_to_binary(cp.at(index)->double_low_bytes[1]) +
					 util::convert_to_binary(cp.at(index)->double_low_bytes[2]) +
					 util::convert_to_binary(cp.at(index)->double_low_bytes[3]) ;	

	retval = util::convert_to_double(a1+a2) ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::get_double_value()") ;
#endif
//end of code added 13/11/2004

	return retval ;
}

//This method verifies that the constant pool satisfies
//all the structural constraints (example: for a Class item,
//the name_index really refers to a Utf8 item, etc.). It is 
//called during pass 2 of the verification process.
bool Class::verify()
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::verify()") ;
	FunctionCallLogger fcl("Class::verify()") ;
#endif
//end of code added 13/11/2004	

	bool retval = true ;

	for(int i=0; i < constant_pool_count-1; i++)
	{
		if(cp.at(i)->tag == constant_pool::CONSTANT_Class)
		{

			if(cp.at(cp.at(i)->name_index-1)->tag 
				!= constant_pool::CONSTANT_Utf8)
			{
				retval = false ;
			}
		}
		if(cp.at(i)->tag == constant_pool::CONSTANT_Fieldref ||
		   cp.at(i)->tag == constant_pool::CONSTANT_Methodref ||
		   cp.at(i)->tag == constant_pool::CONSTANT_InterfaceMethodref)
		{
			if(cp.at(cp.at(i)->class_index-1)->tag 
				!= constant_pool::CONSTANT_Class)
			{
				retval = false ;
			}

			if(cp.at(cp.at(i)->name_and_type_index-1)->tag 
				!= constant_pool::CONSTANT_NameAndType)
			{
				retval = false ;
			}
		}
		if(cp.at(i)->tag == constant_pool::CONSTANT_String)
		{
			if(cp.at(cp.at(i)->string_index-1)->tag 
				!= constant_pool::CONSTANT_Utf8)
			{
				retval = false ;
			}
		}
		if(cp.at(i)->tag == constant_pool::CONSTANT_NameAndType)
		{
			if(cp.at(cp.at(i)->name_index-1)->tag 
				!= constant_pool::CONSTANT_Utf8)
			{
				retval = false ;
			}

			if(cp.at(cp.at(i)->descriptor_index-1)->tag 
				!= constant_pool::CONSTANT_Utf8)
			{
				retval = false ;
			}
		}
	}

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::verify()") ;
#endif
//end of code added 13/11/2004

	return retval ;
}


//this methods checks that all the class, field,
//method names/descriptors are well formed
bool Class::verify_names()
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("Class::verify_names()") ;
	FunctionCallLogger fcl("Class::verify_names()") ;
#endif
//end of code added 13/11/2004	

	bool retval = true ;

	for(int i=0; i < constant_pool_count-1; i++)
	{
		if(cp.at(i)->tag == constant_pool::CONSTANT_Class)
		{
			string name = get_symbol_name(cp.at(i)->name_index) ;

			if(util::is_valid_class_name(name) == false)
				retval = false ;
		}

		if(cp.at(i)->tag == constant_pool::CONSTANT_Fieldref)
		{
			if(util::is_valid_identifier(get_name(i)) == false)
			{
				cout << "2. " << get_name(i) << endl ;
				retval = false ;
			}
			if(util::is_valid_descriptor(get_descriptor(i)) == false)
			{
				cout << "3. " << get_descriptor(i) << endl ;
				retval = false ;
			}
		}

		if(cp.at(i)->tag == constant_pool::CONSTANT_Methodref ||
		   cp.at(i)->tag == constant_pool::CONSTANT_InterfaceMethodref)
		{

			string method_name = get_name(i) ;

			if(method_name != "<clinit>" && method_name != "<init>")
			{
				if(util::is_valid_identifier(get_name(i)) == false)
				{
					cout << "4. " << get_name(i) << endl ;
					retval = false ;
				}
			}
			if(util::is_valid_method_descriptor(get_descriptor(i)) == false)
			{
				cout << "5. " << get_descriptor(i) << endl ;
				retval = false ;
			}
		}


	}

//code added 13/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("Class::verify_names()") ;
#endif
//end of code added 13/11/2004	

	return retval ;
}
