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

#ifndef _UTIL_
#define _UTIL_

#ifndef WIN32
typedef long long __int64 ;
#endif

#include <string>
#include <vector>
#include <stack>

//code added 17/10/2004
#include "jvm.h"
//end of code added 17/10/2004

using namespace std ;

//this is a dummy exception class
class FormatException
{
} ;


class util  
{
public:
	static int convert_from_hex(char *) ;
	static string convert_to_binary(int) ;
	static string convert_to_binary(long) ;
	static __int64 convert_to_long(string) ;
	static double convert_to_double(string) ;
	static bool is_valid_method_descriptor(string) ;
	static bool is_valid_descriptor(string) ;
	static bool is_valid_class_name(string) ;
	static bool is_valid_identifier(string) ;
	static vector<string>* tokenise(string, char) ;
	static bool is_primitive(string) ;
	static vector<string>* parse_params(string) ;
	static void build_native_method_name1(string, string, char *) ;
	static void build_native_method_name2(string, string, string, char *) ;
	static void replace_substring(string&,string, string);
	static void reverse_stack(stack<string> *s, stack<string> *t);
	static void debugprint(FILE *fp, char *s ...) ;

	//code added 17/10/2004
	//this method takes a given class name and searches the 'classes' map
	//of the jvm object for the corresponding class. the third parameter
	//is the defining class loader
	static Class * getClassFromName(jvm *theJVM, string class_name, Class *loader) ;

	//this method formats a descriptor into a form suitable for display
	//purposes, as in the toString methods in java.lang.Method, for example)
	static string formatDescriptor(string descriptor) ;

	//end of code added 17/10/2004

//code added 7/11/2004
#if DEBUG_LEVEL >= 3	
	static string getInstruction(int) ;
#endif
//end of code added 7/11/2004

//code added 8/10/2005
        //this method to replace the other replace_substring() method
        //(less intrusive, does not modify the passed string)        
	static string replaceSubstring(const string&,string, string);
//end of code added 8/10/2005

private:
	static __int64 power(int,int) ;

//code added 7/11/2004
#if DEBUG_LEVEL >= 3
	//can't use a plain array because
	//there's a break after 202 (jumps to 254/255)
	static map<int, string> instructions ;
	static bool blnInstructionsMapInitialised ;
	
	static void initialiseInstructionsMap() ;
#endif
//end of code added 7/11/2004	
	
};

#endif
