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

#include "java_lang_VMClassLoader.h"

#include "jvm.h"
#include "util.h"

#include <cassert>

using namespace std ;

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

JNIEXPORT jclass JNICALL Java_java_lang_VMClassLoader_getPrimitiveClass(JNIEnv *env, 
                                                                        jclass cls, 
                                                                        jstring str)
{
        string primitive_name(env->GetStringUTFChars(str, 0)) ;  

        string class_name ;
        
        if(primitive_name == "boolean")
                class_name = "java/lang/Boolean" ;
        else if(primitive_name == "byte")
                class_name = "java/lang/Byte" ;
        else if(primitive_name == "char")
                class_name =  "java/lang/Character" ;
        else if(primitive_name == "double")
                class_name =  "java/lang/Double" ;
        else if(primitive_name == "float")
                class_name =  "java/lang/Float" ;
        else if(primitive_name == "int")
                class_name =  "java/lang/Integer" ;
        else if(primitive_name == "long")
                class_name =  "java/lang/Long" ;
        else if(primitive_name == "short")
                class_name =  "java/lang/Short" ; 

        //TO DO: how to handle "void"?
        
        typedef map<key, Class *>::const_iterator CI ;        

        CI ci = theJVM->classes.find(key(NULL,class_name)) ;	      
		
	if(ci == theJVM->classes.end())
		theJVM->load_class(class_name) ;
	
	ci = theJVM->classes.find(key(NULL,class_name)) ;
		
	assert(ci != theJVM->classes.end()) ;
        
        Class *c = theJVM->classes[ci->first] ;

        return (jclass)theJVM->getClassObject(c) ;        
}
