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

#include "java_lang_Runtime.h"

#include "jvm.h"

using namespace std ;

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_traceInstructions(JNIEnv *env, jobject obj, jboolean val)
{
  if(val == JNI_TRUE)
    theJVM->blnTraceInstructions == true ;
  else
    theJVM->blnTraceInstructions == false ;
}


extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_traceMethodCalls(JNIEnv *env, jobject obj, jboolean val)
{
  if(val == JNI_TRUE)
    theJVM->blnTraceMethodCalls == true ;
  else
    theJVM->blnTraceMethodCalls == false ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_runFinalizersOnExit(JNIEnv *env, jclass cls, jboolean val)
{

  //TO DO: security manager related stuff 

  if(val == JNI_TRUE)
    theJVM->blnRunFinalizersOnExit == true ;
  else
    theJVM->blnRunFinalizersOnExit == false ;
}

extern "C" JNIEXPORT jlong JNICALL Java_java_lang_Runtime_freeMemomry(JNIEnv *env, jobject obj)
{
  //TO DO: to be implemented when we implement GC
  return 0 ;
}

extern "C" JNIEXPORT jlong JNICALL Java_java_lang_Runtime_maxMemory(JNIEnv *env, jobject obj)
{
  //TO DO: to be implemented when we implement GC
  return 0 ;
}

extern "C" JNIEXPORT jlong JNICALL Java_java_lang_Runtime_totalMemory(JNIEnv *env, jobject obj)
{
  //TO DO: to be implemented when we implement GC
  return 0 ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_gc(JNIEnv *env, jobject obj)
{
  //TO DO: to be implemented when we implement GC
  return ; 
}

extern "C" JNIEXPORT jint JNICALL Java_java_lang_Runtime_availableProcessors(JNIEnv *env, jobject obj)
{
  return 1 ;
}

extern "C" JNIEXPORT jobject JNICALL Java_java_lang_Runtime_getLocalizedInputStream(JNIEnv *env,
										    jobject obj,
										    jobject instream)
{
  //we are not doing any localisation
  return instream ;
}

extern "C" JNIEXPORT jobject JNICALL Java_java_lang_Runtime_getLocalizedOutputStream(JNIEnv *env,
									   jobject obj,
									   jobject outstream)
{
  //we are not doing any localisation
  return outstream ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_addShutdownHook(JNIEnv *env,
									 jobject obj,
									 jobject thread_obj)
{

  //TO DO: security manager stuff

  thread *thrd = static_cast<thread *>(thread_obj) ;

  //to prevent the same thread being added more than once
  for(int i=0; i<theJVM->shutdown_hooks.size(); i++)
    {
      if(theJVM->shutdown_hooks.at(i) == thrd)
	{
	  theJVM->throw_exception("java/lang/IllegalArgumentException") ;
	  return ;
	}
    }

  theJVM->shutdown_hooks.push_back(thrd) ;

}

extern "C" JNIEXPORT jboolean JNICALL Java_java_lang_Runtime_removeShutdownHook(JNIEnv *env,
										jobject obj,
										jobject thread_obj)
{
  //TO DO: security manager stuff

  thread *thrd = static_cast<thread *>(thread_obj) ;

  if(theJVM->blnShuttingDown == true)
    {
      theJVM->throw_exception("java/lang/IllegalStateException") ;
      return JNI_FALSE ;
    }

  //delete the thread from the 'shutdown_hooks' vector
  theJVM->shutdown_hooks.erase(find(theJVM->shutdown_hooks.begin(),
				    theJVM->shutdown_hooks.end(),
				    thrd)) ;


  return JNI_TRUE ;
}

extern "C" JNIEXPORT jobject JNICALL Java_java_lang_Runtime_getRuntime(JNIEnv *env, jclass cls)
{
  return static_cast<jobject>(theJVM->runtime_object) ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_load(JNIEnv *env, jobject obj, jstring filename)
{
  //TO DO: security manager stuff

  const char *library_name = env->GetStringUTFChars(filename, 0) ;	

  HINSTANCE handle ;

#ifdef WIN32
  handle = LoadLibrary(library_name) ;
#else
  handle = dlopen(library_name, RTLD_LAZY) ;
#endif

  theJVM->native_library_handles.push_back(handle) ;	

  env->ReleaseStringUTFChars(filename,library_name) ;  

  return ;


}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_loadLibrary(JNIEnv *env, jobject obj, jstring libname)
{

  //TO DO: security manager stuff

  const char *library_name = env->GetStringUTFChars(libname, 0) ;	

  char *library_name1 ;

  strcpy(library_name1,library_name) ;

  HINSTANCE handle ;

#ifdef WIN32
  handle = LoadLibrary(library_name) ;
#else
  strcat(library_name1,".so") ;
  handle = dlopen(library_name1, RTLD_LAZY) ;
#endif

  theJVM->native_library_handles.push_back(handle) ;	

  env->ReleaseStringUTFChars(libname,library_name) ;  

  return ;

}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_halt(JNIEnv *env, jobject obj, jint status)
{
  //TO DO: security manager stuff

  exit(status) ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_exit(JNIEnv *env, jobject obj, jint status)
{
  //TO DO: security manager stuff

  theJVM->Shutdown(status) ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Runtime_runFinalization(JNIEnv *env, jobject obj)
{
  theJVM->RunFinalisation() ;
}
