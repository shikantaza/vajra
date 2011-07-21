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

#ifndef _LOGGER_
#define _LOGGER_
#include <cstdio>
#include <cassert>
#include <string>

//code added 6/11/2004
#include <sys/time.h>
//end of code added 6/11/2004

using namespace std ;

class Logger
{
	private:
		static FILE *_log ;

		//code added 6/11/2004
		//code changed 26/4/2005
		//static bool blnInitialised ;
		//end of code changed 26/4/2005
				
		static struct timeval _tp0, _tp ;
		//end of code added 6/11/2004
		
		//code added 7/8/2005
		static int indents ;
		//end of code added 7/8/2005
	public:
		void static Log(string entry) ;
		
		//code added 13/11/2004
		static void LogFunctionEntry(string function_name) ;
		static void LogFunctionExit(string function_name) ;
		//end of code added 13/11/2004
	
//code added 7/8/2005
#if DEBUG_LEVEL >= 3	
		static void setIndents(int n) ;
#endif	
//end of code added 7/8/2005
	
} ;

//code added 7/8/2005
//this class makes logging function calls easier;
//just create a local variable of FunctionCallLogger
//at the start of the function; the constructors and
//destructors will take care of the rest [ALL possible
//exit points] (from Scott Meyers' book)
#if DEBUG_LEVEL >= 3
class FunctionCallLogger
{
	private:
		string _functionName ;
	public:
		FunctionCallLogger(string functionName)
		{
			_functionName = functionName ;
			Logger::LogFunctionEntry(_functionName) ;		
		}
		
		~FunctionCallLogger()
		{
			Logger::LogFunctionExit(_functionName) ;		
		}
} ;
#endif
//end of code added 7/8/2005

#endif
