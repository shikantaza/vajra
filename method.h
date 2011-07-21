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

#ifndef _METHOD_
#define _METHOD_

#include <string>
#include <vector>

#include "ClassFile.h"
#include "vajra.h"

#include "jni.h"

//code changed 18/11/2004
//#include "method_key.h"
//end of code changed 18/11/2004

using namespace std ;

class Class ;
class object ;

class code_struct
{
public:
	vector<int> byte_code ;
	vector<exception_table *> ex_tbl ;
} ;

//code changed on 29/6/2004
//class method
class method : public _jobject
//end of code changed 29/6/2004
{
public:

	//identifies the class to which the 
	//method belongs
	Class *type ;

	//these are used to identify what the return type of the method is
	static const int REFERENCE ;
	static const int INTEGER ;
	static const int FLOAT ;
	static const int DOUBLE ;
	static const int LONG ;
	static const int BYTE ;

	int access_flags ;

	//these flags store the access flags of the method
	bool blnPublic ;	
	bool blnPrivate ;
	bool blnProtected ;
	bool blnStatic ;
	bool blnFinal ;
	bool blnSynchronized ;
	bool blnNative ;
	bool blnAbstract ;
	bool blnStrict ;

	string name ;

	string descriptor ;		//parameter_string & return descriptor
	string parameter_string ;	//together equal descriptor; they are
	string return_descriptor ; 	//stored separately so that verification
					//(one of those in pass 2) is made easier

	vector<string> *parameters ;

	code_struct code ;

    //if it is a native method,
    //the pointer to the native method is
    //stored here

    void (*void_fnPtr)(JNIEnv *, jobject, ...) ;
    object* (*jobject_fnPtr)(JNIEnv *, jobject, ...) ;
    jboolean (*jboolean_fnPtr)(JNIEnv *, jobject, ...) ;
    jbyte (*jbyte_fnPtr)(JNIEnv *, jobject, ...) ;
    jchar (*jchar_fnPtr)(JNIEnv *, jobject, ...) ;
    jshort (*jshort_fnPtr)(JNIEnv *, jobject, ...) ;
    jint (*jint_fnPtr)(JNIEnv *, jobject, ...) ;
    jlong (*jlong_fnPtr)(JNIEnv *, jobject, ...) ;
    jfloat (*jfloat_fnPtr)(JNIEnv *, jobject, ...) ;
    jdouble (*jdouble_fnPtr)(JNIEnv *, jobject, ...) ;

    //indicates whether the native method
    //has been bound
    bool blnBound ;
    
    //vector<int> parameters ;
    int return_type_flag ;
    int return_type_reference_index ;	//if return type is REFERENCE,
					//the index into the classes
					//vector

    vector<line_number_table *> line_numbers ;

    //this method determines the line number
    //for a given code index by scanning through
    //the line_numbers vector
    int get_line_number(int) ;

    int max_locals ;

    method() ;
    ~method() ;

    //code addded 7/8/2004
    //to store the exception thrown by a method
    vector<Class *> exceptions ;
    //end of code added 7/8/2004

    //code added 13/8/2005
    //this object is used in reflection scenarios
    object *method_object ;
    //end of code added 13/8/2005

} ;
#endif
