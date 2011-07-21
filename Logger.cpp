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

#include "logger.h"

FILE *Logger::_log ;

#include <iostream>

//code added 30/4/2005
#include <sys/types.h>
#include <unistd.h>
//end of code added 30/4/2005

//code added 6/11/2004
//code changed 30/4/2005
//extern bool blnLoggerInitialised ;
//end of code added 30/4/2005

struct timeval Logger::_tp0 ;
struct timeval Logger::_tp ;

//code changed 7/8/2005
//extern int indents ;
int Logger::indents = 0 ;
//end of code changed 7/8/2005

//end of code added 6/11/2004
	
void Logger::Log(string entry)
{

#if DEBUG_LEVEL >= 3

	//just for testing
	//for(int i=0; i<2*indents; i++)
	//	cout << " " ;
	//
	//cout << entry << endl << flush ;
	//return ;
	//end just for testing
	

	//extension of the log file is '.c' in order
	//to take advantage of editors which have
	//folding markers. this helps us to 
	//focus on the areas of the log file we want to
	//more easily

	//code changed 30/4/2005
	//if(blnLoggerInitialised == false)
	// {
	//    gettimeofday(&_tp0, NULL) ;
	//    Logger::_log = fopen("vajra_1.c", "w") ;		
	//    blnLoggerInitialised = true ;
	//  }
	//else
	//  Logger::_log = fopen("vajra_1.c", "a") ;

	pid_t pid = getpid() ;
	char logfileName[20] ;
	memset(logfileName, '\0', 20) ;
	
	sprintf(logfileName, "vajra_log_%d.c", pid) ;
	
	Logger::_log = fopen(logfileName,"a") ;
	//end of code changed 30/4/2005
	

	//code added 6/11/2004
	for(int i=0; i<2*indents; i++)
		fprintf(Logger::_log," ") ;

	//uncomment this for enabling timing		
	//gettimeofday(&Logger::_tp, NULL) ;

	//fprintf(Logger::_log, "[%lf]:",(_tp.tv_sec + _tp.tv_usec/1e6) - 
	//				(_tp0.tv_sec + _tp0.tv_usec/1e6)) ;
	
	//end of code added 6/11/2004

	assert(Logger::_log != NULL) ;
	
	fprintf(Logger::_log,"%s",entry.data()) ;
	fprintf(Logger::_log,"\n") ;
		
	fclose(Logger::_log) ;

#endif
	
}

//code added 13/11/2004
void Logger::LogFunctionEntry(string function_name)
{
#if DEBUG_LEVEL >= 3	
	indents++ ;
	Logger::Log("Entering function " + function_name + " {") ; //curly braces added for the 
								   //benefit of editors with
								   //folding markers
#endif	
}
void Logger::LogFunctionExit(string function_name)
{
#if DEBUG_LEVEL >= 3	
	Logger::Log("Exiting function " + function_name) ;	
	Logger::Log("}") ; //curly braces added for the 
			   //benefit of editors with
			   //folding markers
	indents-- ;
#endif	
}
//end of coded added 13/11/2004

//code added 7/8/2005
#if DEBUG_LEVEL >= 3
void Logger::setIndents(int n)
{
	
	indents = n ;
}
#endif
//end of code added 7/8/2005
