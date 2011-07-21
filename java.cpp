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

#include <iostream>

//code added 6/2/2006
#include <stdexcept>
//end of code added 6/2/2006

#include "jvm.h"

//code added 6/11/2004
#include "logger.h"
//end of code added 6/11/2004

//we have a global jvm object
jvm *theJVM ;

//code added 24/5/2004, changed 6/8/2005
//string classpath ;
//end of code added 24/5/2004, changed 6/8/2005

//code added 6/11/2004, changed 7/8/2005
//to indent the debug output
//int indents = 0 ;
//end of code added 6/11/2004, changed 7/8/2005

//code added 14/11/2004
bool blnOutfileInitialised = false ;
//end of code added 14/11/2004

//code added 26/4/2005
//code changed 30/4/2005
//bool blnLoggerInitialised = false ;
//end of code changed 30/4/2005
//end of code added 26/4/2005

int main(int argc, char **argv)
{

//code added 7/8/2005
#if DEBUG_LEVEL >= 3
	Logger::setIndents(0) ;
#endif	
//end of code added 7/8/2005
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
    Logger::Log("Entering main()") ;
#endif
//end of code added 6/11/2004

    //code added 6/8/2005
    string classpath ;
    //end of code added 6/8/2005
	
    if(argc == 1)
    {
        cout << "Usage: vajra [-classpath <classpath>] <class name>" << endl ;
        exit(1) ;
    }
    
    //code addded 24/5/2004
    bool classpath_present = false ;

    //if present, '-classpath' should be the second commandline parameter
    for(int i=0; i<argc; i++)
      {
	if((strcmp(argv[i],"-classpath")==0))
	  {
	    classpath_present = true ;
	    if(i != 1)
	      {
		cout << "Usage: vajra [-classpath <classpath>] <class name>" << endl ;
		exit(1) ;
	      }
	  }
      }

    //if classpath is present, there should be a minimum
    //of four parameters
    if(classpath_present == true and argc <= 3)
      {
	cout << "Usage: vajra [-classpath <classpath>] <class name>" << endl ;
	exit(1) ;
      }

    if(classpath_present == true)
      classpath = argv[2] ;

    //code changed 4/2/2006
    //string env_classpath = getenv("CLASSPATH") ;
    
    //code changed 6/2/2006
    //string env_classpath = getenv("VAJRA_CLASSPATH") ;
    string env_classpath ;
    try
    {
            env_classpath = getenv("VAJRA_CLASSPATH") ;
    }
    catch(logic_error)
    {
            //environment variable not set
            env_classpath = "" ;
    }
    //end of code changed 6/2/2006
    
    //end of code changed 4/2/2006
    
    if(classpath_present == true)
      classpath += ":" + env_classpath ;
    else
      classpath = env_classpath ;

    //end of code added 24/5/2004

    vector<string> args ;

    //code changed 24/5/2004
    //for(int i=2; i<argc; i++)
      //args.push_back(string(argv[i])) ; 

    if(classpath_present == true)
      {
	for(int i=4; i<argc; i++)
	  args.push_back(string(argv[i])) ; 
      }
    else
      {
	for(int i=2; i<argc; i++)
	  args.push_back(string(argv[i])) ; 
      }

    //end of code changed 24/5/2004

    //jvm JVM ;

    //code changed 6/8/2005
    //theJVM = new jvm() ;
    theJVM = new jvm(classpath) ;
    //end of code changed 6/8/2005

    //code changed 24/5/2004
    //theJVM->init(argv[1], &args) ;
    if(classpath_present == true)
      theJVM->init(argv[3], args) ;
    else
      theJVM->init(argv[1], args) ;
    //end of code changed 24/5/2004

    delete theJVM ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
    Logger::Log("Exiting main()") ;
#endif
//end of code added 6/11/2004

    return 0 ;

}
