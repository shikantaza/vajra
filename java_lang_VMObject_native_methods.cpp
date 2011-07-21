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

#include "java_lang_VMObject.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

using namespace std ;

JNIEXPORT jobject JNICALL Java_java_lang_VMObject_clone(JNIEnv *env, jclass cls, jobject obj1)
{

#if DEBUG_LEVEL >= 2
	cout << "Entering clone()" << endl << flush ;
#endif

	object *obj = static_cast<object*>(obj1) ;
	
	object *cloned_object = new object(*obj) ;

#if DEBUG_LEVEL >= 2	
	cout << "Exiting clone()" << endl << flush ;
#endif
	
	return (jobject)cloned_object ;
	//cout << "TO DO: VMObject.clone() to be implemented" << endl << flush ;
	
}

JNIEXPORT void JNICALL Java_java_lang_VMObject_notify(JNIEnv *env, jclass cls, jobject obj1)
{
	cout << "TO DO: VMObject.notify() to be implemented" << endl << flush ;
}

JNIEXPORT void JNICALL Java_java_lang_VMObject_notifyAll(JNIEnv *env, jclass cls, jobject obj1)
{
	cout << "TO DO: VMObject.notifyAll() to be implemented" << endl << flush ;
}

JNIEXPORT void JNICALL Java_java_lang_VMObject_wait(JNIEnv *env, jclass cls, jobject obj1, jlong long1, jint int1)
{
	cout << "TO DO: VMObject.wait() to be implemented" << endl << flush ;
}
