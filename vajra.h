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

#ifndef _VAJRA_
#define _VAJRA_

#ifdef WIN32
#include <windows.h>
#endif

#include <map>
#include <vector>

#include "jni.h"
#include "field.h"
#include "method.h"
#include "ClassFile.h"
#include "monitor.h"

using namespace std ;

#ifndef WIN32
typedef void * HINSTANCE ;
#endif

#ifndef WIN32
typedef long long __int64 ;
#endif

//forward declarations
class Class ;
class method ;
class value ;

class primitive_value
{
public:
	static const int BOOLEAN ;
	static const int _BYTE ;
	static const int SHORT ;
	static const int INT ;
	static const int LONG ;
	static const int CHAR ;
	static const int FLOAT ;
	static const int DOUBLE ;

	int value_type ;

	//this applies only to float 
	//and double values
	bool IsNaN ;		//is it a NaN?
	bool IsInfinity ;	//is it infinity?
	bool IsPositive ;	//applies only for infinity & zero

	bool bool_value ;
	int i_value ;
	__int64 long_value ;
	float float_value ;
	double double_value ;

	//code added 15/9/2005
	primitive_value()
	{
		IsNaN  = false ;
		IsInfinity  = false ;
		IsPositive  = false ;
		bool_value  = false ;
	}
	//end of code added 15/9/2005

} ;


class object : public _jobject
{
public:

	//class objects will have their attributes
	//set in the 'Class' class itself. there will
	//be just a pointer to that class here

	//idntifies whether it is a class object
	bool blnClassObject ;

	//if it is a class object, this pointer
	//stores the reference to the Class variable
	//that will implement the class object functionality
	Class *class_pointer ;

	//identifies null objects
	bool blnNull ;

	//identifies the class to which the
	//object belongs; valid only 
	//for non-array objects
	Class *type ;

	//identifies whether the object is an
	//array object or not
	bool blnArrayObject ;

	//identifies whether the array object's
	//components are objects or primitives
	bool blnPrimitiveArrayObject ;

	//if the components are objects,
	//identifies the type of the objects 
	//(there is a redundancy there, as we 
	//can determine the type of the component 
	//from the individual items too)
	Class *array_component_type ;

	//if the components are primitives,
	//identifies the type of the primitives 
	//(there is a redundancy there, as we 
	//can determine the type of the component 
	//from the individual items too)
	int primitive_type ;
		
	vector<unsigned char> data ;

	//if the object is an array object
	//storing objects (as opposed to primitives),
	//this vector stores the objects that
	//constitute the array
	vector<object *> items ;

	//if the array object stores primitives,
	//this vector stores the primitive values
	vector<primitive_value *> primitives ;

	int ref_count ; //when ref_count becomed zero, garbage collect the object

	monitor mon ;

	//contains the fields (could be references or 
	//primitives) in the object
	map<string, value *> fields ;

	object() ;
	~object() ;

	//code added 1/7/2004
	bool blnFinalised ;
	//end of code added 1/7/2004


	//code added 24/10/2004
	//to handle primitive-wrapping problems
	
	bool isPrimitive ;

	//TO DO: we should also add a member
	//to identify what type of primitive 
	//object this is
	
	jboolean bool_val ;
	jbyte byte_val ;
	jchar char_val ;
	jshort short_val ;
	jint int_val ;
	jlong long_val ;
	jfloat float_val ;
	jdouble double_val ;
	//end of code added 24/10/2004

	//code added 23/11/2004
	bool blnStringObject ;
	//end of code added 23/11/2004

	//code added 14/1/2005
	object *parent_object ;
	//end of code added 14/1/2005

	//code added 22/3/2005
	//just for debugging; will be removed later
	//if the object is a string object, this will store
	//the string value; populated by jvm::create_string_object()
	string string_val ;
	//end of code added 22/3/2005

	//code added 2/5/2005
	//this method pretty-prints the object's contents.
	//it recurses through the object's fields (if they are
	//not primitives, that is, and calls display() for them as well)
	void display() ;
	//end of code addded 2/5/2005
	
	//code added 13/8/2005
	method *method_pointer ;
	//end of code added 13/8/2005
} ;


//code changed 19/2/2005
//class method_key ;
//end of code changed 19/2/2005

class value
{
public:
	static const int BOOLEAN ;
	static const int _BYTE ;
	static const int SHORT ;
	static const int INT ;
	static const int LONG ;
	static const int CHAR ;
	static const int FLOAT ;
	static const int DOUBLE ;
	static const int REFERENCE ;
	static const int RETURN_ADDRESS ;

	int value_type ;

	//this applies only to float 
	//and double values
	bool IsNaN ;		//is it a NaN?
	bool IsInfinity ;	//is it infinity?
	bool IsPositive ;	//applies only for infinity & zero

	bool bool_value ;
	//int i_value ; //commented out because JNI defines jint as long
	long i_value ;
	__int64 long_value ;
	float float_value ;
	double double_value ;
	int ret_add_value ;
	object *ref_value ;

	//code added 29/1/2005
	//this is required because in it's absence, creation of
	//a new 'value' object in jvm::i_load() screws up another
	//'value' object created earlier

	//code changed 15/9/2005
	//value::value() {}
	value::value()
	{
		IsNaN = false ;
		IsInfinity = false ;
		IsPositive = false ;
		bool_value = false ;
	}
	//end of code changed 15/9/2005

        value::~value()
        {
                //destructor deliberately does not
                //free ref_value
        }
        
	//end of code added 29/1/2005

		
	//code added 22/5/2005
	//dumps the contents of the value
#if DEBUG_LEVEL >= 3
	void display() ;
#endif		
	//end of code added 22/5/2005
} ;

class static_value : public value
{
public:
	string descriptor ;
	Class *type ; //identifies the class to which 
		      //the static value belongs
} ;

class runtime_constant_pool
{
	public:

		static const int CLASS ;
		static const int INTERFACE ;  //is this needed?
		static const int FIELD ;
		static const int CLASS_METHOD ;
		static const int INTERFACE_METHOD ;
		static const int STRING_LITERAL ;
		static const int INTEGER_CONSTANT ;
		static const int FLOAT_CONSTANT ;
		static const int LONG_CONSTANT ;
		static const int DOUBLE_CONSTANT ;

		bool valid_flag ;
		int flag ;				//whether the item is a class, field, etc.

		//definitions of symbolic references
		string class_interface_name ;

		string field_name ;
		string field_descriptor ;
		string field_parent_class_name ;

		string method_name ;
		string method_descriptor ;
		string method_parent_class_name ;

		//idetifies whether the symbolic reference
		//has been resolved or not
		bool blnResolved ;

		Class *resolved_class ;
		method *resolved_method ;

		//string name ;				//name of the item
		//string descriptor ;			//decriptor for fields, methods
		//string parent_class_name ;		//name of parent class

/*
		int index ;				//to resolve symbolic names to index into references array
		int parent_index ;			//for methods and fields
		int parent_class_index ;		//index into the references array that refers to parent class
		int field_method_index ;		//within the class, to identify the field/method
*/
		string str_value ;			//value of string literal
		__int64 int_value ;			//this is for both integer and long constants
		double double_value ;			//this is for both float and double constants

		//code added 15/9/2005
		runtime_constant_pool()
		{
			valid_flag = false ;
			blnResolved = false ;
		}
		//end of code added 15/9/2005

} ;


class Interface
{
public:
	string interface_name ;		//symbolic name ;
	Class *interface_ref ;
} ;

//class/interface definition
//we are deriving Class from _jclass
//so that it will be easy to implement 
//the JNI functions (is this a hack, or is 
//there a better way?)
class Class : public _jclass
{
	public:

		Class(const Class&) ;
		Class() ;
		~Class() ;

		Class& operator=(const Class&) ;

		//string get_method_name(int) ;
		string get_descriptor(int) ;
		string get_symbol_name(int) ;
		string get_name(int) ;

		string get_string_value(int) ;
		int get_int_value(int) ;
		float get_float_value(int) ;
		__int64 get_long_value(int) ;
		double get_double_value(int) ;

		bool verify() ;
		bool verify_names() ;

		string name ;
		string super_class_name ;

		string source_file_name ;

		Class *super_class ;

		bool blnVerified ;  //flag to indicate whether the class
				    //has been verified or not

		bool blnPrepared ;  //flag to indicate whether the class
				    //has been prepared or not

		bool blnInitialised ;  //flag to indicate whether the class
				    	//has been initialised or not

		//these flags store the access flags of the class/interface
		bool blnPublic ;	//public class
		bool blnFinal ;		//is a final class
		bool blnSuper ;		//treat superclass methods specially (invokespecial)
		bool blnInterface ;	//is an interface, not a class
		bool blnAbstract ;	// is an abstract class; cannot ne instantiated

		//this is NULL if this class was loaded
		//by the Bootstrap Loader; otherwise,
		//it contains the pointer to the class
		//that loaded it
		Class *initiating_loader ;
		Class *defining_loader ;

		vector<Interface *> interfaces ;

		int constant_pool_count ;
		vector<constant_pool *> cp ;
		vector<runtime_constant_pool *> rcp ;
		vector<field *> fields ;

		//code changed 17/11/2004
		//map<method_key, method *> methods ;
		map<string, method *> methods ;
		//end of code changed 17/11/2004

		bool blnArrayClass ; 	// this will be true if this is
					// an array

		bool blnPrimitiveArray ;  // true if the array is
                                          // a primitive array

		int nodims ; //how many dimensions in the array
		Class *array_class_ref ; //if it is an array class, this variable
				      	 //stores the address of the element class

		//if the array is a primitive array,
		//these indicate which primitive
		static const int PRIMITIVE_BYTE ;
		static const int PRIMITIVE_CHAR ;
		static const int PRIMITIVE_DOUBLE ;
		static const int PRIMITIVE_FLOAT ;
		static const int PRIMITIVE_INT ;
		static const int PRIMITIVE_LONG ;
		static const int PRIMITIVE_SHORT ;
		static const int PRIMITIVE_BOOLEAN ;

		int primitive_type ;

		monitor mon ;

		map<string, static_value *> static_fields ;

		//actually a hack; since the functionality
		//required of a class object is supplied in
		//in this class itself, we just instantiate
		//a dummy Object, which will contain a pointer
		//to this
		object *class_object ;

		//this stores the handle to the native method DLL
		//(if any); NULL if no native methods exist
		//code changed 15/5/2004
		//HINSTANCE native_library_handle ;
		//end of code changed 15/5/2004

		//code added 29/6/2004
		//if the class represents a primitive type
		bool blnPrimitiveClass ;

		//if primitive type, which one
		//we already have a variable called 'primitive_type' (for array classes),
		//hence the clumsy name
		int primitive_type_of_class ; 
		//end of code added 29/6/2004

		//code added 30/6/2004
		//if this class contains any inner classes, this
		//vector will contain them
		vector<Class *> inner_classes ;

		//inner classes will have this populated with
		//the declaring class
		Class *declaring_class ;

		//end of code added 30/6/2004

		//code added 30/6/2004
		//if the class represents 'void'
		bool blnVoid ;
		//end of code added 30/6/2003

		//code added 15/8/2005
		//this is the actual class object.
		//the other data member (class_object)
		//is to be phased out
		object *_class_object ;
		//end of code added 15/8/2005
} ;


class object ;



//derived from value because of the
//similarities
class LocalVariable : public value
{
public:
	bool valid_flag ; //this will be false for the 
			  //second local variable for storing
			  //double and long values

	//code added 15/9/2005
	LocalVariable()
	{
		valid_flag = false ;
	}
	//end of code adde 15/9/2005

        //code added 18/9/2005
        ~LocalVariable()
        {
                //this does not delete the 'ref_value' member.
                //that will be handled in jvm's destructor
        }
        //end of code added 18/9/2005
} ;


#endif
