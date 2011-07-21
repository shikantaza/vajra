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

#include <assert.h>

#include "method.h"

const int method::REFERENCE = 1 ;
const int method::INTEGER = 2 ;
const int method::FLOAT = 3 ;
const int method::DOUBLE = 4 ;
const int method::LONG = 5 ;
const int method::BYTE = 6 ;

//code added 13/11/2004
#include <iostream>
#include "logger.h"
//end of code added 13/11/2004

method::method():blnBound(false)
{
  //code added 19/2/2005
  blnPublic = false ;
  blnProtected = false ;
  blnPrivate = false ;
  //end of code added 19/2/2005

  //code added 13/8/2005
  method_object = NULL ;
  //end of code added 13/8/2005
        
  //code added 23/8/2005
  blnFinal = false ;
  //end of code added 23/8/2005

  //code added 15/9/2005
  blnStatic = false ;
  blnSynchronized = false ;
  blnNative = false ;
  blnAbstract = false ;
  blnStrict = false ;
  blnBound = false ;
  //end of code added 15/9/2005

}

method::~method()
{

	delete parameters ;

	for(int i=0; i < line_numbers.size(); i++)
		delete line_numbers.at(i) ;
}

int method::get_line_number(int index)
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("method::get_line_number()") ;
        FunctionCallLogger fcl("method::get_line_number()") ;
#endif
//end of code added 13/11/2004

	int retval ;

	bool found = false ;

	assert(index >= 0 && index < code.byte_code.size()) ;

	for(int i=0; i < line_numbers.size() && found == false; i++)
	{
		
		if(line_numbers.at(i)->start_pc < index)
			continue ;
		else
		{
			//added i > 0 condition 13/11/2004
			if(line_numbers.at(i)->start_pc >= index && i > 0)
				retval = line_numbers.at(i-1)->line_number ;
			else
				retval = line_numbers.at(i)->line_number ;
			found = true ;			
		}
	}

	if(found == false)
		retval = line_numbers.at(0)->line_number ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("method::get_line_number()") ;
#endif
//end of code added 13/11/2004

	return retval ;
}
