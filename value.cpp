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

//#include "value.h"

//code added 22/5/2005
#include <iostream>
using namespace std ;
//end of code added 22/5/2005

const int value::BOOLEAN = 1;
const int value::_BYTE = 2 ;
const int value::SHORT = 3 ;
const int value::INT = 4 ;
const int value::LONG = 5 ;
const int value::CHAR = 6 ;
const int value::FLOAT = 7 ;
const int value::DOUBLE = 8 ;
const int value::REFERENCE = 9 ;
const int value::RETURN_ADDRESS = 10 ;

//code added 22/5/2005
#if DEBUG_LEVEL >= 3
void value::display()
{
	//cout << "--------------dumping value object " << this << "---------" << endl << flush ;
	cout << "value type = " ;
	
	switch(value_type)
	{
		case BOOLEAN:
			cout << "boolean" << endl << flush ;
			break ;			
		case _BYTE:
			cout << "byte" << endl << flush ;
			break ;			
		case SHORT:
			cout << "short" << endl << flush ;
			break ;			
		case INT:
			cout << "int" << endl << flush ;
			break ;			
		case CHAR:
			cout << "char" << endl << flush ;
			break ;			
		case LONG:
			cout << "long" << endl << flush ;
			break ;			
		case FLOAT:
			cout << "float" << endl << flush ;
			break ;			
		case DOUBLE:
			cout << "double" << endl << flush ;
			break ;			
		case REFERENCE:
			cout << "reference" << endl << flush ;
			break ;			
		case RETURN_ADDRESS:
			cout << "return address" << endl << flush ;
			break ;			
	}
	
	cout << "value = " ;
	switch(value_type)
	{
		case BOOLEAN:
			cout << ((bool_value == true) ? "true" : "false") << endl << flush ;
			break ;			
		case _BYTE:
		case SHORT:
		case INT:
		case CHAR:
			cout << i_value << endl << flush ;
			break ;
		case LONG:
			cout << long_value << endl << flush ;
			break ;
		case FLOAT:
			cout << float_value << endl << flush ;
			break ;			
		case DOUBLE:
			cout << double_value << endl << flush ;
			break ;			
		case REFERENCE:
			cout << ref_value << endl << flush ;
			ref_value->display() ;
			break ;			
		case RETURN_ADDRESS:
			cout << ret_add_value << endl << flush ;
			break ;			
	}
	//cout << "---------------end of dump---------------------- " << endl << flush ;
}
#endif
//end of code added 22/5/2005
