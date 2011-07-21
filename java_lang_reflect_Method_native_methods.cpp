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

#include "java_lang_reflect_Method.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

using namespace std ;

JNIEXPORT jboolean JNICALL Java_java_lang_reflect_Method_equals(JNIEnv *env, jobject this_method, jobject that_method)
{
  method *this_method1 = static_cast<method *>(this_method) ;

  method *that_method1 = static_cast<method *>(that_method) ;

  if(this_method1->type == that_method1->type &&
     this_method1->name == that_method1->name &&
     this_method1->descriptor == that_method1->descriptor)
    return JNI_TRUE ;
  else
    return JNI_FALSE ;

}

JNIEXPORT jclass JNICALL Java_java_lang_reflect_Method_getDeclaringClass(JNIEnv *env, jobject obj)
{
  method *mthd = static_cast<method *>(obj) ;

  return static_cast<jclass>(mthd->type) ;
}

JNIEXPORT jstring JNICALL Java_java_lang_reflect_Method_getName(JNIEnv *env, jobject obj)
{

  method *mthd = static_cast<method *>(obj) ;

  string s1(mthd->type->name) ;

  return env->NewString(s1.data(), s1.length()) ;
}

JNIEXPORT jint JNICALL Java_java_lang_reflect_Method_getModifiers(JNIEnv *env, jobject obj)
{

  method *mthd = static_cast<method *>(obj) ;

  int modifier = 0 ;

  if(mthd->blnPublic == true)
    modifier += 0x0001 ;	

  if(mthd->blnPrivate == true)
    modifier += 0x0002 ;

  if(mthd->blnProtected == true)
    modifier += 0x0004 ;

  if(mthd->blnStatic == true)
    modifier += 0x0008 ;

  if(mthd->blnFinal == true)
    modifier += 0x0010 ;

  if(mthd->blnSynchronized == true)
    modifier += 0x0020 ;

  if(mthd->blnNative == true)
    modifier += 0x0100 ;

  if(mthd->blnAbstract == true)
    modifier += 0x0400 ;

  if(mthd->blnStrict == true)
    modifier += 0x0800 ;

  return modifier ;

}

JNIEXPORT jclass JNICALL Java_java_lang_reflect_Method_getReturnType(JNIEnv *env, jobject obj)
{

  method *mthd = static_cast<method *>(obj) ;

  return(util::getClassFromName(theJVM,
				mthd->return_descriptor,
				mthd->type->defining_loader)) ;
}

JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_Method_getParameterTypes(JNIEnv *env, jobject obj)
{
  method *mthd = static_cast<method *>(obj) ;

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
  jarray params_array = env->NewObjectArray(mthd->parameters->size(),
					   static_cast<jclass>(cls),
					   some_temp_object) ;


  for(int i=0; i<mthd->parameters->size(); i++)
    {

      ci = theJVM->classes.find(key(NULL,mthd->parameters->at(i))) ;  

      Class *param_class = theJVM->classes[ci->first] ;

      env->SetObjectArrayElement(static_cast<jobjectArray>(params_array),
				 i,
				 static_cast<jobject>(param_class->class_object)) ;
    }

  delete some_temp_object ;

  return static_cast<jobjectArray>(params_array) ;

}

JNIEXPORT jint JNICALL Java_java_lang_reflect_Method_hashCode(JNIEnv *env, jobject obj)
{
  //we simply return the pointer value of the object
  return (int)(obj) ;
}

JNIEXPORT jstring JNICALL Java_java_lang_reflect_Method_toString(JNIEnv *env, jobject obj)
{
  method *mthd = static_cast<method *>(obj) ;

  string str_val ;

  if(mthd->blnPublic == true)
    str_val = "public ";	
  else if(mthd->blnPrivate == true)
    str_val += "private " ;
  else if(mthd->blnProtected == true)
    str_val += "protected " ;

  str_val += util::formatDescriptor(mthd->return_descriptor) ;

  string mthd_name = mthd->name ;

  util::replace_substring(mthd_name, "/", ".") ;

  str_val += mthd_name ;

  str_val += "(" ;

  for(int i=0; i<mthd->parameters->size(); i++)
    {
      string temp_str = mthd->parameters->at(i) ;

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
      if(i > 0 && i < mthd->parameters->size() - 1)
	str_val += "," ;

    } //end of for

  str_val += ")" ;

  return env->NewString(str_val.data(), str_val.length()) ;
}


JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_Method_getExceptionTypes(JNIEnv *env, jobject obj)
{
  method *mthd = static_cast<method *>(obj) ;

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
  jarray exceptions_array = env->NewObjectArray(mthd->exceptions.size(),
						static_cast<jclass>(cls),
						some_temp_object) ;


  for(int i=0; i<mthd->exceptions.size(); i++)
    {

      Class *exception_class = mthd->exceptions.at(i);

      env->SetObjectArrayElement(static_cast<jobjectArray>(exceptions_array),
				 i,
				 static_cast<jobject>(exception_class->class_object)) ;
    }

  delete some_temp_object ;

  return static_cast<jobjectArray>(exceptions_array) ;
  
}

JNIEXPORT jobject JNICALL Java_java_lang_reflect_Method_invoke(JNIEnv *env,
							       jobject mthdobj,
							       jobject obj,
							       jobjectArray args)
{
	
  //TO DO: verify access control
	
  method *mthd = static_cast<method *>(mthdobj) ;
  object *obj1 = static_cast<object *>(obj) ;

  int size = mthd->parameters->size() ;

  Class *clazz = mthd->type ;

  if(mthd->blnStatic == true)
  {
    if(clazz->blnInitialised == false)
    {
	theJVM->InitialiseClass(clazz) ;
    }
  }
  else
  {
    if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
    }
    
    if(!(obj1->type != clazz ||
      theJVM->IsSuperClass(clazz, obj1->type) == true ||
      theJVM->ImplementsInterface(obj1->type, clazz) == true))    
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
    }
  }

  if(env->GetArrayLength(args) != size) //actual and formal parameter numbers
  					//are different
  {
    theJVM->throw_exception("java/lang/IllegalArgumentException") ;
  }
      	
  //TO DO: cover the other ways by which 
  //IllegalArgumentException can be thrown
  
  jvalue *values = new jvalue[size] ;

  //this is wrong; we need to unwrap the primitives correctly;
  for(int i=0; i < size; i++)
    {
      object *obj = static_cast<object *>(env->GetObjectArrayElement(args, i)) ;

      values[i].l = obj ;

    }  
  
  const char *method_name = mthd->name.data() ;
  const char *method_sig = mthd->descriptor.data() ;

  jobject obj_retval ;
  jboolean bool_retval ;
  jbyte byte_retval ;
  jchar char_retval ;
  jshort short_retval ;
  jint int_retval ;
  jlong long_retval ;
  jfloat float_retval ;
  jdouble double_retval ;

  jmethodID mID = env->GetMethodID(clazz,
				   method_name,
				   method_sig) ;

  string temp_str = mthd->return_descriptor ;

  if(mthd->blnStatic == true)
  {

    if(temp_str == "B")
    {
      byte_retval = env->CallStaticByteMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *byte_obj = theJVM->createObject() ;
      
      byte_obj->isPrimitive = true ;
      byte_obj->byte_val = byte_retval ;

      return (jobject)byte_obj ;
    }
    else if(temp_str == "C")
    {
      char_retval = env->CallStaticCharMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *char_obj = theJVM->createObject() ;
      
      char_obj->isPrimitive = true ;
      char_obj->char_val = char_retval ;

      return (jobject)char_obj ;
    }
    else if(temp_str == "D")
    {
      double_retval = env->CallStaticDoubleMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *double_obj = theJVM->createObject() ;
      
      double_obj->isPrimitive = true ;
      double_obj->double_val = double_retval ;
      
      return (jobject)double_obj ;
    }
    else if(temp_str == "F")
    {
      float_retval = env->CallStaticFloatMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *float_obj = theJVM->createObject() ;
      
      float_obj->isPrimitive = true ;
      float_obj->float_val = float_retval ;

      return (jobject)float_obj ;
    }
    else if(temp_str == "I")
    {
      int_retval = env->CallStaticIntMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *int_obj = theJVM->createObject() ;
      
      int_obj->isPrimitive = true ;
      int_obj->int_val = int_retval ;

      return (jobject)int_obj ;
    }
    else if(temp_str == "J")
    {
      long_retval = env->CallStaticLongMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *long_obj = theJVM->createObject() ;
      
      long_obj->isPrimitive = true ;
      long_obj->long_val = long_retval ;

      return (jobject)long_obj ; 
    }
    else if(temp_str == "S")
    {
      short_retval = env->CallStaticShortMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *short_obj = theJVM->createObject() ;
      
      short_obj->isPrimitive = true ;
      short_obj->short_val = short_retval ;

      return (jobject)short_obj ;
    }
    else if(temp_str == "Z")
    {
      bool_retval = env->CallStaticBooleanMethodA(clazz, mID, values) ;
      delete[] values ;
      
      object *bool_obj = theJVM->createObject() ;
      
      bool_obj->isPrimitive = true ;
      bool_obj->bool_val = bool_retval ;

      return (jobject)bool_obj ;
    }
    else if(temp_str.at(0) == 'L' || //i.e. it is a reference type
            temp_str.at(0) == '[')   // or array
    {
      obj_retval = env->CallStaticObjectMethodA(clazz, mID, values) ;
      delete[] values ;
      return obj_retval ;        
    }
    else //it is a void return type
    {
      env->CallStaticVoidMethodA(clazz, mID, values) ;
      delete[] values ;
      return NULL ;        
    }    
  	
  	
  }
  else //it is an instance method
  {
    if(temp_str == "B")
    {
      byte_retval = env->CallByteMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *byte_obj = theJVM->createObject() ;
      
      byte_obj->isPrimitive = true ;
      byte_obj->byte_val = byte_retval ;

      return (jobject)byte_obj ;
    }
    else if(temp_str == "C")
    {
      char_retval = env->CallCharMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *char_obj = theJVM->createObject() ;
      
      char_obj->isPrimitive = true ;
      char_obj->char_val = char_retval ;

      return (jobject)char_obj ;
    }
    else if(temp_str == "D")
    {
      double_retval = env->CallDoubleMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *double_obj = theJVM->createObject() ;
      
      double_obj->isPrimitive = true ;
      double_obj->double_val = double_retval ;

      return (jobject)double_obj ;
    }
    else if(temp_str == "F")
    {
      float_retval = env->CallFloatMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *float_obj = theJVM->createObject() ;
      
      float_obj->isPrimitive = true ;
      float_obj->float_val = float_retval ;

      return (jobject)float_obj ;
    }
    else if(temp_str == "I")
    {
      int_retval = env->CallIntMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *int_obj = theJVM->createObject() ;
      
      int_obj->isPrimitive = true ;
      int_obj->int_val = int_retval ;

      return (jobject)int_obj ;
    }
    else if(temp_str == "J")
    {
      long_retval = env->CallLongMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *long_obj = theJVM->createObject() ;
      
      long_obj->isPrimitive = true ;
      long_obj->long_val = long_retval ;

      return (jobject)long_obj ;
    }
    else if(temp_str == "S")
    {
      short_retval = env->CallShortMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *short_obj = theJVM->createObject() ;
      
      short_obj->isPrimitive = true ;
      short_obj->short_val = short_retval ;

      return (jobject)short_obj ;
    }
    else if(temp_str == "Z")
    {
      bool_retval = env->CallBooleanMethodA(obj, mID, values) ;
      delete[] values ;
      
      object *bool_obj = theJVM->createObject() ;
      
      bool_obj->isPrimitive = true ;
      bool_obj->bool_val = bool_retval ;

      return (jobject)bool_obj ;
    }
    else if(temp_str.at(0) == 'L' || //i.e. it is a reference type
            temp_str.at(0) == '[')   // or array
    {
      obj_retval = env->CallObjectMethodA(obj, mID, values) ;
      delete[] values ;
      return obj_retval ;        
    }
    else //it is a void return type
    {
      env->CallVoidMethodA(obj, mID, values) ;
      delete[] values ;
      return NULL ;        
    }    
 	
  } 

}
