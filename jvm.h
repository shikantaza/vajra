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

#ifndef _JVM_
#define _JVM_

#include <iostream>

#include <map>
#include <vector>
#include <stack>
#include <stdint.h>

#include "jni.h"

#include "vajra.h"
//#include "object.h"
//#include "value.h"
#include "exceptions.h"

using namespace std ;

typedef uintptr_t word ;

struct twowords
{
#ifdef REVERSED_CALLING_CONVENTION
    word second ;
    word first ;
#else
    word first ;
    word second ;
#endif
} ;

union word64
{
    struct twowords w ;
    jdouble d ;
    jlong l ;
} ;

/*
struct arg_values
{
    void *void_arg ;
    bool blnFloat ;
    bool blnDouble ;
    float float_arg ;
    double double_arg ;
} ;
*/

class key
{
public:
	Class *loader ;
	string fully_qualified_name ;

	key(Class *, string) ;

	bool operator <(const key& a) const
	{
		if(a.loader < this->loader)
			return true ;

		if(a.loader == this->loader)
			return (a.fully_qualified_name < this->fully_qualified_name) ;

		return false ;

	} 
} ;

class loading_constraint
{
public:
	string class_name ;
	Class *L1 ;
	Class *L2 ;

	loading_constraint(string, Class *, Class *) ;

	bool operator <(const loading_constraint& a) const
	{
		if(a.class_name < this->class_name)
			return true ;

		if(a.class_name == this->class_name)
		{
			if(a.L1 < this->L1)
				return true ;
			if(a.L1 == this->L1)
				return a.L2 < this->L2 ;

			return false ;
		}

		return false ;
	}
} ;

class frame
{
	public:
		vector<LocalVariable *> local_variables ;
		stack<value *> operand_stack ;

		object *mon_obj ; //the object on which the method has obtained a lock

		//reference to the runtime constant
		//pool of the class to which the
		//current method belongs
		vector<runtime_constant_pool *> *rcp ; 

		method *curr_method ;

		//stores the contents of the thread's
		//PC register when the current frame
		//was created; at the this frame is popped, 
		//this value will be written back to the 
		//thread's PC register
		int prev_pc_value ;

		//code added 13/2/2005
		bool blnExecutingNativeMethod ;
		//end of code added 13/2/2005

		//code added 15/1/2005
		frame()
		{
			mon_obj = NULL ;
			
			//code added 13/2/2005
			blnExecutingNativeMethod = false ;
			//end of code added 13/2/2005
			
//code added 24/7/2005
#if DEBUG_LEVEL >= 1
			cout << "Frame " << this << " created" << endl << flush ;
#endif			
//end of code added 24/7/2005

		}
		//end of code added 15/1/2005

		//code added 25/7/2005
		//to aid in debugging. currently only frame objects
		//created from opcodes.cpp will invoke this constructor.
		//frame objects created in jni.cpp will continue to
		//invoke the no-arguments constructor above.
		frame(method *m)
		{
			curr_method = m ;
			rcp = &(m->type->rcp);
			
			mon_obj = NULL ;

			blnExecutingNativeMethod = false ;
#if DEBUG_LEVEL >= 1
			cout << "Frame " << this << "[" << m->type->name << "->" << m->name << "()] created {" << endl << flush ;
#endif						
		}
		//end of code added 25/7/2005
		
		~frame()
		{

			for(int i=0; i<local_variables.size(); i++)
				delete local_variables.at(i) ;

			while(!operand_stack.empty())
			{
				delete operand_stack.top() ;
				operand_stack.pop() ;
			}
			
//code added 24/7/2005
#if DEBUG_LEVEL >= 1
			cout << "Frame " << this << "[" << curr_method->type->name << "->" << curr_method->name << "()] deleted }"  << endl<< flush ;
#endif			
//end of code added 24/7/2005
			
		}
} ;

class thread : public _jobject
{
public:

       	int thread_id ;  //uniquely identifies the thread
       	string name ;	 //name of the thread
       	int pc ;	 //program counter

       	//stores the number of instructions
       	//executed for this thread in the current
       	//pass; will be set to zero when
       	//the VM passes cotrol to it.
       	//when this value reaches a pre-determined value,
       	//the thread will be preempted by the VM
       	int nof_inst_in_curr_pass ;

       	bool blnReady ; //is the thread ready to run?

       	stack<frame *> java_stack ;

       	//if an exception is being thrown, 
       	//this will be set to true. it will be
       	//reset to false once the exception has
       	//been handled
       	bool blnExceptionBeingThrown ;

       	//when an exception is thrown, the thread
       	//is given a pointer to the exception object
       	object *exception_object ;

       	//this stack is meant for debugging purposes.
       	//it will print a trace of the exception.
       	stack<string> exception_trace ;

       	//this will store references to all objects passed to
       	//native methods. it will be populated when
       	//a native method is invoked and emptied
       	//when the native method returns
	vector<object *> local_ref_registry ;

        //moved to the jvm class
        //JNIEnv jni_env ;

        vector<monitor> monitors ;

	//code added 3/7/2004
	//for implementation of java.lang.Thread
	bool blnDaemon ; //is this a daemon thread?

	bool blnAlive ; //is the thread alive?

	int priority ; //priority of the thread
	//end of code added 3/7/2004

	//code added 5/7/2004
	//for implementation of java.lang.Thread
	bool blnInterrupted ; //has the thread been interrupted?

	bool blnAskedToSleep ; //has this thread been asked to sleep?

	long sleepMilliSecs ; //if asked to sleep, how many milliseconds

	int sleepNanoSecs ; //if asked to sleep, how many nanoseconds

	//end of code added 5/7/2004

	//code added 6/7/2004
	//for implementation of java.lang.thread
	bool blnSuspended ; //has this thread been suspended?

	//end of code added 6/7/2004

	//code added 7/7/2004
	bool blnStarted ; //has the thread started execution?

	//end of code added 7/7/2004

	//code added 27/8/2005
	thread()
	{
		blnExceptionBeingThrown = false ;

		//code added 15/9/2005
		blnDaemon = false ;
		blnAlive = false ;
		blnInterrupted = false ;
		blnAskedToSleep = false ;
		blnSuspended = false ;
		blnStarted = false ;
		//end of code added 15/9/2005

	}
	//end of code added 27/8/2005
        
        ~thread()
        {
                while(!java_stack.empty())
                {
                        delete java_stack.top() ;
                        java_stack.pop() ;
                }
        }
} ;

//code added 24/6/2004
//forward definition for MyJNINativeInterface_
//MyJNINativeInterface_ will contain a pointer
//to the 'jvm' object (this is a better
//approach than to add the required data structure
//in a piece-meal fasnion (a la native_library_handles --
//this will be refactored later)
class jvm ;
//end of code added 24/6/2004

//code added 13/5/2004
class MyJNINativeInterface_ : public JNINativeInterface_
{
public:
  //TO DO: this can be knocked off since
  //we have included jvm_ptr itself
  vector<HINSTANCE> *native_library_handles ;

  //code added 24/6/2004
  //for implementing Classpath's classes' native methods
  jvm *jvm_ptr ;
  //end of coded added 24/6/2004

} ;
//end of code added 13/5/2004

class jvm
{
public:

       	static const int VIRTUAL_MACHINE_ERROR ;
       	static const int NO_CLASS_DEF_FOUND ;
       	static const int FORMAT_ERROR ;
       	static const int VERIFY_ERROR ;
       	static const int LINKAGE_ERROR ;
       	static const int CLASS_CIRCULARITY_ERROR ;
       	static const int INCOMPATIBLE_CLASS_CHANGE_ERROR ;
       	static const int NULL_POINTER_EXCEPTION ;
       	static const int ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION ;
       	static const int ARRAY_STORE_EXCEPTION ;
       	static const int NEGATIVE_ARRAY_SIZE_EXCEPTION ;
       	static const int ILLEGAL_MONITOR_STATE_EXCEPTION ;

       	vector<object *> heap ;

       	//all string instances will be stored
       	//here; whenever a string literal is to be
       	//derived (from a runtime constant pool entry),
       	//we first check if the string exists here; if yes,
       	//we return the object, otherwise we create a new string object
       	//and add it to the pool
       	map<string, object *> strings ;

       	map<key, Class *> classes ;
       	vector<thread *> threads ;

       	int nof_instructions ;  //simple implementation of
       			        //threads: we execute nof_instrcutions
       				//in a thread, then switch to next
       				//thread

       	int curr_thread ;

       	map<loading_constraint, loading_constraint *> constraint_set ;

       	//int resolve(string) ;
       	void load_class(string) ;
       	void link_class(Class *) ;
       	void verify_class(Class *) ;

       	//this method prepares the class 
       	//passed as parameter.
       	//preparation is the creation of the static fields of the
       	//class/interface and initialising these fields to their
       	//default values 
       	void prepare_class(Class *) ;

       	Class * resolve_class(Class *, string) ;
       	field * resolve_field(Class *, string, string, string, bool) ;
       	method * resolve_class_method(Class *, string, string, string) ;
       	method * resolve_interface_method(Class *, string, string, string) ;

       	bool check_field_access(Class *, Class *, field *) ;
       	bool check_method_access(Class *, Class *, method *) ;


       	string get_package_name(Class *) ;

       	void insert_constraint(string, Class *, Class *) ;
       	void delete_constraint(string, Class *, Class *) ;
       	bool blnLoadingConstViolated() ;

	//code changed 13/5/2004
        //JNINativeInterface_ intf ;
	MyJNINativeInterface_ intf ;
	//end of code changed 13/5/2004

        JNIInvokeInterface_ invoke_intf ;

        JDK1_1InitArgs init_args ;

	/***** JNI-related functions have been made global

       	//begin JNI functions
       	jint GetVersion(JNIEnv *) ;

       	jclass DefineClass(JNIEnv *, const char *, jobject, const jbyte *, jsize) ;
       	jclass FindClass(JNIEnv *, const char *) ;
       	jclass GetSuperclass(JNIEnv *, jclass) ;
       	jboolean IsAssignableFrom(JNIEnv *, jclass, jclass) ;

       	jint Throw(JNIEnv *, jthrowable) ;
       	jint ThrowNew(JNIEnv *, jclass, const char *) ;
       	jthrowable ExceptionOccurred(JNIEnv *) ;
       	void ExceptionDescribe(JNIEnv *) ;
       	void ExceptionClear(JNIEnv *) ;
       	void FatalError(JNIEnv *, const char *) ;

       	jobject NewGlobalRef(JNIEnv *, jobject) ;
       	void DeleteGlobalRef(JNIEnv *, jobject) ;
       	void DeleteLocalRef(JNIEnv *, jobject) ;

       	//the ..V() methods have been commented out because
       	//of the 'va_start used in function with
       	//fixed args' compiler error in GCC. The corresonding
       	//..V() function pointer in intf is set to the
       	//address of the equivalent (...) method
       	//e.g. NewObjectV is set to NewObject

       	jobject AllocObject(JNIEnv *, jclass) ;
       	jobject NewObject(JNIEnv *, jclass, jmethodID ...) ;
       	jobject NewObjectA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jobject NewObjectV(JNIEnv *, jclass, jmethodID, va_list) ;
       	jclass GetObjectClass(JNIEnv *, jobject) ;
       	jboolean IsInstanceOf(JNIEnv *, jobject, jclass) ;
       	jboolean IsSameObject(JNIEnv *, jobject, jobject) ;

       	jfieldID GetFieldID(JNIEnv *, jclass, const char *, const char *) ;

       	jobject GetObjectField(JNIEnv *, jobject, jfieldID) ;
       	jboolean GetBooleanField(JNIEnv *, jobject, jfieldID) ;
       	jbyte GetByteField(JNIEnv *, jobject, jfieldID) ;
       	jchar GetCharField(JNIEnv *, jobject, jfieldID) ;
       	jshort GetShortField(JNIEnv *, jobject, jfieldID) ;
       	jint GetIntField(JNIEnv *, jobject, jfieldID) ;
       	jlong GetLongField(JNIEnv *, jobject, jfieldID) ;
       	jfloat GetFloatField(JNIEnv *, jobject, jfieldID) ;
       	jdouble GetDoubleField(JNIEnv *, jobject, jfieldID) ;

       	void SetObjectField(JNIEnv *, jobject, jfieldID, jobject) ;
       	void SetBooleanField(JNIEnv *, jobject, jfieldID, jboolean) ;
       	void SetByteField(JNIEnv *, jobject, jfieldID, jbyte) ;
       	void SetCharField(JNIEnv *, jobject, jfieldID, jchar) ;
       	void SetShortField(JNIEnv *, jobject, jfieldID, jshort) ;
       	void SetIntField(JNIEnv *, jobject, jfieldID, jint) ;
       	void SetLongField(JNIEnv *, jobject, jfieldID, jlong) ;
       	void SetFloatField(JNIEnv *, jobject, jfieldID, jfloat) ;
       	void SetDoubleField(JNIEnv *, jobject, jfieldID, jdouble) ;

       	jmethodID GetMethodID(JNIEnv *, jclass, const char *, const char *) ;

       	//since the Call<type>Methods share a lot of common code,
       	//we use a common method
       	void CallVoidMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	void CallVoidMethod(JNIEnv *, jobject, jmethodID ...) ;
       	void CallVoidMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//void CallVoidMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jobject CallObjectMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jobject CallObjectMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jobject CallObjectMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jobject CallObjectMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jboolean CallBooleanMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jboolean CallBooleanMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jboolean CallBooleanMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jboolean CallBooleanMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jbyte CallByteMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jbyte CallByteMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jbyte CallByteMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jbyte CallByteMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jchar CallCharMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jchar CallCharMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jchar CallCharMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jchar CallCharMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jshort CallShortMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jshort CallShortMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jshort CallShortMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jshort CallShortMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jint CallIntMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jint CallIntMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jint CallIntMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jint CallIntMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jlong CallLongMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jlong CallLongMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jlong CallLongMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jlong CallLongMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jfloat CallFloatMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jfloat CallFloatMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jfloat CallFloatMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jfloat CallFloatMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	jdouble CallDoubleMethodInternal(JNIEnv *, jobject, jmethodID, vector<LocalVariable *> *) ;
       	jdouble CallDoubleMethod(JNIEnv *, jobject, jmethodID ...) ;
       	jdouble CallDoubleMethodA(JNIEnv *, jobject, jmethodID, jvalue *) ;
       	//jdouble CallDoubleMethodV(JNIEnv *, jobject, jmethodID, va_list) ;

       	void CallNonvirtualVoidMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	void CallNonvirtualVoidMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	void CallNonvirtualVoidMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//void CallNonvirtualVoidMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jobject CallNonvirtualObjectMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jobject CallNonvirtualObjectMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jobject CallNonvirtualObjectMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jobject CallNonvirtualObjectMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jboolean CallNonvirtualBooleanMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jboolean CallNonvirtualBooleanMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jboolean CallNonvirtualBooleanMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jboolean CallNonvirtualBooleanMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jbyte CallNonvirtualByteMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jbyte CallNonvirtualByteMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jbyte CallNonvirtualByteMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jbyte CallNonvirtualByteMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jchar CallNonvirtualCharMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jchar CallNonvirtualCharMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jchar CallNonvirtualCharMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jchar CallNonvirtualCharMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jshort CallNonvirtualShortMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jshort CallNonvirtualShortMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jshort CallNonvirtualShortMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jshort CallNonvirtualShortMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jint CallNonvirtualIntMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jint CallNonvirtualIntMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jint CallNonvirtualIntMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jint CallNonvirtualIntMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jlong CallNonvirtualLongMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jlong CallNonvirtualLongMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jlong CallNonvirtualLongMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jlong CallNonvirtualLongMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jfloat CallNonvirtualFloatMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jfloat CallNonvirtualFloatMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jfloat CallNonvirtualFloatMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jfloat CallNonvirtualFloatMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jdouble CallNonvirtualDoubleMethodInternal(JNIEnv *, jobject, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jdouble CallNonvirtualDoubleMethod(JNIEnv *, jobject, jclass, jmethodID ...) ;
       	jdouble CallNonvirtualDoubleMethodA(JNIEnv *, jobject, jclass, jmethodID, jvalue *) ;
       	//jdouble CallNonvirtualDoubleMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list) ;

       	jfieldID GetStaticFieldID(JNIEnv *, jclass, const char *, const char *) ;

       	jobject GetStaticObjectField(JNIEnv *, jclass, jfieldID) ;
       	jboolean GetStaticBooleanField(JNIEnv *, jclass, jfieldID) ;
       	jbyte GetStaticByteField(JNIEnv *, jclass, jfieldID) ;
       	jchar GetStaticCharField(JNIEnv *, jclass, jfieldID) ;
       	jshort GetStaticShortField(JNIEnv *, jclass, jfieldID) ;
       	jint GetStaticIntField(JNIEnv *, jclass, jfieldID) ;
       	jlong GetStaticLongField(JNIEnv *, jclass, jfieldID) ;
       	jfloat GetStaticFloatField(JNIEnv *, jclass, jfieldID) ;
       	jdouble GetStaticDoubleField(JNIEnv *, jclass, jfieldID) ;

       	void SetStaticObjectField(JNIEnv *, jclass, jfieldID, jobject) ;
       	void SetStaticBooleanField(JNIEnv *, jclass, jfieldID, jboolean) ;
       	void SetStaticByteField(JNIEnv *, jclass, jfieldID, jbyte) ;
       	void SetStaticCharField(JNIEnv *, jclass, jfieldID, jchar) ;
       	void SetStaticShortField(JNIEnv *, jclass, jfieldID, jshort) ;
       	void SetStaticIntField(JNIEnv *, jclass, jfieldID, jint) ;
       	void SetStaticLongField(JNIEnv *, jclass, jfieldID, jlong) ;
       	void SetStaticFloatField(JNIEnv *, jclass, jfieldID, jfloat) ;
       	void SetStaticDoubleField(JNIEnv *, jclass, jfieldID, jdouble) ;

       	jmethodID GetStaticMethodID(JNIEnv *, jclass, const char *, const char *) ;

       	void CallStaticVoidMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	void CallStaticVoidMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	void CallStaticVoidMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//void CallStaticVoidMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jobject CallStaticObjectMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jobject CallStaticObjectMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jobject CallStaticObjectMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jobject CallStaticObjectMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jboolean CallStaticBooleanMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jboolean CallStaticBooleanMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jboolean CallStaticBooleanMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jboolean CallStaticBooleanMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jbyte CallStaticByteMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jbyte CallStaticByteMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jbyte CallStaticByteMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jbyte CallStaticByteMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jchar CallStaticCharMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jchar CallStaticCharMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jchar CallStaticCharMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jchar CallStaticCharMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jshort CallStaticShortMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jshort CallStaticShortMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jshort CallStaticShortMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jshort CallStaticShortMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jint CallStaticIntMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jint CallStaticIntMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jint CallStaticIntMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jint CallStaticIntMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jlong CallStaticLongMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jlong CallStaticLongMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jlong CallStaticLongMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jlong CallStaticLongMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jfloat CallStaticFloatMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jfloat CallStaticFloatMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jfloat CallStaticFloatMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jfloat CallStaticFloatMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jdouble CallStaticDoubleMethodInternal(JNIEnv *, jclass, jmethodID, vector<LocalVariable *> *) ;
       	jdouble CallStaticDoubleMethod(JNIEnv *, jclass, jmethodID, ...) ;
       	jdouble CallStaticDoubleMethodA(JNIEnv *, jclass, jmethodID, jvalue *) ;
       	//jdouble CallStaticDoubleMethodV(JNIEnv *, jclass, jmethodID, va_list) ;

       	jstring NewString(JNIEnv *, const jchar *, jsize) ;
       	jsize GetStringLength(JNIEnv *, jstring) ;
       	const jchar * GetStringChars(JNIEnv *, jstring, jboolean *) ;
       	void ReleaseStringChars(JNIEnv *, jstring, const jchar *) ;

       	jstring NewStringUTF(JNIEnv *, const char *) ;
       	jsize GetStringUTFLength(JNIEnv *, jstring) ;
       	//const char * GetStringUTFChars(JNIEnv *, jstring, jboolean *) ;
       	//void ReleaseStringUTFChars(JNIEnv *, jstring, const char *) ;

        jsize GetArrayLength(JNIEnv *, jarray) ;
        jobjectArray NewObjectArray(JNIEnv *, jsize, jclass, jobject) ;
        jobject GetObjectArrayElement(JNIEnv *, jobjectArray, jsize) ;
        void SetObjectArrayElement(JNIEnv *, jobjectArray, jsize, jobject) ;

        jbooleanArray NewBooleanArray(JNIEnv *, jsize) ;
        jbyteArray NewByteArray(JNIEnv *, jsize) ;
        jcharArray NewCharArray(JNIEnv *, jsize) ;
        jshortArray NewShortArray(JNIEnv *, jsize) ;
        jintArray NewIntArray(JNIEnv *, jsize) ;
        jlongArray NewLongArray(JNIEnv *, jsize) ;
        jfloatArray NewFloatArray(JNIEnv *, jsize) ;
        jdoubleArray NewDoubleArray(JNIEnv *, jsize) ;

        jboolean *GetBooleanArrayElements(JNIEnv *, jbooleanArray, jboolean *) ;
        jbyte *GetByteArrayElements(JNIEnv *, jbyteArray, jboolean *) ;
        jchar *GetCharArrayElements(JNIEnv *, jcharArray, jboolean *) ;
        jshort *GetShortArrayElements(JNIEnv *, jshortArray, jboolean *) ;
        jint *GetIntArrayElements(JNIEnv *, jintArray, jboolean *) ;
        jlong *GetLongArrayElements(JNIEnv *, jlongArray, jboolean *) ;
        jfloat *GetFloatArrayElements(JNIEnv *, jfloatArray, jboolean *) ;
        jdouble *GetDoubleArrayElements(JNIEnv *, jdoubleArray, jboolean *) ;

        void ReleaseBooleanArrayElements(JNIEnv *, jbooleanArray, jboolean *, jint) ;
        void ReleaseByteArrayElements(JNIEnv *, jbyteArray, jbyte *, jint) ;
        void ReleaseCharArrayElements(JNIEnv *, jcharArray, jchar *, jint) ;
        void ReleaseShortArrayElements(JNIEnv *, jshortArray, jshort *, jint) ;
        void ReleaseIntArrayElements(JNIEnv *, jintArray, jint *, jint) ;
        void ReleaseLongArrayElements(JNIEnv *, jlongArray, jlong *, jint) ;
        void ReleaseFloatArrayElements(JNIEnv *, jfloatArray, jfloat *, jint) ;
        void ReleaseDoubleArrayElements(JNIEnv *, jdoubleArray, jdouble *, jint) ;

        void GetBooleanArrayRegion(JNIEnv *, jbooleanArray, jsize, jsize, jboolean *) ;
        void GetByteArrayRegion(JNIEnv *, jbyteArray, jsize, jsize, jbyte *) ;
        void GetCharArrayRegion(JNIEnv *, jcharArray, jsize, jsize, jchar *) ;
        void GetShortArrayRegion(JNIEnv *, jshortArray, jsize, jsize, jshort *) ;
        void GetIntArrayRegion(JNIEnv *, jintArray, jsize, jsize, jint *) ;
        void GetLongArrayRegion(JNIEnv *, jlongArray, jsize, jsize, jlong *) ;
        void GetFloatArrayRegion(JNIEnv *, jfloatArray, jsize, jsize, jfloat *) ;
        void GetDoubleArrayRegion(JNIEnv *, jdoubleArray, jsize, jsize, jdouble *) ;

        void SetBooleanArrayRegion(JNIEnv *, jbooleanArray, jsize, jsize, jboolean *) ;
        void SetByteArrayRegion(JNIEnv *, jbyteArray, jsize, jsize, jbyte *) ;
        void SetCharArrayRegion(JNIEnv *, jcharArray, jsize, jsize, jchar *) ;
        void SetShortArrayRegion(JNIEnv *, jshortArray, jsize, jsize, jshort *) ;
        void SetIntArrayRegion(JNIEnv *, jintArray, jsize, jsize, jint *) ;
        void SetLongArrayRegion(JNIEnv *, jlongArray, jsize, jsize, jlong *) ;
        void SetFloatArrayRegion(JNIEnv *, jfloatArray, jsize, jsize, jfloat *) ;
        void SetDoubleArrayRegion(JNIEnv *, jdoubleArray, jsize, jsize, jdouble *) ;

        jint RegisterNatives(JNIEnv *, jclass, const JNINativeMethod *, jint) ;
        jint UnregisterNatives(JNIEnv *, jclass) ;

        jint MonitorEnter(JNIEnv *, jobject) ;
        jint MonitorExit(JNIEnv *, jobject) ;

        jint GetJavaVM(JNIEnv *, JavaVM **) ;
	//end JNI functions

        //begin Invocation API functions
        //jint DestroyJavaVM(JavaVM *) ;
        //jint AttachCurrentThread(JavaVM *, void **, void *) ;
        //jint DetachCurrentThread(JavaVM *) ;
        //end Invocation API functions

        ************ JNI-related functions have been made global */


	//code changed 6/8/2005
	//jvm() ;
	jvm(string) ;
	//end of code changed 6/8/2005
	
       	~jvm() ;
       	void init(string, vector<string>&) ;
       	void execute_instruction() ;
       	void run() ;

       	bool IsAssignmentCompatible(Class *, Class *) ;
       	bool IsSuperClass(Class *, Class *) ;
       	bool ImplementsInterface(Class *, Class *) ;

       	object *create_exception_object(string) ;
       	value *create_value(object *) ;
       	void throw_exception(string) ;

       	void aload(int) ;
       	void astore(int) ;
       	void athrow() ;

       	void dload(int) ;
       	void dstore(int) ;

       	value *duplicate(value *) ;
       	bool IsCategory1(value *) ;
       	bool IsCategory2(value *) ;

       	void fload(int) ;
       	void fstore(int) ;

       	void InitialiseClass(Class *) ;

       	void iload(int) ;
       	void istore(int) ;

       	method *GetMethod(Class *, string, string) ;
       	LocalVariable *create_local_variable(value *) ;

       	void EmptyValueStack(stack<value *> *) ;

       	void lload(int) ;
       	void lstore(int) ;

       	object *clone_object(object *) ;
       	primitive_value *clone_primitive_value(primitive_value *) ;
	value *clone_value(value *) ;

        //this vector stores all the dynamic
        //link libraries loaded into the  JVM (future use)
	//code changed 13/5/2004
        //vector<string> libraries ;
        vector<HINSTANCE> native_library_handles ;
	//code changed 13/5/2004

        bool bind_native_method(method *) ;
	
	//code changed 3/8/2005
        //void execute_native_method(method *, value *, stack<value *>&);
	void execute_native_method(method *, value *, stack<value *>&, bool);
	//end of code changed 3/8/2005

        JNIEnv jni_env ;

        JavaVM_ java_vm ;

	//code added 16/5/2004
	//refactoring the code in DefineClass() and load_class()
	//both these methods will call load_class_internal()
	Class *load_class_internal(string class_name, Class *initiating_loader, Class *defining_loader) ;
	//end of code added 16/5/2004

	//code added 1/7/2004
	//(for implementatiom of java.lang.Runtime)
	//TO DO: incorporate these in opcodes.cpp
	bool blnTraceInstructions ;
	bool blnTraceMethodCalls ;

	bool blnRunFinalizersOnExit ;

	vector<thread *> shutdown_hooks ;

	bool blnShuttingDown ; //Vajra is in the process of shutting down

	//TO DO: this object should be initialised at some point
	object *runtime_object ;

	void Shutdown(int) ;

	void RunFinalisation() ;

	//end of code added 1/7/2004

	//code added 21/11/2004
	map<string, string> system_properties ;
	
	void PopulateSystemProperty(string key, string value) ;
	//end of code added 21/11/2004

	//code added 13/2/2005
	object *create_string_object(string s) ;
	//end of code added 13/2/2005
	
	//code added 13/2/2005
	bool blnStringClassInitialised ;
	//end of code added 13/2/2005
	
	//code added 13/2/2005
	void create_system_properties() ;
	//end of code added 13/2/2005

	//code added 25/2/2005
	void setup_string_init(object *obj, string s) ;
	//end of code added 25/2/2005

	//code added 2/3/2005
	bool blnSystemPropertiesCreated ;
	//end of code added 2/3/2005

	//code added 29/5/2005
	LocalVariable *create_local_variable(jvalue *v, string param_type) ;
	//end of code added 29/5/2005

	//code added 6/8/2005
	string classpath ;
	//end of code added 6/8/2005
	
	//code added 7/8/2005
	//this does the job of InitialiseClass() and, in addtion
	//executes the <clinit> method as well
	void InitialiseClass1(Class *) ;
	//end of code added 7/8/2005
	
	//code added 9/8/2005
	object *getClassObject(Class *) ;
	//end of code added 9/8/2005

	//code added 13/8/2005
	object *getMethodObject(method *) ;
	//end of code added 13/8/2005

	//code added 14/8/2005
	//this method creates an object and populates
	//its fields, but does not execute <init>
	//(ideally, the JNI methods NewObject() and NewObjectA()
	//should make calls to this method)
	object *createObject(Class *c) ;
	//end of code added 14/8/2005
        
        //code added 10/9/2005
        //this is to enable all newly created objects to be
        //added to the heap. All direct calls to object's
        //constructor to be replaced with this method
        object *createObject()
        {
                object *obj = new object() ;
                heap.push_back(obj) ;
                return obj ;
        }
        //end of code added 10/9/2005

        //code added 12/9/2005
        //these methods are for ensuring that
        //all the value and primitive_value
        //objects are deleted.
        vector<value *> values ;
        
        value *createValue()
        {
                value *v = new value() ;
                values.push_back(v) ;
                return v ;
        }
        
        vector<primitive_value *> primitiveValues ;
        
        primitive_value *createPrimitiveValue()
        {
                primitive_value *pv = new primitive_value() ;
                primitiveValues.push_back(pv) ;
                return pv ;
        }        
        //end of code added 12/9/2005
        
        //code added 17/9/2005
        void populateLocalVariablesForFrame(jmethodID*, va_list*, vector<LocalVariable *>&) ;
        //end of code added 17/9/2005

        //code added 18/9/2005
        vector<LocalVariable *> localVariables ;

        LocalVariable* createLocalVariable()
        {
                LocalVariable *lv = new LocalVariable() ;
                localVariables.push_back(lv) ;
                return lv ;
        }
        //end of code added 18/9/2005

} ;
#endif
