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

#include "util.h"

#include <math.h>
// 25/12/2003: commented out because gcc 3.3.2 complains that
// this file is not available; also doesn't seem to be required
//#include <bitset.h>
#include <vector>
#include <cctype>

#include <cassert>

#include <cstdarg>

//code added 6/11/2004
extern int indents ;
//end of code added  6/11/2004

//code added 7/11/2004
#if DEBUG_LEVEL >= 3
map<int, string> util::instructions ;
bool util::blnInstructionsMapInitialised ;
#endif
//end of code added 7/11/2004

//converts a hex number (passed as a string) to
//its decimal equivalent
int util::convert_from_hex(char *s)
{

	int retval = 0 ;

	int v ;

	unsigned int n = strlen(s) ;

	for(int i=0; i<n; i++)
	{
		if(s[i] == '0')v = 0 ;
		if(s[i] == '1')v = 1 ;
		if(s[i] == '2')v = 2 ;
		if(s[i] == '3')v = 3 ;
		if(s[i] == '4')v = 4 ;
		if(s[i] == '5')v = 5 ;
		if(s[i] == '6')v = 6 ;
		if(s[i] == '7')v = 7 ;
		if(s[i] == '8')v = 8 ;
		if(s[i] == '9')v = 9 ;
		if(s[i] == 'A')v = 10 ;
		if(s[i] == 'B')v = 11 ;
		if(s[i] == 'C')v = 12 ;
		if(s[i] == 'D')v = 13 ;
		if(s[i] == 'E')v = 14 ;
		if(s[i] == 'F')v = 15 ;

		//code changed 19/2/2005
		//retval += v * pow(16.0,(double)(n-i-1)) ;
		retval += (int)(v * pow(16.0,(double)(n-i-1))) ;
		//end of code changed 19/2/2005
	}

	return(retval) ;
}


string util::convert_to_binary(int val)
{

	if(val == 0)
		return("00000000") ;

	if(val > 255)
		return("11111111") ;

	string retval = "" ;
	int divisor = 128 ;

	int quo, rem ;

	rem = val ;

	while(rem > 0)
	{
		quo = val / divisor ;
		rem = val % divisor ;

		if(quo > 0) retval += "1" ;
		else		retval += "0" ;

		val = rem ;
		divisor /= 2 ;
	}

	//pad the string with zeros so that 
	//its length becomes eight characters
	int l = retval.length() ;

	for(int i=0; i<8-l; i++)
		retval += "0" ;
	
	return retval ;
}

string util::convert_to_binary(long orig_val)
{

	if(orig_val < 0)
		return("") ;

	long val = orig_val ;

	long divisor = 1 ;

	while(divisor <= val)
		divisor *= 2 ;
	
	long  quo, rem ;

	string retval = "" ;

	while(divisor > 0)
	{
		quo = val / divisor ;
		rem = val % divisor ;

		if(quo > 0)
			retval += "1" ;
		else
		{
			if(val != orig_val)
				retval += "0" ;
		}
		val = rem ;

		divisor /= 2 ;

	}

	return retval ;
}

/*
IEEE 754 standard for double precision numbers

bit 1:		sign bit (s); +1 if '0'; -1 if '1'
bits 2-12:	biased exponent (e)
bits 13-64:	fraction (f)

if 0 < e < 2047, value = (-1)**s x 2**(e-1023) x (1.f)
if e = 0 and f != 0, value = (-1)**s x 2**(-1022) x (0.f)
if e = 0 and f = 0, value = 0
for other values of e and f, value evaluates 
to +/- infinity, NaN; we are not bothered with them now

e is calculated directly by converting the 11-bit number into 
its decimal equivalent

f = .b b  ......b
      1 2        52
e.g. if the bits corresponding to f are 01011...,
f is calulated as (0 x 2 ** -1) + (1 x 2 ** -2) + (0 * 2 ** -3) + ...
(Note: '**' represents exponentiation)
*/
double util::convert_to_double(string s1)
{

	int i ;

	double retval = 0 ;

	if(s1.length() != 64)
		return 0 ;

	int s, e = 0 ;

	double m = 0 ;

	if(s1.at(0) == '0')
		s = 1 ;
	else
		s = -1 ;

	string str1 = "" ; 

	for(i=1; i<12; i++)
		str1 += s1.at(i) ;

	for(i=10; i>=0; i--)
	{
		if(str1.at(i) == '1')
		  //code changed 19/2/2005
		  //e += pow(2.0,10-i) ;
		  e += (int)pow(2.0,10-i) ;
		  //end of code changed 19/2/2005
	}

	string str2 = "" ;

	
	for(i=12; i<64; i++)
		str2 += s1.at(i) ;

	for(i=51; i>=0; i--)
	{
		if(str2.at(i) == '1')
			m += pow(2.0,-1*i - 1) ;
	}


	if(e > 0 && e < 2047)
		retval = s * pow(2.0,e-1023) * (1 + m) ;

	if(e == 0 && m != 0)
		retval = s * pow(2.0,-1022) * m ;

	if(e == 0 && m == 0)
		retval = 0 ;

	return retval ;
}

/*
bit 1: sign bit
bits 2-64: value
if sign bit is 0 -> long value = value
else		 -> long value = value - pow(2,63)
*/
__int64 util::convert_to_long(string s1)
{

	int i ;

	__int64 retval = 0 ;

	for(i=1; i<64; i++)
	{
		if(s1.at(i) == '1')
			retval += util::power(2,63-i) ;
	}

	if(s1.at(0) == '0')
		/*null */ ;
	else
		retval = retval - util::power(2,63) ;
	
 	return retval ;
}

__int64 util::power(int x, int y)
{
	__int64 retval = 1 ;

	for(int i=0; i < y; i++)
		retval *= x ;

	return retval ;
}

bool util::is_valid_descriptor(string s)
{

	if(s.size() == 0)
		return false ;

	if(s == "B" || s == "C" || s == "D" || s == "F" ||
		s == "I" || s == "J" || s == "S" || s == "Z")
		return true ;
	else if(s[0] == 'L')
	{

		if(is_valid_class_name(s.substr(1,s.size()-2)) == false)
			return false ;

		if(s.find(";") == s.size()-1)
			return true ;
	}
	else if(s[0] == '[')
		return is_valid_descriptor(s.substr(1,s.length()-1)) ;
	else
		return is_valid_identifier(s.substr(1,s.length()-1)) ;

    return false ;
}

bool util::is_valid_method_descriptor(string s)
{

	//first character should be ')'
	if(s[0] != '(')
		return false ;

	int rp = s.find(")") ;

	//a right paranthesis must occur in the descriptor
	if(rp == string::npos)
		return false ;

	//1. extract the substring between the paranthesis
	//2. pass this string to the split function
	//   which extracts the individual descriptors
	//3. verify that each of these individual descriptors
	//   are valid
	string params = s.substr(1,rp - 1) ;

/*
	vector<string> v ;

	int ix = 0 ;

	while(ix < params.size())
	{

		string s = params.substr(ix,params.size()-ix) ;

		if(s[0] == 'B' || s[0] == 'C' || s[0] == 'D' || 
			s[0] == 'F' || s[0] == 'I' || s[0] == 'J' || 
			s[0] == 'S' || s[0] == 'Z')
		{
			v.insert(v.end(), s.substr(0,1)) ;
			ix++ ;
		}
		else if(s[0] == 'L')
		{
			int sc = s.find(";") ;
			if(sc == string::npos)
				return false ;
			v.insert(v.end(), s.substr(0,sc+1)) ;
			ix += sc + 1 ;
		}
		else if(s[0] == '[')
		{
			int i=1 ;
			string s1 = "[" ;
			while(s[i] == '[')
			{
				s1 += '[' ;
				i++ ;
				ix++ ;
			}

			if(s[i] == 'B' || s[i] == 'C' || s[i] == 'D' || s[i] == 'F' ||
				s[i] == 'I' || s[i] == 'J' || s[i] == 'S' || s[i] == 'Z')
			{
				s1 += s[i] ;
				v.insert(v.end(), s1) ;
				ix += 2 ;
			}
			else if(s[i] == 'L')
			{
				s1 += s[i] ;
				int sc = s.find(";") ;
				if(sc == string::npos) return false ;

				s1 += s.substr(i+1,sc-i) ;

				v.insert(v.end(), s1) ;
				ix += sc + 2 ;
			}
			else
				return false ;
		}
		else
			return false ;


	}
*/
	vector<string> *v = parse_params(params) ;
	if(v == NULL)
		return false ;
 
	//marks end of code to be replaced
	//with call to parse_params

	for(int i=0; i<v->size(); i++)
	{
		if(is_valid_descriptor(v->at(i)) == false)
		{
			delete v ;
			return false ;
		}
	}

	delete v ;

	//extract the return value descriptor
	//and verify that this is also valid
	string return_desc = s.substr(rp+1, s.size() - rp - 1) ;

	if(return_desc == "V")
		return true ;
	else if(is_valid_descriptor(return_desc) == false)
		return false ;

	return true ;
}

bool util::is_valid_class_name(string s)
{

	//for the time being, these are the checks:
	//1. first character should be a letter
	//2. can contain any letter/digit, forward slash, underscore, dollar sign
	//3. two forward slashes shouldn't occur together
	//4. last character should not be a forward slash

	if(s[0] == '[')
		return(is_valid_descriptor(s)) ;

	if(!isalpha(s[0]))
		return false ;

	for(int i=1; i < s.size(); i++)
	{
		if(!(isalpha(s[i]) || isdigit(s[i]) || s[i] == '/'  
		     || s[i] == '_' || s[i] == '$'))
			return false ;
	}

	if(s.find("//") != string::npos)
		return false ;

	if(s[s.size()-1] == '/') 
		return false ;

	return true ;
}

bool util::is_valid_identifier(string s)
{

	if(s.length() == 0)
		return true ;

	//first character should be a letter
	if(!(isalpha(s[0])))
		return false ;

	//other characters must be letters or digits
	for(int i=1; i<s.size(); i++)
	{
		if(!(isalpha(s[i]) || isdigit(s[i]) || s[i] == '_' || s[i] == '$' || s[i] == '/'))
			return false ;
	}
	return true ;
}

vector<string>* util::tokenise(string s, char c)
{

	vector<string> *v = new vector<string>; 

	int l = s.length(), i=0, ptr=0 ;

	while(i<l)
	{
		if(s[i] == c)
		{
			v->push_back(s.substr(ptr, i-ptr)) ;
			ptr = i + 1 ;
		}
		i++ ;
	}


	if(ptr < l)
		v->push_back(s.substr(ptr)) ;

	return v ;
}

bool util::is_primitive(string name)
{
	if(name == "B" ||
	   name == "C" ||
	   name == "D" ||
	   name == "F" ||
	   name == "I" ||
	   name == "J" ||
	   name == "S" ||
	   name == "Z")
		return true ;
	else
		return false ;
}

vector<string>* util::parse_params(string params)
{
	vector<string> *v = new vector<string>; 

	int ix = 0 ;

	while(ix < params.size())
	{

		string s = params.substr(ix,params.size()-ix) ;

		if(s[0] == 'B' || s[0] == 'C' || s[0] == 'D' || 
			s[0] == 'F' || s[0] == 'I' || s[0] == 'J' || 
			s[0] == 'S' || s[0] == 'Z')
		{
			v->insert(v->end(), s.substr(0,1)) ;
			ix++ ;
		}
		else if(s[0] == 'L')
		{
			int sc = s.find(";") ;
			if(sc == string::npos)
				return NULL ;
			v->insert(v->end(), s.substr(0,sc+1)) ;
			ix += sc + 1 ;
		}
		else if(s[0] == '[')
		{
			int i=1 ;
			string s1 = "[" ;
			while(s[i] == '[')
			{
				s1 += '[' ;
				i++ ;
				ix++ ;
			}


			if(s[i] == 'B' || s[i] == 'C' || s[i] == 'D' || s[i] == 'F' ||
				s[i] == 'I' || s[i] == 'J' || s[i] == 'S' || s[i] == 'Z')
			{
				s1 += s[i] ;
				v->insert(v->end(), s1) ;
				ix += 2 ;
			}
			else if(s[i] == 'L')
			{
				s1 += s[i] ;
				int sc = s.find(";") ;
				if(sc == string::npos) return NULL ;

				s1 += s.substr(i+1,sc-i) ;

				v->insert(v->end(), s1) ;
				ix += sc + 1 ; //this was 2 earlier
			}
			else
				return NULL ;
		}
		else
			return NULL ;


	}

	return v ;
}

//this method builds the native method name
//for the given method name; it stores the
//result in the third parameter
//this function is to be used to build the
//native method name without the signature
void util::build_native_method_name1(string class_name, string method_name,char *buf)
{

    string retval = "Java_" ;

    assert(buf != NULL) ;

    //this is to prevent the underscore between the
    //class name and method name to be replaced by "_1"
    util::replace_substring(method_name,"_","_1") ;

    retval += class_name + "_" + method_name ;

    //replace "/" with "_", "_" with "_1",
    //";" with "_2" and "[" with "_3"
    //retval.replace(retval.find("_"),retval.length(),"_1") ;
    //retval.replace(retval.find("/"),retval.length(),"_") ;
    //retval.replace(retval.find(";"),retval.length(),"_2") ;
    //retval.replace(retval.find("["),retval.length(),"_3") ;

    //util::replace_substring(retval,"_","_1") ;
    util::replace_substring(retval,"/","_") ;
    util::replace_substring(retval,";","_2") ;
    util::replace_substring(retval,"[","_3") ;

    int l = retval.length() ;

    for(int i=0; i<l; i++)
        buf[i] = retval.at(i) ;

    buf[l] = '\0' ;
}

//this method builds the native method name
//for the given method name; it stores the
//result in the fourth parameter
//this function is to be used to build the
//native method name with the signature
void util::build_native_method_name2(string class_name, string method_name,string param_string, char *buf)
{

    string retval = "Java_" ;

    assert(buf != NULL) ;

    //this is to prevent the underscore between the
    //class name and method name to be replaced by "_1"
    util::replace_substring(method_name,"_","_1") ;

    //we need to strip out the parantheses from
    //the parameter string
    retval += class_name + "_" + method_name + "__" + param_string.substr(1,param_string.length()-2) ;

    //replace "/" with "_", "_" with "_1",
    //";" with "_2" and "[" with "_3"
    //retval.replace(retval.find("_"),retval.length(),"_1") ;
    //retval.replace(retval.find("/"),retval.length(),"_") ;
    //retval.replace(retval.find(";"),retval.length(),"_2") ;
    //retval.replace(retval.find("["),retval.length(),"_3") ;
    //util::replace_substring(retval,"_","_1") ;
    util::replace_substring(retval,"/","_") ;
    util::replace_substring(retval,";","_2") ;
    util::replace_substring(retval,"[","_3") ;


    int l = retval.length() ;

    for(int i=0; i<l; i++)
        buf[i] = retval.at(i) ;

    buf[l] = '\0' ;
}

//this function replaces occurrences of string s2 in
//string s1 with s3
void util::replace_substring(string& s1, string s2, string s3)
{

//    unsigned int i = 0 ;
//
//    while((i = s1.find(s2)) != string::npos)
//        s1.replace(i,s2.length(),s3) ;

    int i = 0 ;
    string *copy = new string() ;
    int s1_len = s1.length() ;
    int s2_len = s2.length() ;
    int s3_len = s3.length() ;

    while(i < s1_len)
    {
        if(s1.substr(i,s2_len) == s2)
        {
            *copy += s3 ;
            //code changed 20/8/2005
            //i += s3_len ;
            i += s2_len ;
            //end of code changed 20/8/2005
            
        }
        else
        {
            *copy += s1[i] ;
            i++ ;
        }
    }

    s1 = *copy ;

    delete copy ;
}


//this function reverses the given stack
//and places the reversed contents in another
//stack
void util::reverse_stack(stack<string> *s, stack<string> *t)
{
  while(!s->empty())
    {
      t->push(s->top()) ;
      s->pop() ;
    }

}

//this method is used to print debug information into a
//specified file. The file should already have been opened
void util::debugprint(FILE *fp, char *s ...)
{

#if DEBUG_LEVEL >= 3

  const int MAX_VAR_ARGS = 10 ;

  void *p[MAX_VAR_ARGS] ;

  assert(fp != NULL) ;

  va_list ap ;

  va_start(ap, s) ;

  for(int i=0;i<MAX_VAR_ARGS; i++)
  {
    p[i] = va_arg(ap, void *) ;
    if(p[i] == 0)break ;
  }

  fprintf(fp, s, p[0], p[1], p[2], p[3], p[4],
	  p[5], p[6], p[7], p[8], p[9]) ;

  //code added 7/11/2004
  fflush(fp) ;
  //end of code added 7/11/2004

#endif	

}

//code added 17/10/2004
Class * util::getClassFromName(jvm *theJVM, string class_name, Class *loader)
{
  typedef map<key, Class *>::const_iterator CI ;

  CI ci = theJVM->classes.find(key(loader, class_name)) ;  

#if DEBUG_LEVEL >= 3
  assert(ci != theJVM->classes.end()) ;
#endif

  return theJVM->classes[ci->first] ;

}

string util::formatDescriptor(string descriptor)
{

  string retval = "" ;

  if(descriptor == "B")
    retval += "byte" ;
  else if(descriptor == "C")
    retval += "char" ;
  else if(descriptor == "D")
    retval += "double" ;
  else if(descriptor == "F")
    retval += "float" ;
  else if(descriptor == "I")
	retval += "int" ;
  else if(descriptor == "J")
    retval += "long" ;
  else if(descriptor == "S")
    retval += "short" ;
  else if(descriptor == "Z")
    retval += "boolean" ;
  else if(descriptor.at(0) == 'L') //i.e. it is a reference type
    {
      
      string ref_type = descriptor.substr(1,descriptor.length()-2) ; //we need to omit the ';' at the end
      
      util::replace_substring(ref_type, "/", ".") ;

      retval += ref_type ;

    }
  else if(descriptor.at(0) == '[') // array
    {
      string array_type ;

      //count the dimensions
      int dims = 1 ;
      
      for(int j=1; j<descriptor.length(); j++)
	{
	  if(descriptor.at(j) == '[')
	    dims++ ;
	  else
	    break ;
	}

      array_type = descriptor.substr(dims,descriptor.length()-1) ;

      if(array_type == "B")
	retval += "byte" ;
      else if(array_type == "C")
	retval += "char" ;
      else if(array_type == "D")
	retval += "double" ;
      else if(array_type == "F")
	retval += "float" ;
      else if(array_type == "I")
	retval += "int" ;
      else if(array_type == "J")
	retval += "long" ;
      else if(array_type == "S")
	retval += "short" ;
      else if(array_type == "Z")
	retval += "boolean" ;
      else if(array_type.at(0) == 'L') //i.e. it is a reference type
	{

	  //again omitting the ';' at the end	      
	  string array_type2 = array_type.substr(1,array_type.length()-2) ;

	  util::replace_substring(array_type2,"/",".") ;

	  retval += array_type2 ;

	}

      for(int j=0; j<dims; j++)
	retval += "[]" ;

    }

  return retval ;
}
//end of code added 17/10/2004

//code added 7/11/2004
#if DEBUG_LEVEL >= 3
void util::initialiseInstructionsMap()
{

	util::instructions[0] = "nop" ;
	instructions[1] = "aconst_null" ;
	instructions[2] = "iconst_m1" ;
	instructions[3] = "iconst_0" ;
	instructions[4] = "iconst_1" ;
	instructions[5] = "iconst_2" ;
	instructions[6] = "iconst_3" ;
	instructions[7] = "iconst_4" ;
	instructions[8] = "iconst_5" ;
	instructions[9] = "lconst_0" ;
	instructions[10] = "lconst_1" ;
	instructions[11] = "fconst_0" ;
	instructions[12] = "fconst_1" ;
	instructions[13] = "fconst_2" ;
	instructions[14] = "dconst_0" ;
	instructions[15] = "dconst_1" ;
	instructions[16] = "bipush" ;
	instructions[17] = "sipush" ;
	instructions[18] = "ldc" ;
	instructions[19] = "ldc_w" ;
	instructions[20] = "ldc2_w" ;
	instructions[21] = "iload" ;
	instructions[22] = "lload" ;
	instructions[23] = "fload" ;
	instructions[24] = "dload" ;
	instructions[25] = "aload" ;
	instructions[26] = "iload_0" ;
	instructions[27] = "iload_1" ;
	instructions[28] = "iload_2" ;
	instructions[29] = "iload_3" ;
	instructions[30] = "lload_0" ;
	instructions[31] = "lload_1" ;
	instructions[32] = "lload_2" ;
	instructions[33] = "lload_3" ;
	instructions[34] = "fload_0" ;
	instructions[35] = "fload_1" ;
	instructions[36] = "fload_2" ;
	instructions[37] = "fload_3" ;
	instructions[38] = "dload_0" ;
	instructions[39] = "dload_1" ;
	instructions[40] = "dload_2" ;
	instructions[41] = "dload_3" ;
	instructions[42] = "aload_0" ;
	instructions[43] = "aload_1" ;
	instructions[44] = "aload_2" ;
	instructions[45] = "aload_3" ;
	instructions[46] = "iaload" ;
	instructions[47] = "laload" ;
	instructions[48] = "faload" ;
	instructions[49] = "daload" ;
	instructions[50] = "aaload" ;
	instructions[51] = "baload" ;
	instructions[52] = "caload" ;
	instructions[53] = "saload" ;
	instructions[54] = "istore" ;
	instructions[55] = "lstore" ;
	instructions[56] = "fstore" ;
	instructions[57] = "dstore" ;
	instructions[58] = "astore" ;
	instructions[59] = "istore_0" ;
	instructions[60] = "istore_1" ;
	instructions[61] = "istore_2" ;
	instructions[62] = "istore_3" ;
	instructions[63] = "lstore_0" ;
	instructions[64] = "lstore_1" ;
	instructions[65] = "lstore_2" ;
	instructions[66] = "lstore_3" ;
	instructions[67] = "fstore_0" ;
	instructions[68] = "fstore_1" ;
	instructions[69] = "fstore_2" ;
	instructions[70] = "fstore_3" ;
	instructions[71] = "dstore_0" ;
	instructions[72] = "dstore_1" ;
	instructions[73] = "dstore_2" ;
	instructions[74] = "dstore_3" ;
	instructions[75] = "astore_0" ;
	instructions[76] = "astore_1" ;
	instructions[77] = "astore_2" ;
	instructions[78] = "astore_3" ;
	instructions[79] = "iastore" ;
	instructions[80] = "lastore" ;
	instructions[81] = "fastore" ;
	instructions[82] = "dastore" ;
	instructions[83] = "aastore" ;
	instructions[84] = "bastore" ;
	instructions[85] = "castore" ;
	instructions[86] = "sastore" ;
	instructions[87] = "pop" ;
	instructions[88] = "pop2" ;
	instructions[89] = "dup" ;
	instructions[90] = "dup_x1" ;
	instructions[91] = "dup_x2" ;
	instructions[92] = "dup2" ;
	instructions[93] = "dup2_x1" ;
	instructions[94] = "dup2_x2" ;
	instructions[95] = "swap" ;
	instructions[96] = "iadd" ;
	instructions[97] = "ladd" ;
	instructions[98] = "fadd" ;
	instructions[99] = "dadd" ;
	instructions[100] = "isub" ;
	instructions[101] = "lsub" ;
	instructions[102] = "fsub" ;
	instructions[103] = "dsub" ;
	instructions[104] = "imul" ;
	instructions[105] = "lmul" ;
	instructions[106] = "fmul" ;
	instructions[107] = "dmul" ;
	instructions[108] = "idiv" ;
	instructions[109] = "ldiv" ;
	instructions[110] = "fdiv" ;
	instructions[111] = "ddiv" ;
	instructions[112] = "irem" ;
	instructions[113] = "lrem" ;
	instructions[114] = "frem" ;
	instructions[115] = "drem" ;
	instructions[116] = "ineg" ;
	instructions[117] = "lneg" ;
	instructions[118] = "fneg" ;
	instructions[119] = "dneg" ;
	instructions[120] = "ishl" ;
	instructions[121] = "lshl" ;
	instructions[122] = "ishr" ;
	instructions[123] = "lshr" ;
	instructions[124] = "iushr" ;
	instructions[125] = "lushr" ;
	instructions[126] = "iand" ;
	instructions[127] = "land" ;
	instructions[128] = "ior" ;
	instructions[129] = "lor" ;
	instructions[130] = "ixor" ;
	instructions[131] = "lxor" ;
	instructions[132] = "iinc" ;
	instructions[133] = "i2l" ;
	instructions[134] = "i2f" ;
	instructions[135] = "i2d" ;
	instructions[136] = "l2i" ;
	instructions[137] = "l2f" ;
	instructions[138] = "l2d" ;
	instructions[139] = "f2i" ;
	instructions[140] = "f2" ;
	instructions[141] = "f2d" ;
	instructions[142] = "d2i" ;
	instructions[143] = "d2l" ;
	instructions[144] = "d2f" ;
	instructions[145] = "i2b" ;
	instructions[146] = "i2c" ;
	instructions[147] = "i2s" ;
	instructions[148] = "lcmp" ;
	instructions[149] = "fcmpl" ;
	instructions[150] = "fcmpg" ;
	instructions[151] = "dcmpl" ;
	instructions[152] = "dcmpg" ;
	instructions[153] = "ifeq" ;
	instructions[154] = "ifne" ;
	instructions[155] = "iflt" ;
	instructions[156] = "ifge" ;
	instructions[157] = "ifgt" ;
	instructions[158] = "ifle" ;
	instructions[159] = "if_icmpeq" ;
	instructions[160] = "if_icmpne" ;
	instructions[161] = "if_icmplt" ;
	instructions[162] = "if_icmpge" ;
	instructions[163] = "if_icmpgt" ;
	instructions[164] = "if_icmple" ;
	instructions[165] = "if_aicmpeq" ;
	instructions[166] = "if_aicmpne" ;
	instructions[167] = "goto" ;
	instructions[168] = "jsr" ;
	instructions[169] = "ret" ;
	instructions[170] = "tableswitch" ;
	instructions[171] = "lookupswitch" ;
	instructions[172] = "ireturn" ;
	instructions[173] = "lreturn" ;
	instructions[174] = "freturn" ;
	instructions[175] = "dreturn" ;
	instructions[176] = "areturn" ;
	instructions[177] = "return" ;
	instructions[178] = "getstatic" ;
	instructions[179] = "putstatic" ;
	instructions[180] = "getfield" ;
	instructions[181] = "putfield" ;
	instructions[182] = "invokevirtual" ;
	instructions[183] = "invokespecial" ;
	instructions[184] = "invokestatic" ;
	instructions[185] = "invokeinterface" ;
	instructions[186] = "xxxunusedxxx" ;
	instructions[187] = "new" ;
	instructions[188] = "newarray" ;
	instructions[189] = "anewarray" ;
	instructions[190] = "arraylength" ;
	instructions[191] = "athrow" ;
	instructions[192] = "checkcast" ;
	instructions[193] = "instanceof" ;
	instructions[194] = "monitorenter" ;
	instructions[195] = "monitorexit" ;
	instructions[196] = "wide" ;
	instructions[197] = "multinewarray" ;
	instructions[198] = "ifnull" ;
	instructions[199] = "ifnonnull" ;
	instructions[200] = "goto_w" ;
	instructions[201] = "jsr_w" ;
	instructions[202] = "breakpoint" ;
	instructions[254] = "impdep1" ;
	instructions[255] = "impdep2" ;
}

string util::getInstruction(int i)
{
	if(util::blnInstructionsMapInitialised == false)
	{
		util::initialiseInstructionsMap() ;
		util::blnInstructionsMapInitialised = true ;
	}
	
	return util::instructions[i] ;
}

#endif
//end of code adde 7/11/2004

//code added 8/10/2004
string util::replaceSubstring(const string& s1, string s2, string s3)
{

    int i = 0 ;
    string copy ;
    int s1_len = s1.length() ;
    int s2_len = s2.length() ;
    int s3_len = s3.length() ;

    while(i < s1_len)
    {
        if(s1.substr(i,s2_len) == s2)
        {
            copy += s3 ;
            i += s2_len ;
            
        }
        else
        {
            copy += s1[i] ;
            i++ ;
        }
    }

    return copy ;
}
//end of code added 8/10/2005
