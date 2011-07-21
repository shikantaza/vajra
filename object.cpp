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

#include "vajra.h"

//code added 2/5/2005
#include <iostream>
//end of code added 2/5/2005

//code added 29/7/2005
#if DEBUG_LEVEL >= 2
#include <sys/time.h>
#endif
//end of code added 29/7/2005

const int primitive_value::BOOLEAN = 1;
const int primitive_value::_BYTE = 2 ;
const int primitive_value::SHORT = 3 ;
const int primitive_value::INT = 4 ;
const int primitive_value::LONG = 5 ;
const int primitive_value::CHAR = 6 ;
const int primitive_value::FLOAT = 7 ;
const int primitive_value::DOUBLE = 8 ;

const int value::BOOLEAN = 1;
const int value::_BYTE = 2 ;
const int value::SHORT = 3 ;
const int value::INT = 4 ;
const int value::LONG = 5 ;
const int value::CHAR = 6 ;
const int value::FLOAT = 7 ;
const int value::DOUBLE = 8 ;
const int value::REFERENCE = 9 ;
const int value::RETURN_ADDRESS = 10 ;

//code added 22/5/2005
#if DEBUG_LEVEL >= 3
void value::display()
{
	//cout << "--------------dumping value object " << this << "---------" << endl << flush ;
	cout << "value type = " ;
	
	switch(value_type)
	{
		case BOOLEAN:
			cout << "boolean" << endl << flush ;
			break ;			
		case _BYTE:
			cout << "byte" << endl << flush ;
			break ;			
		case SHORT:
			cout << "short" << endl << flush ;
			break ;			
		case INT:
			cout << "int" << endl << flush ;
			break ;			
		case CHAR:
			cout << "char" << endl << flush ;
			break ;			
		case LONG:
			cout << "long" << endl << flush ;
			break ;			
		case FLOAT:
			cout << "float" << endl << flush ;
			break ;			
		case DOUBLE:
			cout << "double" << endl << flush ;
			break ;			
		case REFERENCE:
			cout << "reference" << endl << flush ;
			break ;			
		case RETURN_ADDRESS:
			cout << "return address" << endl << flush ;
			break ;			
	}
	
	cout << "value = " ;
	switch(value_type)
	{
		case BOOLEAN:
			cout << ((bool_value == true) ? "true" : "false") << endl << flush ;
			break ;			
		case _BYTE:
		case SHORT:
		case INT:
		case CHAR:
			cout << i_value << endl << flush ;
			break ;
		case LONG:
			cout << long_value << endl << flush ;
			break ;
		case FLOAT:
			cout << float_value << endl << flush ;
			break ;			
		case DOUBLE:
			cout << double_value << endl << flush ;
			break ;			
		case REFERENCE:
			cout << ref_value << endl << flush ;
			ref_value->display() ;
			break ;			
		case RETURN_ADDRESS:
			cout << ret_add_value << endl << flush ;
			break ;			
	}
	//cout << "---------------end of dump---------------------- " << endl << flush ;
}
#endif

object::object()
{

	blnArrayObject = false ;
	blnPrimitiveArrayObject = false ;

	blnNull = false ;

	//code added 2/7/2004
	blnFinalised = true ; //TO DO: temporarily set to true; to be made false
	ref_count = 0 ;
	//end of code added 2/7/2004
	
	//code added 2/5/2005
	array_component_type = NULL ;
	type = NULL ;
	//end of code added 2/5/2005
        
        //code added 26/8/2005
        isPrimitive = false ;
        blnStringObject = false ;
        //end of code added 26/8/2005

	//code added 15/9/2005
	blnClassObject = false ;
	//end of code added 15/9/2005

#if DEBUG_LEVEL >= 2
//code added 2/5/2005
	cout << "Created object(" << this << ")" << endl << flush ;
//end of code added 2/5/2005
#endif
	
}
object::~object()
{

	//for(int i=0; i<items.size(); i++)
	//	delete items.at(i) ;

	//for(int i=0; i<primitives.size(); i++)
	//	delete primitives.at(i) ;

	//typedef map<string, value *>::const_iterator CI ;

	//for(CI p=fields.begin(); p!= fields.end(); ++p)
	//	delete fields[p->first];

//code added 28/7/2005	
#if DEBUG_LEVEL >= 2
	struct timeval t;
	gettimeofday(&t, NULL) ;
	cout.setf(ios_base::fixed, ios_base::floatfield) ;
	cout << t.tv_sec + t.tv_usec/1e6 << endl << flush ;
	cout << "Deleted object(" << this << ")" << " type " << this->type->name << endl << flush ;
#endif
//end of code added 28/7/2005	
	
}

//code added 2/5/2005
//this method is for debugging purposes; it prints
//the contents of an object
void object::display()
{
	cout << "Printing object details:" << endl << flush ;
	cout << "--------------------begin-----------------------" << endl << flush ;
	cout << "Object address: " << this << endl << flush ;

	cout << "Class Object? " << ((blnClassObject == true) ? "true" : "false") << endl << flush ;
	if(blnClassObject == true)
		cout << "Class object for: " << class_pointer->name << endl << flush ;

	cout << "Null Object? " << ((blnNull == true) ? "true" : "false") << endl << flush ;
	
	if(blnArrayObject != true && type != NULL)
		cout << "Object type: " << type->name << endl << flush ;
	
	cout << "Array object? " << ((blnArrayObject == true) ? "true" : "false")  << endl << flush ;
	
	if(blnArrayObject == true)
	{
		cout <<  "Primitive array object? " << ((blnPrimitiveArrayObject == true) ? "true" : "false") << endl << flush ;
		if(blnPrimitiveArrayObject == true)
			cout <<  "Primitive array type: " << primitive_type << endl << flush ;
		else
		{
			if(array_component_type != NULL)
				cout << "Array type: " << array_component_type->name  << endl << flush ;
		}
	}	
	
	cout <<  "Object data: " ;
	
	for(int i=0; i<data.size(); i++)
		cout << data.at(i) ;
	cout << endl << flush ;
	
	if(blnArrayObject == true)
	{
		cout <<  "Array data: " << endl << flush ;
		
		if(blnPrimitiveArrayObject == false)
		{
			for(int i=0; i<items.size(); i++)
				cout << items.at(i) << endl << flush ;
		}
		else //it's a primitive array object
		{
			for(int i=0; i<primitives.size(); i++)
			{
			
				switch(primitive_type)
				{
					case primitive_value::BOOLEAN:
						cout << ((primitives.at(i)->bool_value == true) ? "true" : "false") << endl << flush ;
						break ;
					case primitive_value::_BYTE:
					case primitive_value::CHAR:
					case primitive_value::SHORT:
					case primitive_value::INT:
						cout << primitives.at(i)->i_value << endl << flush ;
						break ;
					case primitive_value::LONG:
						cout << primitives.at(i)->long_value << endl << flush ;
						break ;
					case primitive_value::FLOAT:
						cout << primitives.at(i)->float_value << endl << flush ;
						break ;
					case primitive_value::DOUBLE:
						cout << primitives.at(i)->double_value << endl << flush ;
						break ;
				} //end of switch				
				
			} //end of for
			
		} //end else
		
	} //end of if(blnArrayObject == true)
	
	typedef map<string, value *>::const_iterator CII ;
	
	for(CII ci=fields.begin(); ci!=fields.end(); ci++)
	{
		cout << "Field name: " << ci->first << " " << "Value: " ;
		
		if(fields[ci->first]->value_type == value::BOOLEAN)
			cout << ((fields[ci->first]->bool_value == true) ? "true" : "false") << endl << flush ;
		else if(fields[ci->first]->value_type == value::_BYTE ||
			fields[ci->first]->value_type == value::SHORT ||
			fields[ci->first]->value_type == value::CHAR  ||
			fields[ci->first]->value_type == value::INT)
			cout << fields[ci->first]->i_value << endl << flush ; 
		else if(fields[ci->first]->value_type == value::LONG)
			cout << fields[ci->first]->long_value << endl << flush ;
		else if(fields[ci->first]->value_type == value::FLOAT)
			cout << fields[ci->first]->float_value << endl << flush ;
		else if(fields[ci->first]->value_type == value::DOUBLE)
			cout << fields[ci->first]->double_value << endl << flush ;
		else if(fields[ci->first]->value_type == value::REFERENCE)
			cout << fields[ci->first]->ref_value << endl << flush ;
		else if(fields[ci->first]->value_type == value::RETURN_ADDRESS)
			cout << fields[ci->first]->ret_add_value << endl << flush ;
	} //end of for
	
	cout << "Primitive object? " << ((isPrimitive == true) ? "true" : "false") << endl <<flush ;
	
	cout << "Finalised? " << ((blnFinalised == true) ? "true" : "false") << endl <<flush ;
	
	cout << "String object? " << ((blnStringObject == true) ? "true" : "false") << endl <<flush ;
	
	if(blnStringObject == true)
		cout << "String value: " << string_val << endl << flush ;
	
	cout << "---------------------end------------------------" << endl << flush ;
}
//end of code added 2/5/2005
