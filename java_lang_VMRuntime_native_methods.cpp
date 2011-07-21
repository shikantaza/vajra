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

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "java_lang_VMRuntime.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

using namespace std ;

JNIEXPORT jint JNICALL Java_java_lang_VMRuntime_availableProcessors(JNIEnv *env, jclass cls)
{
  return 1 ;	
}

JNIEXPORT jlong JNICALL Java_java_lang_VMRuntime_freeMemory(JNIEnv *env, jclass cls)
{
  //TO DO: to be implemented when we implement GC
  return 0 ;	
}

JNIEXPORT jlong JNICALL Java_java_lang_VMRuntime_totalMemory(JNIEnv *env, jclass cls)
{
  //TO DO: to be implemented when we implement GC
  return 0 ;	
}

JNIEXPORT jlong JNICALL Java_java_lang_VMRuntime_maxMemory(JNIEnv *env, jclass cls)
{
  //TO DO: to be implemented when we implement GC
  return 0 ;	
}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_gc(JNIEnv *env, jclass cls)
{
  //TO DO: to be implemented when we implement GC
  return ; 	
}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_runFinalization(JNIEnv *env, jclass cls)
{
  theJVM->RunFinalisation() ;
}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_runFinalizationForExit(JNIEnv *env, jclass cls)
{
  //TO DO: convert this to a call to RunFinalisationForExit() method
  //in the 'jvm' class (yet to be written)
  theJVM->RunFinalisation() ;	
}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_traceInstructions(JNIEnv *env, jclass cls, jboolean val)
{
  if(val == JNI_TRUE)
    theJVM->blnTraceInstructions == true ;
  else
    theJVM->blnTraceInstructions == false ;	
}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_traceMethodCalls(JNIEnv *env, jclass cls, jboolean val)
{
  if(val == JNI_TRUE)
    theJVM->blnTraceMethodCalls == true ;
  else
    theJVM->blnTraceMethodCalls == false ;	
}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_runFinalizersOnExit(JNIEnv *env, jclass cls, jboolean val)
{
  //TO DO: security manager related stuff 

  if(val == JNI_TRUE)
    theJVM->blnRunFinalizersOnExit == true ;
  else
    theJVM->blnRunFinalizersOnExit == false ;	
}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_exit(JNIEnv *env, jclass cls, jint status)
{
  //TO DO: security manager stuff

  theJVM->Shutdown(status) ;	
}

JNIEXPORT jint JNICALL Java_java_lang_VMRuntime_nativeLoad(JNIEnv *env, jclass cls, jstring filename)
{
  //TO DO: security manager stuff

#if DEBUG_LEVEL >= 2
  cout << "entering nativeLoad()" << endl << flush ;
#endif	

  const char *library_name = env->GetStringUTFChars(filename, 0) ;	

#if DEBUG_LEVEL >= 2        
  cout << "nativeLoad(): " << library_name << endl << flush ;
#endif
        
  char library_name1[100] ;
  
  memset(library_name1, '\0', 100) ;

  //strcpy(library_name1, "lib") ;
	
  strcat(library_name1, library_name) ;

  HINSTANCE handle ;

#ifdef WIN32
  handle = LoadLibrary(library_name) ;
#else
  //strcat(library_name1, ".so") ;
  handle = dlopen(library_name1, RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
  //cout << library_name1 << endl << flush ;
  assert(handle != NULL) ;
#endif  

  theJVM->native_library_handles.push_back(handle) ;	

  //cout << library_name << endl << flush ;

  env->ReleaseStringUTFChars(filename,library_name) ;  

#if DEBUG_LEVEL >= 2
  cout << "exiting nativeLoad()" << endl << flush ;
#endif

  return 1;

	
}

JNIEXPORT jstring JNICALL Java_java_lang_VMRuntime_nativeGetLibname(JNIEnv *env, 
								    jclass cls,
								    jstring path,
								    jstring short_name)
{
	
#if DEBUG_LEVEL >= 2
  cout << "Entering nativeGetLibName()" << endl << flush ;
#endif	
	
  const char *path1 = env->GetStringUTFChars(path, 0) ;		
  const char *short_name1 = env->GetStringUTFChars(short_name, 0) ;		
	
  char short_name_with_prefix[100] ;
	
  memset(short_name_with_prefix,'\0',100) ;
  strcat(short_name_with_prefix,"lib") ;
  strcat(short_name_with_prefix,short_name1) ;
	
  char full_name[150] ;
  
  memset(full_name, '\0', 150) ;
  
  strcpy(full_name, path1) ;
 
  strcat(full_name,short_name_with_prefix) ;

#ifndef WIN32
  strcat(full_name,".so") ;
#endif  

  string s1(full_name) ;

#if DEBUG_LEVEL >= 2
  cout << "nativeGetLibName(): "  << s1 << endl << flush ;
  cout << "Exiting nativeGetLibName()" << endl << flush ;
#endif

  return env->NewString(s1.data(), s1.length()) ;
  
}

void insertProperty(JNIEnv *env, jobject obj, jmethodID mID, string key, string val)
{
        
#if DEBUG_LEVEL >= 2
  cout << "Entering insertProperty():" << key << ":" << val << endl << flush ;
#endif
        
  jvalue values[2] ;

  LocalVariable *lv1 = theJVM->createLocalVariable() ;
  //lv1->value_type = value::REFERENCE ; 	//there is some problem; dlopen() fails for this .so
						//if this is not commented out
  lv1->ref_value = theJVM->strings[key] ;

#if DEBUG_LEVEL >= 3
  assert(lv1->ref_value != NULL) ;
#endif

  //values[0] = (jvalue *)(lv1) ;
  values[0].l = lv1->ref_value ;

  LocalVariable *lv2 = theJVM->createLocalVariable() ;
  //lv2->value_type = value::REFERENCE ; //see above
  lv2->ref_value = theJVM->strings[val] ;

#if DEBUG_LEVEL >= 3
  assert(lv2->ref_value != NULL) ;
#endif

  //values[1] = (jvalue *)(lv2) ;
  values[1].l = lv2->ref_value ;

  env->CallObjectMethodA(obj, mID, values) ;

#if DEBUG_LEVEL >= 2
  cout << "Exiting insertProperty()" << endl << flush ;
#endif

}

JNIEXPORT void JNICALL Java_java_lang_VMRuntime_insertSystemProperties(JNIEnv *env, 
								       jclass cls,
								       jobject obj)
{
        
#if DEBUG_LEVEL >= 2	
	cout << "Entering insertSystemProperties" << endl << flush ;
#endif
        
	theJVM->create_system_properties() ;

	object *obj1 = static_cast<object *>(obj) ;

	const char method_name[] = "setProperty" ;
	const char method_sig[] = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;" ;

	//get the method ID for setProperty()
        //code changed 24/9/2005
	//jmethodID mID = env->GetMethodID(static_cast<jclass>(obj1->type),
	//				 method_name,
	//				 method_sig) ;
        jmethodID mID = env->GetMethodID((jclass)(theJVM->getClassObject(obj1->type)),
					 method_name,
					 method_sig) ;
        //end of code changed 24/9/2005

		
	//insertProperty(env, obj, mID, "java.version", "1.1") ;
	insertProperty(env, obj, mID, "java.vm.name", "Vajra") ;
	
	//code added 30/7/2005
	insertProperty(env, obj, mID, "java.tmpdir", "/tmp") ;
	//end of code added 30/7/2005

	
	insertProperty(env, obj, mID, "java.vendor","Rajesh Jayaprakash") ;
	insertProperty(env, obj, mID, "java.vendor.url","") ;
	insertProperty(env, obj, mID, "java.home","") ;
	insertProperty(env, obj, mID, "java.vm.specification.version","1.0") ;
	insertProperty(env, obj, mID, "java.vm.specification.vendor","Sun") ;
	insertProperty(env, obj, mID, "java.vm.specification.name","VMSpec") ;
	insertProperty(env, obj, mID, "java.vm.version","0.5") ;
	insertProperty(env, obj, mID, "java.vm.vendor","Rajesh Jayaprakash") ;
	insertProperty(env, obj, mID, "java.vm.name","Vajra") ;
	insertProperty(env, obj, mID, "java.specification.version","1.0") ;
	insertProperty(env, obj, mID, "java.specification.vendor","Classpath") ;
	insertProperty(env, obj, mID, "java.specification.name","Classpath") ;
	insertProperty(env, obj, mID, "java.class.version","") ;
	insertProperty(env, obj, mID, "java.class.path","") ;

	//code changed 30/7/2005
	//insertProperty(env, obj, mID, "java.library.path","") ;
	insertProperty(env, obj, mID, "java.library.path","/usr/local/classpath/lib/classpath") ;
	//end of code changed 30/7/2005
	
	insertProperty(env, obj, mID, "java.io.tmpdir","") ;
	insertProperty(env, obj, mID, "java.compiler","") ;
	insertProperty(env, obj, mID, "java.ext.dirs","") ;
	insertProperty(env, obj, mID, "os.name","Linux") ;
	insertProperty(env, obj, mID, "os.arch","x86") ;
	insertProperty(env, obj, mID, "os.version","") ;
#ifndef WIN32
	insertProperty(env, obj, mID, "file.separator","/") ;
	insertProperty(env, obj, mID, "path.separator",":") ;
#else
	insertProperty(env, obj, mID, "file.separator","\\") ;
	insertProperty(env, obj, mID, "path.separator",";") ;
#endif
	insertProperty(env, obj, mID, "line.separator","\n") ;
	insertProperty(env, obj, mID, "user.name","") ;
	insertProperty(env, obj, mID, "user.home","") ;
	insertProperty(env, obj, mID, "user.dir","") ;	

#if DEBUG_LEVEL >= 2
	cout << "Exiting insertSystemProperties" << endl << flush ;
#endif

	return ;

}
