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

#include <sys/time.h>

#include "java_lang_VMSystem.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

using namespace std ;

JNIEXPORT void JNICALL Java_java_lang_VMSystem_arraycopy(JNIEnv *env, 
							 jclass cls, 
							 jobject src,
							 jint srcPos,
							 jobject dest,
							 jint destPos,
							 jint length)
{

  //TO DO: add exception handling

#if DEBUG_LEVEL >= 2	
  cout << "Entering arrayCopy()" << endl << flush ;
#endif	

  object *srcObj = static_cast<object *>(src) ;
  object *destObj = static_cast<object *>(dest) ;

#if DEBUG_LEVEL >= 3
  assert(srcObj->blnArrayObject == true) ;
  assert(destObj->blnArrayObject == true) ;
#endif

  if(srcObj->blnPrimitiveArrayObject == true)
    {

#if DEBUG_LEVEL >= 3
      assert(destObj->blnPrimitiveArrayObject == true) ;
#endif
      for(int i=srcPos, j=destPos; i<srcPos + length; i++, j++)
	destObj->primitives.at(j) = srcObj->primitives.at(i) ;
    }
  else
    {
      for(int i=srcPos, j=destPos; i<srcPos + length; i++, j++)
	destObj->items.at(j) = srcObj->items.at(i) ;
    }

#if DEBUG_LEVEL >= 2    
  cout << "Exiting arrayCopy()" << endl << flush ;
#endif    

}

JNIEXPORT jint JNICALL Java_java_lang_VMSystem_identityHashCode(JNIEnv *env,
								jclass cls,
								jobject obj)
{
  //cout << "TO DO: VMSystem.identityHashCode() to be implemented" << endl << flush ;
  return (jint)obj;
}

JNIEXPORT jboolean JNICALL Java_java_lang_VMSystem_isWordsBigEndian(JNIEnv *env,
								    jclass cls)
{
  //cout << "TO DO: VMSystem.isWordBigEndian() to be implemented" << endl << flush ;
}

JNIEXPORT void JNICALL Java_java_lang_VMSystem_setIn(JNIEnv *env,
						     jclass cls,
						     jobject obj)
{
  cout << "TO DO: VMSystem.setIn() to be implemented" << endl << flush ;
}


JNIEXPORT void JNICALL Java_java_lang_VMSystem_setOut(JNIEnv *env,
						      jclass cls,
						      jobject obj)
{
  cout << "TO DO: VMSystem.setOut() to be implemented" << endl << flush ;
}

JNIEXPORT void JNICALL Java_java_lang_VMSystem_setErr(JNIEnv *env,
						      jclass cls,
						      jobject obj)
{
  cout << "TO DO: VMSystem.setErr() to be implemented" << endl << flush ;
}

JNIEXPORT jlong JNICALL Java_java_lang_VMSystem_currentTimeMillis(JNIEnv *env,
								  jclass cls)
{
  struct timeval t ;

  gettimeofday(&t, NULL) ;

  return (jlong)(t.tv_sec * 1000 + t.tv_usec / 1000) ; //tv_sec - unit is second; tv_usec - microsecond
}
