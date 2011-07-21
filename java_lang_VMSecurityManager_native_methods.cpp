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

#include "java_lang_VMSecurityManager.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

using namespace std ;

JNIEXPORT jobjectArray JNICALL Java_java_lang_VMSecurityManager_getClassContext(JNIEnv *env,
										jclass cls)
{
        
#if DEBUG_LEVEL >= 2        
	cout << "In VMSecurityManager.getClassContext()" << endl << flush ;
#endif
	
	jobject some_temp_object = new _jobject() ;

	int size = theJVM->threads.at(theJVM->curr_thread)->java_stack.size() ;

	typedef map<key, Class *>::const_iterator CI ;
			
	CI ci = theJVM->classes.find(key(NULL,"java/lang/Class")) ;	      

	assert(ci != theJVM->classes.end());
	
	Class *jlc = theJVM->classes[ci->first] ;	

	//create an array object
        //code changed 24/9/2005
	//jarray classes_array = env->NewObjectArray(size,
	//				           static_cast<jclass>(jlc),
	//				           some_temp_object) ;
	jarray classes_array = env->NewObjectArray(size,
					           (jclass)theJVM->getClassObject(jlc),
					           some_temp_object) ;
        //end of code changed 24/9/2005


	int counter = size ;

	//used to store the contents of java_stack
	//IMPORTANT: verify that when this goes out of scope,
	//the constituent frame objects don't get deleted
	stack<frame *> temp_stack ;

	//definition for convenience
	stack<frame *>& java_stack = theJVM->threads.at(theJVM->curr_thread)->java_stack ;

	while(!java_stack.empty())
	{
		frame *fm = java_stack.top() ;
		java_stack.pop() ;
		temp_stack.push(fm) ;
		
		env->SetObjectArrayElement(static_cast<jobjectArray>(classes_array),
					   counter-1,
					   theJVM->getClassObject(fm->curr_method->type)) ;
		counter--;
	}

	//repopulate java_stack from temp_stack
	while(!temp_stack.empty())
	{
		frame *fm = temp_stack.top() ;
		temp_stack.pop() ;
		java_stack.push(fm) ;
	}

  	delete some_temp_object ;	
	
	return static_cast<jobjectArray>(classes_array) ;
}

JNIEXPORT jobject JNICALL Java_java_lang_VMSecurityManager_currentClassLoader(JNIEnv *env, 
									      jclass cls)
{
        
#if DEBUG_LEVEL >= 2        
	cout << "In VMSecurityManager.currentClassLoader()" << endl << flush ;
#endif
        
  //return a null object for the time being
  object *null_obj = theJVM->createObject() ;
  null_obj->blnNull = true ;

  return (jobject)null_obj ;

}
