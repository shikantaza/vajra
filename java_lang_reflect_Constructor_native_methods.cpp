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

#include "java_lang_reflect_Constructor.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

using namespace std ;

JNIEXPORT jclass JNICALL Java_java_lang_reflect_Constructor_getDeclaringClass(JNIEnv *env, jobject obj)
{
  method *ctor = static_cast<method *>(obj) ;

  return static_cast<jclass>(ctor->type) ;
}

JNIEXPORT jstring JNICALL Java_java_lang_reflect_Constructor_getName(JNIEnv *env, jobject obj)
{

  method *ctor = static_cast<method *>(obj) ;

  string s1(ctor->type->name) ;

  return env->NewString(s1.data(), s1.length()) ;
}

JNIEXPORT jint JNICALL Java_java_lang_reflect_Constructor_getModifiers(JNIEnv *env, jobject obj)
{

  method *ctor = static_cast<method *>(obj) ;

  int modifier = 0 ;

  if(ctor->blnPublic == true)
    modifier += 0x0001 ;	

  if(ctor->blnPrivate == true)
    modifier += 0x0002 ;

  if(ctor->blnProtected == true)
    modifier += 0x0004 ;

  if(ctor->blnStatic == true)
    modifier += 0x0008 ;

  if(ctor->blnFinal == true)
    modifier += 0x0010 ;

  if(ctor->blnSynchronized == true)
    modifier += 0x0020 ;

  if(ctor->blnNative == true)
    modifier += 0x0100 ;

  if(ctor->blnAbstract == true)
    modifier += 0x0400 ;

  if(ctor->blnStrict == true)
    modifier += 0x0800 ;

  return modifier ;

}

JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_Constructor_getParameterTypes(JNIEnv *env, jobject obj)
{
  cout << "WARNING: not to be called!" << endl << flush ;
/*	
  method *ctor = static_cast<method *>(obj) ;

  typedef map<key, Class *>::const_iterator CI ;

  CI ci = theJVM->classes.find(key(NULL,"java/lang/Class")) ;  

#if DEBUG_LEVEL >= 3
  assert(ci != theJVM->classes.end()) ;
#endif

  Class *cls = theJVM->classes[ci->first] ;

  //this object is required to satisfy the call to NewObjectArray
  //it will be made redundant when we populate the
  //array with the actual field objects
  jobject some_temp_object = new _jobject() ;

  //create an array object
  jarray params_array = env->NewObjectArray(ctor->parameters->size(),
					   static_cast<jclass>(cls),
					   some_temp_object) ;


  for(int i=0; i<ctor->parameters->size(); i++)
    {

      ci = theJVM->classes.find(key(NULL,ctor->parameters->at(i))) ;  

      Class *param_class = theJVM->classes[ci->first] ;

      env->SetObjectArrayElement(static_cast<jobjectArray>(params_array),
				 i,
				 static_cast<jobject>(param_class->class_object)) ;
    }

  delete some_temp_object ;

  return static_cast<jobjectArray>(params_array) ;
*/
}

JNIEXPORT jint JNICALL Java_java_lang_reflect_Constructor_hashCode(JNIEnv *env, jobject obj)
{
  //we simply return the pointer value of the object
  return (int)(obj) ;
}

JNIEXPORT jstring JNICALL Java_Constructor_toString(JNIEnv *env, jobject obj)
{
  method *ctor = static_cast<method *>(obj) ;

  string str_val ;

  if(ctor->blnPublic == true)
    str_val = "public ";	
  else if(ctor->blnPrivate == true)
    str_val += "private " ;
  else if(ctor->blnProtected == true)
    str_val += "protected " ;

  string ctor_name = ctor->name ;

  util::replace_substring(ctor_name, "/", ".") ;

  str_val += ctor_name ;

  str_val += "(" ;

  for(int i=0; i<ctor->parameters->size(); i++)
    {
      string temp_str = ctor->parameters->at(i) ;

      if(temp_str == "B")
	str_val += "byte" ;
      else if(temp_str == "C")
	str_val += "char" ;
      else if(temp_str == "D")
	str_val += "double" ;
      else if(temp_str == "F")
	str_val += "float" ;
      else if(temp_str == "I")
	str_val += "int" ;
      else if(temp_str == "J")
	str_val += "long" ;
      else if(temp_str == "S")
	str_val += "short" ;
      else if(temp_str == "Z")
	str_val += "boolean" ;
      else if(temp_str.at(0) == 'L') //i.e. it is a reference type
	{

	  string ref_type = temp_str.substr(1,temp_str.length()-2) ; //we need to omit the ';' at the end
	  
	  util::replace_substring(ref_type, "/", ".") ;

	  str_val += ref_type ;

	}
      else if(temp_str.at(0) == '[') // array
	{
	  string array_type ;

	  //count the dimensions
	  int dims = 1 ;

	  for(int j=1; j<temp_str.length(); j++)
	    {
	      if(temp_str.at(j) == '[')
		dims++ ;
	      else
		break ;
	    }

	  array_type = temp_str.substr(dims,temp_str.length()-1) ;

	  if(array_type == "B")
	    str_val += "byte" ;
	  else if(array_type == "C")
	    str_val += "char" ;
	  else if(array_type == "D")
	    str_val += "double" ;
	  else if(array_type == "F")
	    str_val += "float" ;
	  else if(array_type == "I")
	    str_val += "int" ;
	  else if(array_type == "J")
	    str_val += "long" ;
	  else if(array_type == "S")
	    str_val += "short" ;
	  else if(array_type == "Z")
	    str_val += "boolean" ;
	  else if(array_type.at(0) == 'L') //i.e. it is a reference type
	    {

	      //again omitting the ';' at the end	      
	      string array_type2 = array_type.substr(1,array_type.length()-2) ;

	      util::replace_substring(array_type2,"/",".") ;

	      str_val += array_type2 ;

	    }

	  for(int j=0; j<dims; j++)
	    str_val += "[]" ;

	}
	
      //if we have more than one parameter,
      //we need to insert commas, except for the last parameter
      if(i > 0 && i < ctor->parameters->size() - 1)
	str_val += "," ;

    } //end of for

  str_val += ")" ;

  return env->NewString(str_val.data(), str_val.length()) ;
}

JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_Constructor_getExceptionTypes(JNIEnv *env, jobject obj)
{
  method *ctor = static_cast<method *>(obj) ;

  typedef map<key, Class *>::const_iterator CI ;

  CI ci = theJVM->classes.find(key(NULL,"java/lang/Class")) ;  

#if DEBUG_LEVEL >= 3
  assert(ci != theJVM->classes.end()) ;
#endif

  Class *cls = theJVM->classes[ci->first] ;

  //this object is required to satisfy the call to NewObjectArray
  //it will be made redundant when we populate the
  //array with the actual field objects
  jobject some_temp_object = new _jobject() ;

  //create an array object
  jarray exceptions_array = env->NewObjectArray(ctor->exceptions.size(),
						static_cast<jclass>(cls),
						some_temp_object) ;


  for(int i=0; i<ctor->exceptions.size(); i++)
    {

      Class *exception_class = ctor->exceptions.at(i);

      env->SetObjectArrayElement(static_cast<jobjectArray>(exceptions_array),
				 i,
				 static_cast<jobject>(exception_class->class_object)) ;
    }

  delete some_temp_object ;

  return static_cast<jobjectArray>(exceptions_array) ;
  
}

JNIEXPORT jobject JNICALL Java_java_lang_reflect_Constructor_newInstance(JNIEnv *env, 
									 jobject obj,
									 jobjectArray initargs)
{
  method *ctor = static_cast<method *>(obj) ;

  jclass clazz = Java_java_lang_reflect_Constructor_getDeclaringClass(env, obj) ;

  int size = ctor->parameters->size() ;

  //construct an array of jvalue items from the object array
  jvalue *values = new jvalue[size] ;

  //this is wrong; we need to unwrap the primitives correctly; no way of
  //doing this right now as 'object' does not have members for storing primitive
  //data; need to revisit this once those members have been added
  for(int i=0; i < size; i++)
    {
      object *obj = static_cast<object *>(env->GetObjectArrayElement(initargs, i)) ;

      values[i].l = obj ;

    }

  const char *method_name = ctor->name.data() ;
  const char *method_sig = ctor->descriptor.data() ;

  jmethodID mID = env->GetMethodID(clazz,
				   method_name,
				   method_sig) ;

  jobject retval = env->NewObjectA(clazz, mID, values) ;

  delete[] values ;

  return retval ;

}
