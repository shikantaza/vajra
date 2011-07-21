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

#include "java_lang_Thread.h"

#include "jvm.h"

#include "util.h"

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

extern "C" JNIEXPORT jboolean JNICALL Java_java_lang_Thread_isDaemon(JNIEnv *env, jobject obj)
{
  thread *thrd = static_cast<thread *>(obj) ;

  if(thrd->blnDaemon == true)
    return JNI_TRUE ;
  else
    return JNI_FALSE ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_setDaemon(JNIEnv *env, jobject obj, jboolean val)
{

  //TO DO: security manager stuff

  thread *thrd = static_cast<thread *>(obj) ;

  if(val == JNI_TRUE)
    thrd->blnDaemon = true ;
  else
    thrd->blnDaemon = false ;
}

extern "C" JNIEXPORT jboolean JNICALL Java_java_lang_Thread_isAlive(JNIEnv *env, jobject obj)
{
  thread *thrd = static_cast<thread *>(obj) ;

  if(thrd->blnAlive == true)
    return JNI_TRUE ;
  else
    return JNI_FALSE ;
}

extern "C" JNIEXPORT jstring JNICALL Java_java_lang_Thread_getName(JNIEnv *env, jobject obj)
{
  thread *thrd = static_cast<thread *>(obj) ;

  string s1(thrd->name) ;

  return env->NewString(s1.data(), s1.length()) ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_setName(JNIEnv *env, jobject obj, jstring name)
{
  thread *thrd = static_cast<thread *>(obj) ;

  string thread_name(env->GetStringUTFChars(name, 0)) ;	

  thrd->name = thread_name ;
}

extern "C" JNIEXPORT jint JNICALL Java_java_lang_Thread_getPriority(JNIEnv *env, jobject obj)
{
  thread *thrd = static_cast<thread *>(obj) ;

  return thrd->priority ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_setPriority(JNIEnv *env, jobject obj, jint priority)
{
  thread *thrd = static_cast<thread *>(obj) ;

  thrd->priority = priority ;
}

extern "C" JNIEXPORT jboolean JNICALL Java_java_lang_Thread_holdsLock(JNIEnv *env, jclass cls, jobject obj)
{
  thread *current_thread = theJVM->threads.at(theJVM->curr_thread) ;

  object *obj_to_be_compared = static_cast<object *>(obj) ;

  if(obj_to_be_compared->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return JNI_FALSE ;
    }

  //this is a very inefficient way of doing things.
  //we need to scan all the frames in the current thread
  //and check if any of these frames have acquired a lock on the
  //given object. the frames are implemented using stacks; unfortunately
  //there are no iterators available for stacks, since they are adapter
  //containers. we therefore create a vector (after cloning the stack)
  //and use this vector to inspect the frames

  stack<frame *> temp_stack(current_thread->java_stack) ;

  vector<frame *> temp_vector ;

  while(temp_stack.empty() == false)
    {
      temp_vector.push_back(temp_stack.top()) ;
      temp_stack.pop() ;
    }

  for(int i = 0; i < temp_vector.size() ; i++)
  {
    if(temp_vector.at(i)->mon_obj == obj_to_be_compared)
      return JNI_TRUE ;
  }

  return JNI_FALSE ;
}

extern "C" JNIEXPORT jboolean JNICALL Java_java_lang_Thread_isInterrupted(JNIEnv *env, jobject obj)
{
  thread *thrd = static_cast<thread *>(obj) ;

  if(thrd->blnInterrupted == true)
    {
      thrd->blnInterrupted = false ;
      return JNI_TRUE ;
    }
  else
    return JNI_FALSE ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_sleep__J(JNIEnv *env, jclass cls, jlong sleeptime)
{
  Java_java_lang_Thread_sleep__JI(env, cls, sleeptime, 0) ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_sleep__JI(JNIEnv *env,
								  jclass cls,
								  jlong sleeptime_millisecs,
								  jint sleeptime_nanosecs)
{
  thread *current_thread = theJVM->threads.at(theJVM->curr_thread) ;

  if(current_thread->blnInterrupted == true)
    {
      current_thread->blnInterrupted = false ;
      theJVM->throw_exception("java/lang/InterruptedException") ;
      return ;
    }

  current_thread->blnAskedToSleep = true ;
  current_thread->sleepMilliSecs = sleeptime_millisecs ;
  current_thread->sleepMilliSecs = sleeptime_nanosecs ;

  return ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_dumpStack(JNIEnv *env, jclass cls)
{

  thread *current_thread = theJVM->threads.at(theJVM->curr_thread) ;

  stack<string> new_stack ;

  util::reverse_stack(&(current_thread->exception_trace), &new_stack) ;

  while(!(new_stack.empty()))
    {
      cout << new_stack.top() << endl ;
      new_stack.pop() ;
    }
}

extern "C" JNIEXPORT jint JNICALL Java_java_lang_Thread_countStackFrames(JNIEnv *env, jobject obj)
{
  thread *current_thread = theJVM->threads.at(theJVM->curr_thread) ;

  if(current_thread->blnSuspended == true)
    {
      current_thread->blnInterrupted = false ;
      theJVM->throw_exception("java/lang/IllegalThreadStateException") ;
      return 0 ;
    }

  return current_thread->java_stack.size() ;

}

//commented out because there is no need for a
//native method for this (instead implemented as currentThread().getThreadGroup().activeCount())
//WARNING: commented out before compiling, so be prepared if you decide
//to uncomment it
/*
extern "C" JNIEXPORT jint JNICALL Java_Thread_activeCount(JNIEnv *env, jclass cls)
{

  //we first get the ThreadGroup object to which the current thread
  //belongs, then get the activeCount from that ThreadGroup object

  thread *current_thread = theJVM->threads.at(theJVM->curr_thread) ;

  const char method_name1[] = "getThreadGroup" ;
  const char method_sig1[] = "Ljava/lang/ThreadGroup;()" ;

  //get the method ID for loadClass()
  jmethodID mID1 = env->GetMethodID(cls,
				    method_name1,
				    method_sig1) ;


  jobject threadGroup = env->CallObjectMethod(static_cast<jobject>(current_thread), mID1) ;

  const char method_name2[] = "activeCount" ;
  const char method_sig2[] = "I()" ;

  //get the method ID for activeCount()
  jmethodID mID2 = env->GetMethodID(static_cast<jclass>(threadGroup->type),
				    method_name2,
				    method_sig2) ;


  return env->CallObjectMethod(threadGroup, mID2) ;


}
*/

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_checkAccess(JNIEnv *env, jobject obj)
{
  //TO DO: implement this method when we do the
  //security manager stuff
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_destroy(JNIEnv *env, jobject obj)
{
  //since this method is not implemented even in Sun's library (per API doculentation),
  //we do likewise
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_join__(JNIEnv *env, jobject obj)
{
  thread *current_thread = theJVM->threads.at(theJVM->curr_thread) ;
  thread *thrd = static_cast<thread *>(obj) ;

  if(thrd->blnAlive == true)
    return ;
  else
    {
      //we reset the program counter of the current thread
      //so that when the JVM takes this thread for execution
      //the next time, it again invokes this method;
      //basically this thread will block till the
      //thread whose join() method is invoked
      current_thread->pc -= 3 ;
    }
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_join__J(JNIEnv *env, jobject obj, jlong time_millis)
{
  //since this is a naive implementation, there is no provision to
  //wait a specified time for a thread to die,
  //i.e. all waits are forever ;

  Java_java_lang_Thread_join__(env, obj) ;

}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_join__JI(JNIEnv *env,
								 jobject obj,
								 jlong time_millis,
								 jint tim_nanos)
{
  //since this is a naive implementation, there is no provision to
  //wait a specified time for a thread to die,
  //i.e. all waits are forever ;

  Java_java_lang_Thread_join__(env, obj) ;
}

extern "C" JNIEXPORT jobject JNICALL Java_java_lang_Thread_currentThread(JNIEnv *env, jclass cls)
{
  return static_cast<jobject>(theJVM->threads.at(theJVM->curr_thread)) ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_start(JNIEnv *env, jobject obj)
{
  thread *thrd = static_cast<thread *>(obj) ;

  if(thrd->blnStarted == true)
    {
      theJVM->throw_exception("java/lang/IllegalThreadStateException") ;
      return ;
    }

  Class *c = (static_cast<object *>(obj))->type ;

  method *m = theJVM->resolve_class_method(c, "java/lang/Thread", "run", "()V") ;

  frame *fm = new frame() ;
  fm->rcp = &(c->rcp) ;
  fm->curr_method = m ;

  fm->local_variables.reserve(m->max_locals + 1) ;

  LocalVariable *lv = theJVM->createLocalVariable() ;
  lv->value_type = value::REFERENCE ;
  lv->ref_value = c->class_object ; 
  fm->local_variables.push_back(lv) ;

  for(int it=0; it<m->max_locals; it++)
    fm->local_variables.push_back(NULL) ;

  thrd->java_stack.push(fm) ;
  thrd->pc = 0 ;

  return ;

}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_suspend(JNIEnv *env, jobject obj)
{
  //TO DO: security stuff

  (static_cast<thread *>(obj))->blnSuspended = true ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_resume(JNIEnv *env, jobject obj)
{
  //TO DO: security stuff

  (static_cast<thread *>(obj))->blnSuspended = false ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_yield(JNIEnv *env, jclass cls)
{
  //not implemented
  return ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_stop_1internal__(JNIEnv *env, jobject obj)
{

  //TO DO: security stuff

  (static_cast<thread *>(obj))->blnAlive = false ;

  theJVM->throw_exception("java/lang/ThreadDeath") ;

  return ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_stop_1internal__Ljava_lang_Throwable_2(JNIEnv *env,
											       jobject obj,
											       jthrowable excp)
{
  //TO DO: security stuff

  if(((object *)(excp))->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  (static_cast<thread *>(obj))->blnAlive = false ;

  env->Throw(excp) ;

  return ;
}

extern "C" JNIEXPORT void JNICALL Java_java_lang_Thread_interrupt(JNIEnv *env, jobject obj)
{
  //TO DO:
  //1. security stuff
  //2. code to send InterruptedException and ClosedByInterruptException
  //3. Selector related stuff

  (static_cast<thread *>(obj))->blnInterrupted = true ;

  return ;

}

extern "C" JNIEXPORT jobject JNICALL Java_java_lang_Thread_CreateThread(JNIEnv *env, jclass cls)
{
  thread *new_thread = new thread() ;

  //TO DO: what should the thread_id be set to?
  //also, find out which boolean attributes (in addition to
  //blnAlive need to be set here [or in thread's constructor in jvm.cpp])

  new_thread->blnReady = true ;
  new_thread->nof_inst_in_curr_pass = 0 ;

  new_thread->blnAlive = true ;

  theJVM->threads.push_back(new_thread) ;

  return static_cast<jobject>(new_thread) ;

}
