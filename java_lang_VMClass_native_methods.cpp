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

#include "java_lang_VMClass.h"

#include "jvm.h"
#include "util.h"

#include <cassert>

using namespace std ;

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

//TO DO: finalise how to handle exceptions


extern "C" JNIEXPORT jclass JNICALL forName_internal
  (JNIEnv *env, jclass cls, jstring class_name1, jboolean blnInitialise, jobject classLoader)
{

  //TO DO: take care of this when you look into security domains
  //From the J2SE Platform API documentation:
  //If the loader is null, and a security manager is present, 
  //and the caller's class loader is not null, then this
  //method calls the security manager's checkPermission method 
  //with a RuntimePermission("getClassLoader") permission to ensure 
  //it's ok to access the bootstrap class loader.
	
  string class_name(env->GetStringUTFChars(class_name1, 0)) ;	
	
  util::replace_substring(class_name, ".", "/") ;
	
  //if classLoader is null, use the bootstrap loader
  //else use it to load the class
  object *classLoaderObj = static_cast<object *>(classLoader) ;

  object *nullObj = theJVM->createObject() ;
  nullObj->blnNull = true ;	
	
  object *obj = (classLoaderObj == NULL) ?  nullObj : classLoaderObj ;	
	
  if(obj->blnNull == true)
    {

      if(theJVM->classes.find(key(NULL,class_name)) == theJVM->classes.end()) 
	//i.e. the class is not already loaded
	{
	  try
	    {
	      theJVM->load_class(class_name) ;
	    }
	  catch(NoClassDefFound)
	    {
	      theJVM->throw_exception("java/lang/ClassNotFoundException") ;
	      return NULL ;
	    }
	}
    }
  else
    {

      const char method_name[] = "loadClass" ;
      const char method_sig[] = "Ljava/lang/Class;(Ljava/lang/String;)" ;

      //get the method ID for loadClass()
      //code changed 24/9/2005
      //jmethodID mID = env->GetMethodID(static_cast<jclass>(obj->type),
      //			       method_name,
      //			       method_sig) ;
      jmethodID mID = env->GetMethodID((jclass)theJVM->getClassObject(obj->type),
				       method_name,
				       method_sig) ;
      //end of code changed 24/9/2005


      //invoke the instance method
      //ClassLoader will thus populate the "classes" map of the 'jvm'
      //class with this class
      jobject loadedClass = env->CallObjectMethod(classLoader, mID) ;

    }

  //link the class
  typedef map<key, Class *>::const_iterator CI ;

  CI ci = theJVM->classes.find(key(obj->type,class_name)) ;  

#if DEBUG_LEVEL >= 3
  assert(ci != theJVM->classes.end()) ;
#endif

  Class *cls1 = theJVM->classes[ci->first] ;

  theJVM->link_class(cls1) ;

  //if blnInitialise is true, initialise the class if it is not an array class
  //if it is an array class, don't initialise it
  if(blnInitialise == JNI_TRUE && cls1->blnArrayClass == false)
  {
	try
	{
		theJVM->InitialiseClass1(cls1) ;
	}
	catch(VerifyError)
	{
		theJVM->throw_exception("java/lang/VerifyError") ;
		return NULL ;		
	}
	catch(LinkageError)
	{
		theJVM->throw_exception("java/lang/LinkageError") ;
		return NULL ;		
	}		

  }

  return (jclass)theJVM->getClassObject(cls1) ;
}

extern "C" JNIEXPORT jclass JNICALL Java_java_lang_VMClass_forName__Ljava_lang_String_2
(JNIEnv *env, jclass cls, jstring str)


{

#if DEBUG_LEVEL >= 2
	cout << "Entering forName()" << endl << flush ;
#endif

  //code changed 24/9/2005
  //Class *cls1 = static_cast<Class *>(cls) ;
  object* obj1 = (object*)(cls) ;
  Class* cls1 = obj1->class_pointer ;
  //end of code changed 24/9/2005

  object *nullObj = theJVM->createObject() ;
  nullObj->blnNull = true ;	
  object *obj ;
	
  if(cls1->defining_loader == NULL)
  	obj =  nullObj ;
  else
	obj = static_cast<object *>(cls1->defining_loader->class_object) ;	
	
  jclass temp = forName_internal(env, 
				 cls, 
				 str, 
				 JNI_TRUE,
			         (jobject)obj) ;
#if DEBUG_LEVEL >= 2
	cout << "Exiting forName()" << endl << flush ;
#endif  
  
  return temp ;
}

extern "C" JNIEXPORT jboolean JNICALL Java_java_lang_VMClass_isPrimitive(JNIEnv *env, jclass this_class, jclass cls)
{

  //object *obj1 = theJVM->getClassObject(static_cast<Class *>(cls)) ;
  object *obj1 = (object *)(cls) ;

  Class *cls1 = obj1->class_pointer ;

  if(cls1->blnPrimitiveClass == true)
    return JNI_TRUE ;
  else
    return JNI_FALSE ;
}

JNIEXPORT jstring JNICALL Java_java_lang_VMClass_getName
  (JNIEnv *env, jclass this_class, jclass cls1)
{
	object *obj1 = (object *)(cls1) ;
	
	Class *cls = obj1->class_pointer ;
cout << "VMClass.getName(): " << cls->name << endl << flush ;
	string s1(cls->name) ;

	util::replace_substring(s1, "/", ".") ;

	return env->NewString(s1.data(), s1.length()) ;	
}

JNIEXPORT jobjectArray JNICALL Java_java_lang_VMClass_getDeclaredConstructors
  (JNIEnv *env, jclass this_class, jclass cls1, jboolean publicCtorsOnly)
{
  object *obj1 = (object *)(cls1) ;

  Class *c = obj1->class_pointer;
//cout << "VMClass.getDeclaredConstructors(): " << c->name << endl << flush ;

  bool blnPublic = (publicCtorsOnly == JNI_TRUE) ? true : false ;	

  //this is the grab bag for storing all the publically accessible methods of the class
  //when we create the object array (using env->NewObjectArray), we need to know how many
  //objects will be stored in that; hence the need for this 

  //code changed 19/2/2005
  //map<method_key, method *> ctors_temp ;
  map<string, method *> ctors_temp ;
  //end of code changed 19/2/2005

  //get the methods of this class first..
  //code changed 19/2/2005
  //typedef map<method_key, method *>::const_iterator CI1 ;
  typedef map<string, method *>::const_iterator CI1 ;
  //end of code changed 19/2/2005

  for(CI1 p=c->methods.begin(); p!=c->methods.end(); p++)
    {

      if(c->methods[p->first]->blnPublic == blnPublic &&
	 c->methods[p->first]->name == "<init>")
	{
	  ctors_temp[p->first] = c->methods[p->first];
	}
    }

    //cout << "VMClass.getDeclaredConstructors(): " << ctors_temp.size() << endl << flush ;
    
  typedef map<key, Class *>::const_iterator CI ;

  CI ci = theJVM->classes.find(key(NULL,"java/lang/reflect/Constructor")) ;  

#if DEBUG_LEVEL >= 3
  assert(ci != theJVM->classes.end()) ;
#endif

  Class *ctor_cls = theJVM->classes[ci->first] ;


  //this object is required to satisfy the call to NewObjectArray
  //it will be made redundant when we populate the
  //array with the actual field objects
  jobject some_temp_object = new _jobject() ;

  //create an array object
  //code changed 24/9/2005
  //jarray ctors_array = env->NewObjectArray(ctors_temp.size(),
  //              			     static_cast<jclass>(ctor_cls),
  //                                         some_temp_object) ;
  jarray ctors_array = env->NewObjectArray(ctors_temp.size(),
					   (jclass)theJVM->getClassObject(ctor_cls),
					   some_temp_object) ;
  //code changed 24/9/2005


  int counter = 0 ;

  for(CI1 p=ctors_temp.begin(); p!=ctors_temp.end(); p++, counter++)

    {
      env->SetObjectArrayElement(static_cast<jobjectArray>(ctors_array),
				 counter,
				 theJVM->getMethodObject(ctors_temp[p->first])) ;
    }

  delete some_temp_object ;

  return static_cast<jobjectArray>(ctors_array) ;	
}

//the definition(s) for Constructor's native methods are here
//just temporariliy, to circumvent the seg fault that occurs
//if we load Constructor's .so file
/*
JNIEXPORT jobjectArray JNICALL Java_java_lang_reflect_Constructor_getParameterTypes(JNIEnv *env, jobject obj)
{
  cout << "Entering Constructor.getParameterTypes()" << endl << flush ;
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

}
*/

JNIEXPORT jobject JNICALL Java_java_lang_reflect_Constructor_constructNative(JNIEnv *env, 
							   		     jobject this_object,
							   		     jobjectArray args,
							   		     jclass cls, 
                                                                             jint slot)
{
	//TO DO: handle the various exceptions
	
	object *obj1 = (object *)this_object ;

	method *m = obj1->method_pointer ;
        
        //cout << "Constructor.constructNative(): " << m->name << "(" << m->descriptor << ")" << endl << flush ;

        object *tempObj = (object *)cls ;
        Class *c = tempObj->class_pointer ;
        
	//cout << "Constructor.constructNative(): " << c << endl << flush ;
        //cout << "Constructor.constructNative(): " << c->name << endl << flush ;
        //cout << "Constructor.constructNative(): " << m->max_locals << endl << flush ;

	frame *fm = new frame(m) ;

	fm->local_variables.reserve(m->max_locals + 1) ;
        
	for(int i=0; i<m->max_locals + 1; i++)
	  fm->local_variables.push_back(NULL) ;

	//create the new object so that <init>
	//can be invoked on it
	object *newObj = theJVM->createObject() ;
	newObj->type = c ;

        
        //push this_object into local variables array
        LocalVariable *lv = theJVM->createLocalVariable() ;
        lv->value_type = value::REFERENCE ;
        lv->ref_value = newObj ;
        
        fm->local_variables.at(0) = lv ;       
        
        //push the args into the local variables array (note the
        //starting value of the index (1)
        object *arrayObject = (object *)(args) ;

        //cout << "Constructor.constructNative(): " << arrayObject->items.size() << endl << flush ;
        
        for(int i=1, j=0; j<arrayObject->items.size(); i++, j++)
        {
                lv = theJVM->createLocalVariable() ;
                lv->value_type = value::REFERENCE ;
                lv->ref_value = arrayObject->items.at(j) ;
                //lv->ref_value->display() ;
                fm->local_variables.at(i) = lv ;
        }
        
	fm->prev_pc_value = theJVM->threads.at(theJVM->curr_thread)->pc ;

	frame *prev_frame = theJVM->threads.at(theJVM->curr_thread)->java_stack.top() ;
		
	theJVM->threads.at(theJVM->curr_thread)->java_stack.push(fm) ;
	theJVM->threads.at(theJVM->curr_thread)->pc = 0 ;

	while(theJVM->threads.at(theJVM->curr_thread)->java_stack.top() != prev_frame)	
		theJVM->execute_instruction() ;
        
        return (jobject)newObj ;
	
}
