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

#include "java_lang_VMThread.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

using namespace std ;

//TO DO: implement other methods declared in java_lang_VMThread.h

JNIEXPORT jobject JNICALL Java_java_lang_VMThread_currentThread(JNIEnv *env, jclass cls)
{
  return static_cast<jobject>(theJVM->threads.at(theJVM->curr_thread)) ;
}
