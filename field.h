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

#ifndef _FIELD_
#define _FIELD_

#include <string>

class Class ;

using namespace std ;


//code changed on 29/6/2004
//class field
class field : public _jobject
//end of code changed on 29/6/2004
{
public:

	//identifies the class in which
	//the field is declared
	Class *parent_class ;

	int access_flags ;

	bool blnPublic ;
	bool blnPrivate ;
	bool blnProtected ;
	bool blnStatic ;
	bool blnFinal ;
	bool blnVolatile ;
	bool blnTransient ;

	//identifies if the field is a primitive
	bool blnPrimitive ;

	//if it is a primitive field,
	//identifies the field type;
	//this takes values from the 
	//'primitive_value' class
	int value_type ;

	string name ;

	//code changed 27/2/2005
	//descriptor is made private to take care of
	//uneplained changes to it's value 
	//string descriptor ;
	//end of code changed 27/2/2005

/*
	//the attributes below are for static values
	char byte_char_value ;
	__int64 short_int_long_value ;
	double float_double_value ;
	int object_index ;			//index into objects array
*/

	//code added 6/2/2005
	field()
	{
		blnPublic  = false ;
		blnPrivate = false ;
		blnProtected = false ;

		//code added 12/2/2005
		blnStatic = false ;
		blnFinal = false ;
		blnVolatile = false ;
		blnTransient = false ;
		//end of code added 12/2/2005

		//code added 15/9/2005
		blnPrimitive = false ;
		//end of code added 15/9/2005
				
	}
	//end of code added 6/2/2005

	//code added 27/2/2005
	string get_descriptor()
	{
	  return descriptor ;
	}

	void set_descriptor(string desc)
	{
	  descriptor = desc ;
	}
	//end of code added 27/2/2005

//code added 27/2/2005
 private:
	string descriptor ;
//end of code added 27/2/2005


} ;
#endif
