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
#include <string.h>
#include <cassert>

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "jvm.h"
#include "ClassFile.h"
//#include "Class.h"
#include "util.h"
//#include "value.h"
#include "logger.h"

//code added 6/8/2005, changed 6/8/2005
//extern string classpath ;
//end of code added 6/8/2005

//code added 14/8/2005
jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) ;
jobject NewObject(JNIEnv *env, jclass clazz, jmethodID methodID ...) ;
//end of code added 14/8/2005

const int jvm::VIRTUAL_MACHINE_ERROR = -1 ;
const int jvm::NO_CLASS_DEF_FOUND = 1 ;
const int jvm::FORMAT_ERROR = 2 ;
const int jvm::VERIFY_ERROR = 3 ;
const int jvm::LINKAGE_ERROR = 4 ;
const int jvm::CLASS_CIRCULARITY_ERROR = 5 ;
const int jvm::INCOMPATIBLE_CLASS_CHANGE_ERROR = 6 ;
/*
const int jvm::NULL_POINTER_EXCEPTION = 7;
const int jvm::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION = 8 ;
const int jvm::ARRAY_STORE_EXCEPTION = 9 ;
const int jvm::NEGATIVE_ARRAY_SIZE_EXCEPTION = 10 ;
const int jvm::ILLEGAL_MONITOR_STATE_EXCEPTION = 11 ;
*/

key::key(Class *l, string n):loader(l),fully_qualified_name(n)
{
} ;

loading_constraint::loading_constraint(string n, Class *l1, Class *l2)
	:class_name(n), L1(l1), L2(l2)
{
} ;


//this does the job of the Bootstrap Loader
void jvm::load_class(string class_name)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
  //Logger::LogFunctionEntry("jvm::load_class(): " + class_name) ;
  FunctionCallLogger fcl("jvm::load_class(): " + class_name) ;
#endif
//end of code added 6/11/2004

	//if it has already been recorded that the loading of 
	//this class has been intiated by the Bootstrap Loader,
	//throw a Linkage error (bullet item 1, Section 5.3.5)
	if(classes.find(key(NULL,class_name)) != classes.end())
		throw LinkageError() ;

	int i, j ;
	
	//if it is an array class, strip the leading '['
	if(class_name.at(0) == '[')
	{
		Class *rf = new Class() ;
		rf->name = class_name ;
		rf->blnArrayClass = true ;

		//for array classes, verification &
		//preparation are not necessary
		rf->blnVerified = true ;
		rf->blnPrepared = true ;

		rf->nodims = 1 ;

		string stripped_name = "" ;

		//it's class_name.length() - 1 because
		//class-array names end with a semi-colon (;)
		//which should be discarded
		for(int j=0; j < class_name.length() - 1; j++)
		{
			if(class_name.at(j) == '[')
			{
				rf->nodims++ ;
				continue ;
			}
			else
				stripped_name += class_name.at(j) ;
		}
		
		//this is for array names that don't end in 
		//semi-colons (primitive arrays)
		if(class_name.at(class_name.length()-1) != ';')
			stripped_name += class_name.at(class_name.length()-1) ;	

#if DEBUG_LEVEL >= 3
		if(stripped_name.length() > 0)
			Logger::Log("stripped name is : " + stripped_name) ;
#endif
		
		if(stripped_name == "B" ||
		   stripped_name == "C" ||
		   stripped_name == "D" ||
		   stripped_name == "F" ||
		   stripped_name == "I" ||
		   stripped_name == "J" ||
		   stripped_name == "S" ||
		   stripped_name == "Z") //component is a primitive type
		{
			rf->blnPrimitiveArray = true ;
#if DEBUG_LEVEL >= 3
			Logger::Log(class_name + " is a primitive") ;
#endif
			if(stripped_name == "B")
				rf->primitive_type = Class::PRIMITIVE_BYTE ;
			if(stripped_name == "C")
				rf->primitive_type = Class::PRIMITIVE_CHAR ;
			if(stripped_name == "D")
				rf->primitive_type = Class::PRIMITIVE_DOUBLE ;
			if(stripped_name == "F")
				rf->primitive_type = Class::PRIMITIVE_FLOAT ;
			if(stripped_name == "I")
				rf->primitive_type = Class::PRIMITIVE_INT ;
			if(stripped_name == "J")
				rf->primitive_type = Class::PRIMITIVE_LONG ;
			if(stripped_name == "S")
				rf->primitive_type = Class::PRIMITIVE_SHORT ;
			if(stripped_name == "Z")
				rf->primitive_type = Class::PRIMITIVE_BOOLEAN ;

			//if the array is a primitive array,
			//its accessabitiy is 'public'
			rf->blnPublic = true ;
		
		}
		else //component is a reference type
		{
			string fully_stripped_name = "" ;
			for(int k = 1; k < stripped_name.length(); k++)
				fully_stripped_name += stripped_name.at(k) ;

			//TO DO: this will only use the Bootstrap Loader 
			//to load the component reference; provision should
			//be made for the component reference to be loaded
			//by some other loader too
			typedef map<key, Class *>::const_iterator CI ;

			CI ci = classes.find(key(NULL,fully_stripped_name)) ;

			if(ci == classes.end())
			{
				try
				{
					load_class(fully_stripped_name) ;
				}
				catch(...)
				{
					throw ;
				}
			}

			//this will return an iterator to the
			//just-loaded class
			ci = classes.find(key(NULL,fully_stripped_name)) ;

#if DEBUG_LEVEL >= 3
			assert(ci != classes.end()) ;
#endif

			rf->array_class_ref = classes[ci->first];

			rf->defining_loader = NULL ;

			rf->initiating_loader = NULL ;

			if(blnLoadingConstViolated() == true)
			{
				delete rf ;
				//cout << "Exception LinkageError: " << class_name << endl ;
				throw LinkageError() ;
			}

			//accessibility of array class is the
			//same as that of its component reference type
			rf->blnPublic = rf->array_class_ref->blnPublic ;

		}

		//create class object


		object *cls_obj = createObject() ;

		cls_obj->blnClassObject = true ;
		cls_obj->class_pointer = rf ;

		rf->class_object = cls_obj ;

		classes[key(NULL,rf->name)] = rf ;


	} //end of if(class_name.at(0) == '[')
	else
	{

	  //code changed 16/5/2004
	  //a lot of code has been moved to load_class_internal()
	  //code physically removed instead of being commented out (to
	  //improve readability)

	  Class *rf1  = load_class_internal(class_name, NULL, NULL) ;

	  classes[key(NULL,rf1->name)] = rf1 ;
	  //end of code changed 16/5/2004
		
	} //end of else portion of if(class_name.at(0) == '[')

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::load_class(): " + class_name) ;
#endif
//end of code added 6/11/2004

	return ;
}

void jvm::init(string class_name, vector<string>& v)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
  //Logger::LogFunctionEntry("jvm::init()") ;
  FunctionCallLogger fcl("jvm::init()") ;
#endif
//end of code added 6/11/2004

  try
    {
      load_class(class_name) ;
    }
  catch(NoClassDefFound)
    {
      return ;
    }
  catch(FormatError)
    {
      return ;
    }

  try
    {
      link_class(classes[key(NULL,class_name)]) ;
    }
  catch(...)
    {
      return ;
    }

  //resolve the main() method
  //in the to-be-executed class
  Class *c ;	
  
  c = classes[key(NULL,class_name)] ;

  method *m ;

  try
    {
      m = resolve_class_method(c, class_name, "main", "([Ljava/lang/String;)V" ) ;
    }
  catch(...)
    {
      cout << "Unable to resolve method main() in class " << class_name << endl ;
      return ;
    }

  //create the main thread
  thread *main_thread = new thread() ;
  main_thread->thread_id = 1 ;
  main_thread->name = "main" ;
  main_thread->blnReady = true ;
  main_thread->nof_inst_in_curr_pass = 0 ;

  //code changed 25/7/2005
  //frame *fm = new frame() ;
  //fm->rcp = &(c->rcp) ;
  //fm->curr_method = m ;
  frame *fm = new frame(m) ;
  //end of code changed 25/7/2005

#if DEBUG_LEVEL >= 2
  cout << fm << ":" << fm->curr_method->type->name << ":" ;
  cout << fm->curr_method->name << endl ;
#endif

  fm->local_variables.reserve(m->max_locals + 1) ;
  
  main_thread->java_stack.push(fm) ;
  main_thread->pc = 0 ;

  curr_thread = 0 ;
  threads.push_back(main_thread) ;

  //populate arguments from the command line
  //parameters into the operand stack (in the form of
  //an array of strings)
  object *obj = createObject() ;
  obj->blnArrayObject = true ;

  LocalVariable *lv = createLocalVariable() ;
  lv->value_type = value::REFERENCE ;
  lv->ref_value = obj ; 
  fm->local_variables.push_back(lv) ;
    
  for(int it=1; it<m->max_locals; it++)
    fm->local_variables.push_back(NULL) ;    

  for(int i=0; i<v.size(); i++)
    {
      //string objects are constructed this way;
      //we check if an object containing the same
      //sequence of characters already exists;
      //if it does, this object is used
      if(strings.find(v.at(i)) != strings.end())
        obj->items.push_back(strings[v.at(i)]) ;
      else
      {
        object *string_obj = create_string_object(v.at(i)) ;
        setup_string_init(string_obj,v.at(i)) ;
        obj->items.push_back(string_obj) ;
        while(main_thread->java_stack.top() != fm)
          execute_instruction() ;
	}
    }  
  
  //TO DO: figure out a way to run the finaliser thread (currently run only from Shutdown())
  //(also the GC thread when we get around to implementing GC).
  //deferred for the time being

  //code added 26/4/2005
  //create_system_properties() ;
  //end of code addded 26/4/2005
    
  run() ;

  //code addded on 1/7/2004
  Shutdown(0) ;
  //end of code on 1/7/2004
  
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
  //Logger::LogFunctionExit("jvm::init()") ;
#endif
//end of code added 6/11/2004  
  
}

jvm::~jvm()
{

#if DEBUG_LEVEL >= 3
  FunctionCallLogger fcl("jvm::~jvm()") ;
#endif
//end of code added 6/11/2004

  typedef map<key, Class *>::const_iterator CI ;

  for(CI p=classes.begin(); p!= classes.end(); ++p)
    delete classes[p->first];

  for(int i=0; i<threads.size() ; i++)
    delete threads.at(i) ;

  typedef map<loading_constraint, loading_constraint *>::const_iterator CI2 ;

  for(CI2 p=constraint_set.begin(); p!= constraint_set.end(); ++p)
    delete constraint_set[p->first];
 
  //code changed 10/9/2005
  for(int i=0; i<heap.size(); i++)
  {
    //delete heap.at(i) ;
    if(heap.at(i) != NULL)
    {
      delete heap.at(i) ;
                   
      for(int j=i+1; j<heap.size(); j++)
      {
        if(heap.at(j) == heap.at(i))
          heap.at(j) = NULL ;
      }
      heap.at(i) = NULL ;
    }
  }
  //end of code changed 10/9/2005

  //code added 12/9/2005
  //uncomment this later or figure out  
  //a more efficient way (takes WAY too long)
  // (NOT REQUIRED; already freed in ~frame)
  /*
  for(int i=0; i<values.size(); i++)
  {
    //delete values.at(i) ;
    if(values.at(i) != NULL)
    {
      delete values.at(i) ;
                   
      for(int j=i+1; j<values.size(); j++)
      {
        if(values.at(j) == values.at(i))
          values.at(j) = NULL ;
      }
      values.at(i) = NULL ;
    } 
  }
  */

  for(int i=0; i<primitiveValues.size(); i++)
    delete primitiveValues.at(i) ;
  //end of code added 12/9/2005

  //code added 18/9/2005
  //uncomment this later (NOT REQUIRED; already freed
  //in ~frame)
  /*
  for(int i=0; i<localVariables.size(); i++)
  {
    //delete localVariables.at(i) ;
    cout << i << endl << flush ;
    if(localVariables.at(i) != NULL)
    {
      delete localVariables.at(i) ;
                   
      for(int j=i+1; j<localVariables.size(); j++)
      {
        if(localVariables.at(j) == localVariables.at(i))
          localVariables.at(j) = NULL ;
      }
      localVariables.at(i) = NULL ;
    }           
  }
  */
  //end of code added 18/9/2005
}

void jvm::prepare_class(Class *c)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::prepare_class()") ;
	FunctionCallLogger fcl("jvm::prepare_class()") ;
#endif
//end of code added 6/11/2004

	if(c->blnPrepared == true)
	{
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::prepare_class()") ;
#endif
//end of code added 6/11/2004		
		return ;
	}

	typedef map<string, static_value *>::const_iterator CI ;

	for(CI p=c->static_fields.begin(); p!= c->static_fields.end(); ++p)
	{

		//if the static field is a reference type,
		//load the class or interface to which it belongs
		static_value *v = c->static_fields[p->first] ;

		if(v->value_type == value::REFERENCE)
		{
			if(v->descriptor[0] == '[')
				v->type = resolve_class(c,v->descriptor) ;
			else
			{
				//remove the leading 'L' and the trailing ';'
				v->type = resolve_class(c,v->descriptor.substr(1,v->descriptor.length()-2)) ;
			}
		}

		c->static_fields[p->first]->bool_value = false ;
		c->static_fields[p->first]->i_value = 0 ;
		c->static_fields[p->first]->long_value = 0 ;
		c->static_fields[p->first]->float_value = 0 ;
		c->static_fields[p->first]->double_value = 0 ;

		object *null_obj = createObject() ;
		null_obj->blnNull = true ;
		c->static_fields[p->first]->ref_value = null_obj ;

		//no need to initialise ret_add_value

	}

	//impose loading constraints
	
	//skip this if we are dealing with
	//java/lang/Object
	if(c->super_class_name == "")
	{
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::prepare_class()") ;
#endif
//end of code added 6/11/2004		
		return ; 
	}

	Class *sc = c->super_class ;

	//code changed 17/11/2004
	//typedef map<method_key, method *>::const_iterator CI1 ;
	typedef map<string, method *>::const_iterator CI1 ;
	//end of code changed 17/11/2004

	for(CI1 p=c->methods.begin(); p!=c->methods.end(); p++)
	{
		if(c->methods[p->first]->name != "<init>" &&
		   c->methods[p->first]->name != "<clinit>")
		{

			for(CI1 q=sc->methods.begin(); q!=sc->methods.end(); q++)
			{

				//are the parameters the same?
				if(c->methods[p->first]->name 
				     == sc->methods[q->first]->name &&
				   c->methods[p->first]->parameter_string 
			     	     == sc->methods[q->first]->parameter_string)
				{
					if(util::is_primitive(c->methods[p->first]->return_descriptor) == false &&
					   c->methods[p->first]->return_descriptor != "V")
					{
						insert_constraint(c->methods[p->first]->return_descriptor,
								  c->defining_loader,
								  sc->defining_loader) ;

						if(blnLoadingConstViolated() == true)
						{
							cout << "Exception LinkageError" << endl ;
							delete_constraint(c->methods[p->first]->return_descriptor,
							  	  	  c->initiating_loader,
							  	  	  sc->initiating_loader) ;

							throw LinkageError() ;
						}

						for(int i=0; i<c->methods[p->first]->parameters->size(); i++)
						{
							if(util::is_primitive(c->methods[p->first]->parameters->at(i)) == false)
							{
								insert_constraint(c->methods[p->first]->parameters->at(i),
									  	  c->defining_loader,
									  	  sc->defining_loader) ;

								if(blnLoadingConstViolated() == true)
								{
									cout << "Exception LinkageError" << endl ;
									delete_constraint(c->methods[p->first]->parameters->at(i),
											  	  c->initiating_loader,
											  	  sc->initiating_loader) ;
	
									throw LinkageError() ;
								}
							} 
						}
					}
				}
			}
		} //end of c->methods[p->first]->name != "<init>")
	}

	for(int i=0; i<c->interfaces.size(); i++)
	{

		//si is the superinterface class
		Class *si = c->interfaces[i]->interface_ref ;

		for(CI1 p=c->methods.begin(); p!=c->methods.end(); p++)
		{

			for(CI1 q=si->methods.begin(); q!=si->methods.end(); q++)
			{
				//are the parameters the same?
				if(c->methods[p->first]->name 
				     == si->methods[q->first]->name &&
				   c->methods[p->first]->parameter_string 
			     	     == si->methods[q->first]->parameter_string)
				{
					if(util::is_primitive(c->methods[p->first]->return_descriptor) == false &&
					   c->methods[p->first]->return_descriptor != "V")
					{
						insert_constraint(c->methods[p->first]->return_descriptor,
								  c->defining_loader,
								  si->defining_loader) ;

						if(blnLoadingConstViolated() == true)
						{
							cout << "Exception LinkageError" << endl ;
							delete_constraint(c->methods[p->first]->descriptor,
								  	  c->initiating_loader,
								  	  si->initiating_loader) ;

							throw LinkageError() ;
						}

						for(int i=0; i<c->methods[p->first]->parameters->size(); i++)
						{
							if(util::is_primitive(c->methods[p->first]->parameters->at(i)) == false)
							{
								insert_constraint(c->methods[p->first]->parameters->at(i),
										  c->defining_loader,
										  si->defining_loader) ;

								if(blnLoadingConstViolated() == true)
								{
									cout << "Exception LinkageError" << endl ;
									delete_constraint(c->methods[p->first]->descriptor,
										  	  c->initiating_loader,
										  	  si->initiating_loader) ;

									throw LinkageError() ;
								}
							} 
						}
					}
				}
			}
		}
	}

	//end of imposing loading constraints

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::prepare_class()") ;
#endif
//end of code added 6/11/2004

	return ;
}

void jvm::link_class(Class *c)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::link_class()") ;
        FunctionCallLogger fcl("jvm::link_class()") ;
#endif
//end of code added 6/11/2004

	//verify and prepare the class
	try
	{
		verify_class(c) ;
		prepare_class(c) ;
	}
	catch(...)
	{
		cout << "Exception LinkageError: Verification of " << c->name << " failed" << endl ;
		throw ;
	}

	//verify and prepare its direct superclass
	if(c->super_class_name != "")
	{
		try
		{
			verify_class(c->super_class) ;
			prepare_class(c->super_class) ;
		}
		catch(...)
		{
			cout << "Exception LinkageError: Verification of " << c->name << " failed" << endl ;
			throw ;
		}

	}

	//verify and prepare its direct superinterfaces
	for(int i=0; i<c->interfaces.size(); i++)
	{
		try
		{
			verify_class(c->interfaces[i]->interface_ref) ;
			prepare_class(c->interfaces[i]->interface_ref) ;
		}
		catch(...)
		{
			cout << "Exception LinkageError: Verification of " << c->name << " failed" << endl ;
			throw ;
		}
	}

	//if c is an array type, prepare its
	//component type (if it is a reference)
	if(c->blnArrayClass == true && c->blnPrimitiveArray == false)
	{
		try
		{
			verify_class(c->array_class_ref) ;
			prepare_class(c->array_class_ref) ;
		}
		catch(...)
		{
			cout << "Exception LinkageError: Verification of " << c->name << " failed" << endl ;
			throw ;
		}
	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::link_class()") ;
#endif
//end of code added 6/11/2004

	return ;
}

void jvm::verify_class(Class *c)
{

//code added 31/7/2005
#if DEBUG_LEVEL >= 3
	assert(c != NULL) ;
#endif	
//end of code added 31/7/1005	
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::verify_class()") ;
        FunctionCallLogger fcl("jvm::verify_class()") ;
#endif
//end of code added 6/11/2004

	if(c->blnVerified == true)
	{
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::verify_class()") ;
#endif
//end of code added 6/11/2004
		return ;
	}

	//every class except Object should have a direct superclass
	if(c->name != "java/lang/Object" && c->super_class_name == "")
	{
		cout << "Exception VerifyError: " << c->name << " should have a superclass" << endl ;
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::verify_class()") ;
#endif		
		throw VerifyError() ;
	}

	if(c->name != "java/lang/Object")
	{
		Class *sc = c->super_class ;

		//check that final classes are not subclassed
		if(sc->blnFinal == true)
		{
			cout << "Exception VerifyError: " << c->name << " is derived from a superclass(" << sc->name << ") that is declared as final" << endl ;
#if DEBUG_LEVEL >= 3
			//Logger::LogFunctionExit("jvm::verify_class()") ;
#endif			
			throw VerifyError() ;
		}
		//end of check that final classes are not subclasseed

	
		//check that final methods are not overridden
		//code changed 17/11/2004
		//typedef map<method_key, method *>::const_iterator CI ;
		typedef map<string, method *>::const_iterator CI ;
		//end of code changed 17/11/2004
		
		for(CI p=c->methods.begin(); p!= c->methods.end(); ++p)
		{

			if(c->methods[p->first]->name != "<init>" &&
			   c->methods[p->first]->name != "<clinit>")
			{

				for(CI q=sc->methods.begin(); q!=sc->methods.end(); q++)
				{

					//1. are the parameters the same?
					//2. is the superclass method final?
					if(c->methods[p->first]->name 
				     	     == sc->methods[q->first]->name &&
					   c->methods[p->first]->parameter_string 
				     	     == sc->methods[q->first]->parameter_string &&
                                        //code changed 16/9/2005
					   //sc->methods[q->first]->blnFinal == true)
       					   sc->methods[q->first]->blnFinal == true &&
                                           c->methods[p->first]->name != "this")
                                        //end of code changed 16/9/2005
					{
						cout << "Exception VerifyError: " << c->name << " overrides a method(" << c->methods[p->first]->name << ") that is declared as final in its superclass(" << sc->name << ")" << endl ;
#if DEBUG_LEVEL >= 3
						//Logger::LogFunctionExit("jvm::verify_class()") ;
#endif						
						throw VerifyError() ;
					}

				}
			}
		}
		//end of check that final methods are not overridden

	}

	//check that the constant pool satisfies the
	//structural constraints
	if(c->verify() == false)
	{
		cout << "Exception VerifyError: " << c->name << "'s constant pool violates structural constraints" << endl ;
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::verify_class()") ;
#endif
		throw VerifyError() ;
	}

	//check that the field/method
	//names/descriptors are well formed
	if(c->verify_names() == false)
	{
		cout << "Exception VerifyError: " << c->name << "'s field/method name(s) are not well formed" << endl ;
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::verify_class()") ;
#endif
		throw VerifyError() ;
	}

	//TO DO: pass 3 (verifying the code section)

	c->blnVerified = true ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::verify_class()") ;
#endif
//end of code added 6/11/2004

	return ;
}

Class * jvm::resolve_class(Class *referring_class, string to_be_resolved_class)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::resolve_class()") ;
        FunctionCallLogger fcl("jvm::resolve_class()") ;
#endif
//end of code added 6/11/2004

//code added 14/2/2005
#if DEBUG_LEVEL >= 3
	assert(referring_class != NULL) ;
	assert(to_be_resolved_class != "") ;
#endif
//end of code added 14/2/2005

	//referring class has been loaded by the Bootstrap Loader
	if(referring_class->defining_loader == NULL)
	{
		//check if the class has already been loaded
		if(classes.find(key(NULL,to_be_resolved_class)) == classes.end())
		{
			try
			{
				load_class(to_be_resolved_class) ;
			}
			catch(...)
			{
				throw ;
			}
		}

		Class *c = classes[key(NULL,to_be_resolved_class)] ;

		if(to_be_resolved_class[0] == '[') //if its an array class
		{


			//it is an array of references
			if(c->blnPrimitiveArray	== false)
			{
				string component_type_name = 
					c->name.substr(c->nodims,
						       c->name.length() - c->nodims - 1) ;

				Class *ct ;

				try
				{
					ct = resolve_class(referring_class, component_type_name) ;
				}
				catch(...)
				{
					throw ;
				}

				c->array_class_ref = ct ;

			}
		}

		//the referring class has access to the to-be-resolved
		//class c if a) c is pubic or b) both the referring class
		//and c belong to the name runtime package.

		//though the runtime package is determined by
		//both the package name AND the defing loader,
		//since this part of the code assumes that
		//the loader is the Bootstrap Loader for both
		if(c->blnPublic == false && get_package_name(referring_class) != get_package_name(c))
		{
			throw IllegalAccessError() ;
			//cout << "Exception IllegalAccessError: " << referring_class->name << "..class..." << to_be_resolved_class << endl ;
		}
		

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::resolve_class()") ;
#endif
//end of code added 6/11/2004
		

		return classes[key(NULL,to_be_resolved_class)] ;
	}
	//TO DO: handle class loading by user defined loaders

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::resolve_class()") ;
#endif
//end of code added 6/11/2004

	return NULL ;
}


field * jvm::resolve_field(Class *referring_class, string class_name, string field_name, string descriptor, bool blnRaiseException)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::resolve_field()") ;
        FunctionCallLogger fcl("jvm::resolve_field()") ;
#endif
//end of code added 6/11/2004

	field *resolved_field = NULL ;

	Class *c ;

	//first resolve the class to which the field belongs
	try
	{
		c = resolve_class(referring_class, class_name) ;
	}
	catch(...)
	{
		throw ;
	}

	//see if this class defines the field
	for(int i=0; i<c->fields.size(); i++)
	{

		if(c->fields[i]->name == field_name &&
		   c->fields[i]->get_descriptor() == descriptor)
		{
			resolved_field = c->fields[i] ;
			break ;
		}
	}

        //TO DO: checking of superinterfaces moved after checking
        //of superclasses (steps 2 and 3 of section 5.4.3.2 of the
        //VM Spec interchanged). This was done to get rid of a assertion
        //failure (resolved field != NULL). Check if this is OK.
        
	//otherwise, if the class has a superclass, check if 
	//the superclass defines the field
	if(resolved_field == NULL && c->super_class_name != "")
	{
		try
		{
			resolved_field = resolve_field(referring_class,
						       c->super_class_name,
						       field_name,
						       descriptor,
					       	false) ;
		}
		catch(...)
		{
			if(blnRaiseException == true)
				throw ;
		}
	}

	//otherwise, check if the field in defined in the 
	//superinterfaces(if any) of the class
	if(resolved_field == NULL)
	{
		for(int i=0; i<c->interfaces.size(); i++)
		{
			resolved_field = resolve_field(referring_class,
						       c->interfaces[i]->interface_name,
						       field_name,
						       descriptor,
						       false) ;
			if(resolved_field != NULL)
				break ;

		}
	}
        
	//verify that the field is accessible to the 
	//referring class
	if(check_field_access(referring_class, c, resolved_field) == false)
	{
		if(blnRaiseException == true)
		{
			cout << referring_class->name << "..field is..." << resolved_field->name << endl ;
			throw IllegalAccessError() ;
		}
	}

	//impose loading constraints
	if(util::is_primitive(resolved_field->get_descriptor()) == false)
	{
	  insert_constraint(resolved_field->get_descriptor(),
				  c->defining_loader,
				  referring_class->defining_loader) ;

		if(blnLoadingConstViolated() == true)
		{
		  delete_constraint(resolved_field->get_descriptor(),
				  	  c->initiating_loader,
				  	  referring_class->initiating_loader) ;
			if(blnRaiseException == true)
			{
				throw LinkageError() ;
			}
		}
	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::resolve_field()") ;
#endif
//end of code added 6/11/2004

	return resolved_field ;
}	

//this method returns the package name 
//of the given class (package name is the
//portion of the class name upto,
//but excluding, the last slash)
string jvm::get_package_name(Class *c)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::get_package_name()") ;
        FunctionCallLogger fcl("jvm::get_package_name()") ;
#endif
//end of code added 6/11/2004

	string s ;

	if(c->blnArrayClass == true)
	{
#if DEBUG_LEVEL >= 3
		assert(c->blnPrimitiveArray == false) ;
#endif
		s = c->array_class_ref->name ;
	}
	else
		s = c->name ;

	int n = s.find_last_of("/") ;

	string retval ;

	if(n == string::npos)
		retval = "" ;
	else
		retval = s.substr(0,n) ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::get_package_name()") ;
#endif
//end of code added 6/11/2004

	return retval ;
}

//this method checks that a given
//field is accessible from the given class
bool jvm::check_field_access(Class *referring_class, Class *owner_class, field *f)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::check_field_access()") ;
        FunctionCallLogger fcl("jvm::check_field_access()") ;
#endif
//end of code added 6/11/2004

//code added 27/2/2005
#if DEBUG_LEVEL >= 3
	assert(referring_class != NULL) ;
	assert(owner_class != NULL) ;
	assert(f != NULL) ;
#endif
//end of code added 27/2/2005

	bool blnAccessible = false ;

	//code added 23/11/2004
	if(referring_class == owner_class)
		blnAccessible = true ;
	//end of code added 23/11/2004

	//if the field is public, it is accessible
	if(f->blnPublic == true)
		blnAccessible = true ;

	if(blnAccessible == false)
	{
		if(f->blnProtected == true)
		{
			//if referring class is a subclass of owner class
			//or is itself the owner class, the field is accessible
			if(referring_class->super_class_name == owner_class->name ||
			   referring_class->name == owner_class->name)
				blnAccessible = true ;
		}
	}

	if(blnAccessible == false)
	{
		//if the field is protected or package private
		if(f->blnProtected == true ||
		   (f->blnPublic == false &&
		    f->blnProtected == false &&
		    f->blnPrivate == false))
		{
			//if the field's class and the referring class
			//belong to the same runtime package
			if(get_package_name(referring_class) == get_package_name(owner_class))
				blnAccessible = true ;
		}
	}

	if(blnAccessible == false)
	{
		//if the field is private
		if(f->blnPrivate == true)
		{
			//if the field is declared in the referring class itself
			for(int i=0; i<referring_class->fields.size(); i++)
			{
				if(referring_class->fields[i]->name == f->name &&
				   referring_class->fields[i]->get_descriptor() == f->get_descriptor())
				{
					blnAccessible = true ;
					break ;
				}
			}

		}

	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::check_field_access()") ;
#endif
//end of code added 6/11/2004

	return blnAccessible ;

}

//this method checks that a given
//method is accessible from the given class
bool jvm::check_method_access(Class *referring_class, Class *owner_class, method *m)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::check_method_access()") ;
        FunctionCallLogger fcl("jvm::check_method_access()") ;
#endif
//end of code added 6/11/2004

	bool blnAccessible = false ;

	//if the method is public, it is accessible
	if(m->blnPublic == true)
		blnAccessible = true ;

	if(blnAccessible == false)
	{
		if(m->blnProtected == true)
		{
			//if referring class is a subclass of owner class
			//or is itself the owner class, the method is accessible
			if(referring_class->super_class_name == owner_class->name ||
			   referring_class->name == owner_class->name)
				blnAccessible = true ;
		}
	}

	if(blnAccessible == false)
	{
		//if the field is protected or package private
		if(m->blnProtected == true ||
		   (m->blnPublic == false &&
		    m->blnProtected == false &&
		    m->blnPrivate == false))
		{
			//if the method's class and the referring class
			//belong to the same runtime package
			if(get_package_name(referring_class) == get_package_name(owner_class))
				blnAccessible = true ;
		}
	}

	if(blnAccessible == false)
	{
		//if the method is private
		if(m->blnPrivate == true)
		{
			//if the method is declared in the referring class itself
			//code changed 17/11/2004
			//typedef map<method_key, method *>::const_iterator CI ;
			typedef map<string, method *>::const_iterator CI ;
			//end of code changed 17/11/2004
			
			for(CI p=referring_class->methods.begin(); p!=referring_class->methods.end(); ++p)
			{
				if(referring_class->methods[p->first]->name == m->name &&
				   referring_class->methods[p->first]->descriptor == m->descriptor)
				{
					blnAccessible = true ;
					break ;
				}
			}

		}

	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::check_method_access()") ;
#endif
//end of code added 6/11/2004

	return blnAccessible ;

}


method * jvm::resolve_class_method(Class *referring_class, string class_name, string method_name, string descriptor)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::resolve_class_method()") ;
        FunctionCallLogger fcl("jvm::resolve_class_method()") ;
#endif
//end of code added 6/11/2004
	
	bool blnMethodResolved = false ;

	method *resolved_method = NULL ;

	Class *c ;

	//first resolve the class to which the method belongs
	try
	{
		c = resolve_class(referring_class, class_name) ;
	}
	catch(...)
	{
		throw ;
	}

	//c should not be an interface
	if(c->blnInterface == true)
	{
		throw IncompatibleClassChangeError() ;
	}

	//check if c declares a method with 
	//the same name and descriptor
	//code changed 17/11/2004
	//typedef map<method_key, method *>::const_iterator CI ;
	typedef map<string, method *>::const_iterator CI ;
	//end of code changed 17/11/2004
	
	//code changed 17/11/2004
	//CI p = c->methods.find(method_key(method_name, descriptor)) ;
	CI p = c->methods.find(method_name + " %% " + descriptor) ;
	//end of code changed 17/11/2004

	if(p != c->methods.end())
	{
		resolved_method = c->methods[p->first] ;
		blnMethodResolved = true ;
	}
	else
	{
		//otherwise, check if the method is 
		//defined in c's superclass (if any) 
		//recursively
		if(c->super_class_name != "")
		{
			try
			{
				resolved_method = resolve_class_method(referring_class,
								c->super_class_name,
								method_name,
								descriptor) ;
				blnMethodResolved = true ;
			}
			catch(...)
			{
				//do nothing
			}
		}
	}

	if(blnMethodResolved == false)
	{
		//otherwise, check if the method in defined in the 
		//superinterfaces(if any) of the class
		for(int i=0; i<c->interfaces.size() && blnMethodResolved == false; i++)
		{
			try
			{
				resolved_method = resolve_class_method(referring_class,
							       c->interfaces[i]->interface_name,
							       method_name,
							       descriptor) ;
				blnMethodResolved = true ;
			}
			catch(...)
			{
				continue ;
			}
		}
	}

	if(blnMethodResolved == false)
		throw NoSuchMethodError();

	//if the resolved method is abstract, 
	//the class to which it belongs
	//should be abstract
	if(resolved_method->blnAbstract == true &&
		c->blnAbstract == false)
	{
		throw AbstractMethodError() ;
	}

	//verify that the method is accessible to the 
	//referring class
	//code changed 30/7/2005
	//TO DO: need to make this check more complete and accurate
	//if(resolved_method->name != "<init>" && check_method_access(referring_class, c, resolved_method) == false)
	if(resolved_method->name != "<init>" && check_method_access(referring_class, c, resolved_method) == false
	   && resolved_method->name != "clone")
	//end of code changed 30/7/2005	
	{
		cout << "referring class: " << referring_class->name << endl ;
		cout << "declaring class: " << class_name << endl ;
		cout << "method:" << resolved_method->name << endl ;
		throw IllegalAccessError() ;
	}

	//impose loading constraints
	if(util::is_primitive(resolved_method->return_descriptor) == false &&
	   resolved_method->return_descriptor != "V")
	{

		insert_constraint(resolved_method->return_descriptor,
				  c->defining_loader,
				  referring_class->defining_loader) ;

		if(blnLoadingConstViolated() == true)
		{
			delete_constraint(resolved_method->return_descriptor,
				  	  c->initiating_loader,
				  	  referring_class->initiating_loader) ;
			throw LinkageError() ;
		}

		for(int i=0; i<resolved_method->parameters->size(); i++)
		{
			if(util::is_primitive(resolved_method->parameters->at(i)) == false)
			{
				insert_constraint(resolved_method->parameters->at(i),
						  c->defining_loader,
						  referring_class->defining_loader) ;

				if(blnLoadingConstViolated() == true)
				{
					delete_constraint(resolved_method->parameters->at(i),
						  	  c->initiating_loader,
						  	  referring_class->initiating_loader) ;
					throw LinkageError() ;
				}
			} 
		}
	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::resolve_class_method()") ;
#endif
//end of code added 6/11/2004

//code added 20/8/2005
#if DEBUG_LEVEL >= 3
	assert(resolved_method != NULL) ;

	//TO DO: replace this with a validate() method 
	//in the 'method' class itself
	assert(resolved_method->type != NULL) ; 

#endif
//end of code added 20/8/2005

	return resolved_method ;
}

method * jvm::resolve_interface_method(Class *referring_class, string class_name, string method_name, string descriptor)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::reolve_interface_method()") ;
        FunctionCallLogger fcl("jvm::resolve_interface_method()") ;
#endif
//end of code added 6/11/2004	
	
	bool blnMethodResolved = false ;
	method *resolved_method ;

	//first resolve the class to which the method belongs
	Class *c ;

	try
	{
		c = resolve_class(referring_class, class_name) ;
	}
	catch(...)
	{
		throw ;
	}

	//c should be an interface
	if(c->blnInterface == false)
        {
		throw IncompatibleClassChangeError() ;
        }

	//check if c declares a method with 
	//the same name and descriptor
	
	//code changed 17/11/2004
	//typedef map<method_key, method *>::const_iterator CI ;
	//CI p = c->methods.find(method_key(method_name, descriptor)) ;
	typedef map<string, method *>::const_iterator CI ;
	CI p = c->methods.find(method_name + " %% " + descriptor) ;
	//end of code changed 17/11/2004

	if(p != c->methods.end())
	{
		resolved_method = c->methods[p->first];
		blnMethodResolved = true ;
	}
	else
	{
		//otherwise, check if the method in defined in the 
		//superinterfaces(if any) of the class
		for(int i=0; i<c->interfaces.size() && blnMethodResolved == false ; i++)
		{
			try
			{
				resolved_method = resolve_interface_method(referring_class,
									   c->interfaces[i]->interface_name,
									   method_name,
									   descriptor) ;
				blnMethodResolved = true ;
			}
			catch(...)
			{
				continue ; //i.e., try the next superinterface
			}
		}
	}

	if(blnMethodResolved == false)
	{
		//otherwise, check that the method
		//exists in the Object class
		Class *obj = classes[key(NULL,"java/lang/Object")] ;
		
		//code changed 17/11/2004
		//CI p1 = obj->methods.find(method_key(method_name, descriptor)) ;
		CI p1 = obj->methods.find(method_name + " %% " + descriptor) ;
		//end of code changed 17/11/2004

		if(p1 == obj->methods.end())
			throw NoSuchMethodError() ;

		resolved_method = obj->methods[p1->first] ;
	}

	//impose loading constraints
	if(util::is_primitive(resolved_method->return_descriptor) == false &&
	   resolved_method->return_descriptor != "V")
	{

		insert_constraint(resolved_method->return_descriptor,
				  c->defining_loader,
				  referring_class->defining_loader) ;

		if(blnLoadingConstViolated() == true)
		{
			delete_constraint(resolved_method->return_descriptor,
				  	  c->initiating_loader,
				  	  referring_class->initiating_loader) ;

			throw LinkageError() ;
		}

		for(int i=0; i<resolved_method->parameters->size(); i++)
		{
			if(util::is_primitive(resolved_method->parameters->at(i)) == false)
			{
				insert_constraint(resolved_method->parameters->at(i),
						  c->defining_loader,
						  referring_class->defining_loader) ;

				if(blnLoadingConstViolated() == true)
				{
					delete_constraint(resolved_method->parameters->at(i),
						  	  c->defining_loader,
						  	  referring_class->defining_loader) ;

					throw LinkageError() ;
				}
			} 
		}
	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::reolve_interface_method()") ;
#endif
//end of code added 6/11/2004

	return resolved_method ;
}

void jvm::insert_constraint(string name, Class *l1, Class *l2)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("jvm::insert_constraint()") ;
        FunctionCallLogger fcl("jvm::insert_constraint()") ;
#endif
//end of code added 6/11/2004	
	
	if(name[0] == 'L')
		name = name.substr(1,name.length()-2) ;

	//loading constraint is valid only
	//when the two loaders are different
	if(l1 == l2)
	{
//code added 13/11/2004
#if DEBUG_LEVEL >= 4
		//Logger::LogFunctionExit("jvm::insert_constraint()") ;
#endif
//end of code added 13/11/2004
		return ;
	}

	//add the constraint only if it doesn't already exist.
	//the order of l1 and l2 don't matter, i.e.,
	//(name,l1,l2) is equivalent to (name,l2,l1)
	if(constraint_set.find(loading_constraint(name,l1,l2)) == constraint_set.end()
	&& constraint_set.find(loading_constraint(name,l2,l1)) == constraint_set.end())
	{
		loading_constraint *lc = new loading_constraint(name,l1,l2) ;
		constraint_set[loading_constraint(name,l1,l2)] = lc ;
	}
	else
	{
//code added 13/11/2004
#if DEBUG_LEVEL >= 4
		//Logger::LogFunctionExit("jvm::insert_constraint()") ;
#endif
//end of code added 13/11/2004		
		return ;
	}

	//the following code is for enforcing
	//transitive closure

	typedef map<loading_constraint, loading_constraint *>::const_iterator CI ;

	//if any other constraint exists for name & l1, 
	//say (name, l1, x), insert (name,x,l2)
	for(CI p=constraint_set.begin(); p != constraint_set.end(); ++p)
	{
		if((constraint_set[p->first]->class_name == name &&
		    constraint_set[p->first]->L1 == l1 &&
		    constraint_set[p->first]->L2 != l2) ||
		   (constraint_set[p->first]->class_name == name &&
		    constraint_set[p->first]->L1 != l2 &&
		    constraint_set[p->first]->L2 == l1))
		{
			insert_constraint(name,constraint_set[p->first]->L2,l2) ;
		}
	}

	//if any other constraint exists for name & l2, 
	//say (name, x, l2), insert (name,l1,x)
	for(CI p=constraint_set.begin(); p != constraint_set.end(); ++p)
	{
		if((constraint_set[p->first]->class_name == name &&
		    constraint_set[p->first]->L1 != l1 &&
		    constraint_set[p->first]->L2 == l2) ||
		   (constraint_set[p->first]->class_name == name &&
		    constraint_set[p->first]->L1 == l2 &&
		    constraint_set[p->first]->L2 != l1))
		{
			insert_constraint(name,l1,constraint_set[p->first]->L1) ;
		}
	}

	//end code to enforce transitive closure
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("jvm::insert_constraint()") ;
#endif
//end of code added 6/11/2004	
}

void jvm::delete_constraint(string name, Class *l1, Class *l2)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("jvm::delete_constraint()") ;
        FunctionCallLogger fcl("jvm::delete_constraint()") ;
#endif
//end of code added 6/11/2004	
	
	//actually, only of these two will exist
	constraint_set.erase(loading_constraint(name,l1,l2)) ;
	constraint_set.erase(loading_constraint(name,l2,l1)) ;
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("jvm::delete_constraint()") ;
#endif
//end of code added 6/11/2004	
	
}

bool jvm::blnLoadingConstViolated()
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionEntry("jvm::blnLoadingConstViolated()") ;
        FunctionCallLogger fcl("jvm::blnLoadingConstViolated()") ;
#endif
//end of code added 6/11/2004

	typedef map<key,Class *>::const_iterator CI ;

	CI p, q ;

	for(p=classes.begin(); p!=classes.end(); ++p)
	{

		Class *C = classes[p->first] ;
		string N = C->name ;
		Class *L = C->initiating_loader ;


		//check whether there exists any other class with name = N
		//but with a different initiating loader

		vector<Class *> v ;

		for(q=classes.begin(); q!=classes.end(); ++q)
		{
			if(classes[q->first]->name == N &&
			   classes[q->first]->initiating_loader != L)
				v.push_back(classes[q->first]) ;
		}

		for(int i=0; i<v.size(); i++)
		{
			if(constraint_set.find(loading_constraint(N,L,v[i]->initiating_loader)) != constraint_set.end() ||
			    constraint_set.find(loading_constraint(N,v[i]->initiating_loader,L)) != constraint_set.end())
				return true ;
		}
	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 4
	//Logger::LogFunctionExit("jvm::blnLoadingConstViolated()") ;
#endif
//end of code added 6/11/2004

	return false ;
}

void jvm::run()
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::run()") ;
        FunctionCallLogger fcl("jvm::run()") ;
#endif
//end of code added 6/11/2004

	while(threads.size() > 0)
	{
		
		//rudimentary multi-threading mechanism:
		//execute 100 instructions in a thread,
		//then pass on to next thread
		if(threads.at(curr_thread)->blnReady == false ||
		   threads.at(curr_thread)->nof_inst_in_curr_pass == 100)
		{
			threads.at(curr_thread)->nof_inst_in_curr_pass = 0 ;
			if(curr_thread != threads.size()-1)
				curr_thread++ ;
			else
				curr_thread = 0 ;
		}

		execute_instruction() ;

		//temporary code; to be moved to
		//appropriate place later (this is required
		//here because execute_instruction() might have
		//resulted in the (only) thread being deleted
		//because of an unhandled exception)
		if(threads.size() > 0)
		{
			threads.at(curr_thread)->nof_inst_in_curr_pass++ ;

			//if a thread finishes, destroy it
			if(threads.at(curr_thread)->java_stack.empty() == true)
			{
				threads.erase(threads.begin()+curr_thread) ;
				if(curr_thread != threads.size()-1)
					curr_thread++ ;
				else
					curr_thread = 0 ;

			}
		}
	}	
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::run()") ;
#endif
//end of code added 6/11/2004	

}


//this is a utility function that determines
//whether a class is the super class of another class
bool jvm::IsSuperClass(Class *a, Class *b)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::IsSuperClass()") ;
        FunctionCallLogger fcl("jvm::IsSuperClass()") ;
#endif
//end of code added 6/11/2004	
	
	Class *t = b ;

	while(t->super_class_name != "")
	{
		if(t->super_class == a)
		{
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::IsSuperClass()") ;
#endif
//end of code added 6/11/2004	
			return true ;
		}
		t = t->super_class ;
	}
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::IsSuperClass()") ;
#endif
//end of code added 6/11/2004	
	
	return false ;
}

//utility function that determines whether a class
//implements an interface (note: though the parameters
//are Class variables, they can be interfaces too)
bool jvm::ImplementsInterface(Class *a, Class *b)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::ImplementsInterface()") ;
        FunctionCallLogger fcl("jvm::ImplementsInterface()") ;
#endif
//end of code added 6/11/2004

	assert(b->blnInterface == true) ;

	for(int i=0; i<a->interfaces.size(); i++)
	{
		if(a->interfaces.at(i)->interface_ref == b)
			return true ;
	}

	//code added 31/7/2005
	if(a->super_class_name != "")
	{		
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::ImplementsInterface()") ;
#endif		
		return(ImplementsInterface(a->super_class,b)) ;
	}
	//end of code added 31/7/2005
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::ImplementsInterface()") ;
#endif
//end of code added 6/11/2004
	
	return false ;
}

//this function checks whether two types are
//assignment compatible with each other
bool jvm::IsAssignmentCompatible(Class *S, Class *T)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
  //Logger::LogFunctionEntry("jvm::IsAssignmentCompatible()") ;
  FunctionCallLogger fcl("jvm::IsAssignmentCompatible()") ;      
#endif
//end of code added 6/11/2004

	//code added 15/8/2005
	//to take care of assignment of null objects
	//(null objects will have their type unset)
	if(S == NULL)
		return true ;
	//end of code added 15/8/2005
	
	bool retval = true ;

	//if S is a class type
	if(S->blnInterface == false && S->blnArrayClass == false)
	{
		//if T is a class type
		if(T->blnInterface == false && T->blnArrayClass == false)
		{
			//S must be the same class as T or
			//S must be a subclass of T
			if( S != T && IsSuperClass(T,S) == false)
				retval = false ;
		}
		//if T is an interface
		if(T->blnInterface == true)
		{
			//S must implement T
			if(ImplementsInterface(S,T) == false)
				retval = false ;
		}
	}
	//if S is an interface
	else if(S->blnInterface == true)
	{
		//if T is a class type, T must be Object
		if(T->blnInterface == false && 
		   T->blnArrayClass == false &&
		   T->name != "java/lang/Object")
			retval = false ;

		//if T is an interface type, T must be the same
		//as S or a superinterface of S
		//(note: we are using the same function,
		//ImplementsInterface(), to check whether a class
		//implements an interface, as well as whether
		//an interface is a superinterface of another interface;
		//the same structure in the class file serves both purposes)
		if( S != T && ImplementsInterface(S,T) == false)
			retval = false ;
	}
	//S is an array type
	else if(S->blnArrayClass == true)
	{
		//if T is a class type, T must be Object
		if(T->blnInterface == false && 
		   T->blnArrayClass == false &&
		   T->name != "java/lang/Object")
			retval = false ;

		//if T is an array type
		if(T->blnArrayClass == true)
		{

			//if S and T are arrays of primitives
			//both should contain the same primitive type
			if(S->blnPrimitiveArray == true &&
			   T->blnPrimitiveArray == true &&
			   S->primitive_type != T->primitive_type)
				retval = false ;

			//if the S and T are arrays of reference types,
			//these types shoud be assignment compatible 
			if(S->blnPrimitiveArray == false &&
			   T->blnPrimitiveArray == false)
			{
				Class *SC = S->array_class_ref ;
				Class *TC = T->array_class_ref ;
				retval = IsAssignmentCompatible(SC, TC) ;
			}

		}

		//if T is an interface type
		if(T->blnInterface == true)
		{
			//T must be Cloneable or java.io.Serializable
			if(T->name != "Cloneable" &&
			   T->name != "java/io/Serializable")
				retval = false ;
		}
		
	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::IsAssignmentCompatible()") ;
#endif
//end of code added 6/11/2004	
	
	return retval ;

}

object * jvm::create_exception_object(string exception_class_name)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::create_exception_object()") ;
        FunctionCallLogger fcl("jvm::create_exception_object()") ;
#endif
//end of code added 6/11/2004
	
	object *obj = createObject() ;

	typedef map<key, Class *>::const_iterator CI ;

	CI ci = classes.find(key(NULL,exception_class_name)) ;

	if(ci == classes.end())
		load_class(exception_class_name) ;

	ci = classes.find(key(NULL,exception_class_name)) ;

	InitialiseClass(classes[ci->first]) ;

	obj->type = classes[ci->first] ;

	//code added 21/8/02
	obj->blnNull = false ;
	//end code added 21/8/02

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::create_exception_object()") ;
#endif
//end of code added 6/11/2004

	return obj ;
}

value * jvm::create_value(object *obj)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::create_value()") ;
        FunctionCallLogger fcl("jvm::create_value()") ;
#endif
//end of code added 6/11/2004	
	
	value *val = createValue() ;

	val->value_type = value::REFERENCE ;
	val->ref_value = obj ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::create_value()") ;
#endif
//end of code added 6/11/2004

	return val ;
}

void jvm::throw_exception(string exception_class_name)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::throw_exception()") ;
        FunctionCallLogger fcl("jvm::throw_exception()") ;
#endif
//end of code added 6/11/2004
	
	//just for convenience's sake
	stack<value *> *op_st = &(threads.at(curr_thread)->java_stack.top()->operand_stack) ;

	op_st->push(create_value(create_exception_object(exception_class_name))) ;
	athrow() ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::throw_exception()") ;
#endif
//end of code added 6/11/2004	
	
}

value * jvm::duplicate(value *s)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::duplicate()") ;
        FunctionCallLogger fcl("jvm::duplicate()") ;
#endif
//end of code added 6/11/2004	
	
	value *d = createValue() ;

	d->value_type = s->value_type ;

	if(d->value_type == value::BOOLEAN ||
	   d->value_type == value::_BYTE ||
	   d->value_type == value::SHORT ||
	   d->value_type == value::INT ||
	   d->value_type == value::CHAR)
		d->i_value = s->i_value ;
	else if(d->value_type == value::LONG)
		d->long_value = s->long_value ;
	else if(d->value_type == value::FLOAT)
	{
		d->float_value = s->float_value ;
		d->IsNaN = s->IsNaN ;
		d->IsInfinity = s->IsInfinity ;
		d->IsPositive = s->IsPositive ;
	}
	else if(d->value_type == value::DOUBLE)
	{
		d->double_value = s->double_value ;
		d->IsNaN = s->IsNaN ;
		d->IsInfinity = s->IsInfinity ;
		d->IsPositive = s->IsPositive ;
	}
	else if(d->value_type == value::REFERENCE)
		//TO DO: should we make a copy of the reference?
		d->ref_value = s->ref_value ;
	else if(d->value_type == value::RETURN_ADDRESS)
		d->ret_add_value = s->ret_add_value ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::duplicate()") ;
#endif
//end of code added 6/11/2004

	return d ;
}

bool jvm::IsCategory1(value *v)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::IsCategory1()") ;
        FunctionCallLogger fcl("jvm::IsCategory1()") ;
#endif
//end of code added 6/11/2004	

	//code added 6/11/2004
	//TO DO: All functions should be made to have
	//one exit point (like here) so that the debug messages
	//(Entering/Exiting) have meaning
	bool retval ;
	//end of code added 6/11/2004
	
	if(v->value_type == value::BOOLEAN ||
	   v->value_type == value::_BYTE ||
	   v->value_type == value::CHAR ||
	   v->value_type == value::SHORT ||
	   v->value_type == value::INT ||
	   v->value_type == value::FLOAT ||
	   v->value_type == value::REFERENCE ||
	   v->value_type == value::RETURN_ADDRESS)
	   	//code changed 6/11/2004
		//return true ;
		retval = true ;
		//end of code changed 6/11/2004
	else
	   	//code changed 6/11/2004
		//return true ;
		retval = false ;
		//end of code changed 6/11/2004

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::IsCategory1()") ;
#endif
//end of code added 6/11/2004
		

	//code added 6/11/2004
	return retval ;
	//end of code added 6/11/2004
		
}

bool jvm::IsCategory2(value *v)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::IsCategory2()") ;
        FunctionCallLogger fcl("jvm::IsCategory2()") ;
#endif
//end of code added 6/11/2004	

	bool retval ;
	
	if(v->value_type == value::LONG ||
	   v->value_type == value::DOUBLE)
		retval = true ;
	else
		retval = false ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::IsCategory2()") ;
#endif
//end of code added 6/11/2004
		
	return retval ;
}

void jvm::InitialiseClass(Class *c)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::InitialiseClass()") ;
        FunctionCallLogger fcl("jvm::InitialiseClass()") ;
#endif
//end of code added 6/11/2004	

//code added 31/7/2005
#if DEBUG_LEVEL >= 3
	assert(c != NULL) ;
#endif	
//end of code added 31/7/2005	
	
	//TO DO: implement the procedure in 2.17.5

	if(c->blnInitialised == true)
	{
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::InitialiseClass()") ;
#endif
//end of code added 6/11/2004		
		return ;
	}

	//commented out because this resulted in the <clinit>
	//of the derived class getting executed ahead of that
	//of the parent class; this code moved after the
	//creation of the frame for the child class's <clinit>
	//if(c->super_class_name != "")
	//	InitialiseClass(c->super_class) ;

	try
	{
		if(c->blnVerified == false)
		{
			verify_class(c) ;
		}
	}
	catch(VerifyError)
	{
		throw_exception("java/lang/VerifyError") ;
		return ;
	}

	try
	{
		if(c->blnPrepared == false)
		{
			prepare_class(c) ;
		}
	}
	catch(LinkageError)
	{
		throw_exception("java/lang/LinkageError") ;
		return ;
	}

	//new frame will be created only if the class
	//contains a <clinit> method
	
	//code changed 17/11/2004
	//map<method_key, method *>::const_iterator p1 = c->methods.find(method_key("<clinit>","()V")) ;
	map<string, method *>::const_iterator p1 = c->methods.find("<clinit> %% ()V") ;
	//end of code changed 17/11/2004

	if(p1 != c->methods.end())
	{
		//code changed 25/7/2005
		//frame *fm = new frame() ;
		//fm->rcp = &(c->rcp) ;
		//fm->curr_method = c->methods[p1->first] ;
		frame *fm = new frame(c->methods[p1->first]) ;
		//end of code changed 25/7/2005
		
#if DEBUG_LEVEL >= 2
		cout << fm << ":" << fm->curr_method->type->name << ":" ;
		cout << fm->curr_method->name << endl ;
#endif

		//code added 27/3/2005
		fm->local_variables.reserve(c->methods[p1->first]->max_locals + 1) ;

		for(int i=0; i<c->methods[p1->first]->max_locals + 1; i++)
		  fm->local_variables.push_back(NULL) ;
		//end of code added 27/3/2005

		fm->prev_pc_value = threads.at(curr_thread)->pc ;

		threads.at(curr_thread)->java_stack.push(fm) ;
		threads.at(curr_thread)->pc = 0 ;	
	}

	if(c->super_class_name != "")
	  InitialiseClass(c->super_class) ;

	c->blnInitialised = true ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::InitialiseClass()") ;
#endif
//end of code added 6/11/2004

}

//this method checks if the given method
//is defined by the class or by
//its superclass or by its superclass's
//superclass and so on, recursively;
//if yes, it returns that method
method * jvm::GetMethod(Class *c, string method_name, string descriptor)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::GetMethod()") ;
        FunctionCallLogger fcl("jvm::GetMethod()") ;
#endif
//end of code added 6/11/2004	

//code added 17/11/2004
#if DEBUG_LEVEL >= 3
	assert(c != NULL) ;
#endif
//end of code added 17/11/2004

	//code changed 17/11/2004
	//typedef map<method_key, method *>::const_iterator CI ;
	typedef map<string, method *>::const_iterator CI ;
	//end of code changed 17/11/2004

	//code changed 17/11/2004
	//CI p = c->methods.find(method_key(method_name, descriptor)) ;
	CI p = c->methods.find(method_name + " %% " + descriptor) ;
	//end of code changed 17/11/2004

	if(p != c->methods.end())
	{
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
		//Logger::LogFunctionExit("jvm::GetMethod()") ;
#endif
//end of code added 6/11/2004
		return c->methods[p->first] ;
	}

	Class *t = c ;

	while(t->super_class_name != "")
	{
		t = t->super_class ;

		//code changed 17/11/2004
		//CI q = t->methods.find(method_key(method_name, descriptor)) ;
		CI q = t->methods.find(method_name + " %% " + descriptor) ;
		//end of code changed 17/11/2004

		if(q != t->methods.end())
		{
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
			//Logger::LogFunctionExit("jvm::GetMethod()") ;
#endif
//end of code added 6/11/2004			
			return t->methods[q->first] ;
		}

		//t = t->super_class ;
	}

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	cout << "GetMethod(): AbstractMethodError for class " << c->name << "." << method_name << endl << flush ;
	//Logger::LogFunctionExit("jvm::GetMethod()") ;
#endif
//end of code added 6/11/2004

	throw AbstractMethodError() ;

}

//this method creates a LocalVariable object
//from the given value object
LocalVariable * jvm::create_local_variable(value *v)
{

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::create_local_variable()") ;
        FunctionCallLogger fcl("jvm::create_local_variable()") ;
#endif
//end of code added 6/11/2004	
	
	LocalVariable *lv = createLocalVariable() ;

	lv->value_type = v->value_type ;

	lv->IsNaN = v->IsNaN ;		
	lv->IsInfinity = v->IsInfinity ;	
	lv->IsPositive = v->IsPositive ;	

	lv->bool_value = v->bool_value ;
	lv->i_value = v->i_value ;
	lv->long_value = v->long_value ;
	lv->float_value = v->float_value ;
	lv->double_value = v->double_value ;
	lv->ret_add_value = v->ret_add_value ;
	lv->ref_value = v->ref_value ;

	lv->valid_flag = true ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::create_local_variable()") ;
#endif
//end of code added 6/11/2004

	return lv ;
}

//this method clones the given primitive_value object
primitive_value * jvm::clone_primitive_value(primitive_value *v)
{
	
#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::clone_primitive_value()") ;
#endif
	
	primitive_value *pv = createPrimitiveValue() ;

	pv->value_type = v->value_type ;

	pv->IsNaN = v->IsNaN ;		
	pv->IsInfinity = v->IsInfinity ;	
	pv->IsPositive = v->IsPositive ;	

	pv->bool_value = v->bool_value ;
	pv->i_value = v->i_value ;
	pv->long_value = v->long_value ;
	pv->float_value = v->float_value ;
	pv->double_value = v->double_value ;

	return pv ;
}

//this method clones the given value object
value * jvm::clone_value(value *v)
{
	
#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::clone_value()") ;
#endif
	
	value *nv = createValue() ;

	nv->value_type = v->value_type ;

	nv->IsNaN = v->IsNaN ;		
	nv->IsInfinity = v->IsInfinity ;	
	nv->IsPositive = v->IsPositive ;	

	nv->bool_value = v->bool_value ;
	nv->i_value = v->i_value ;
	nv->long_value = v->long_value ;
	nv->float_value = v->float_value ;
	nv->double_value = v->double_value ;
	nv->ret_add_value = v->ret_add_value ;
	nv->ref_value = clone_object(v->ref_value) ;

	return nv ;
}

//this method deletes the pointers (of type value)
//in the passed stack; it alse empties the stack
void jvm::EmptyValueStack(stack<value *> *values)
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::EmptyValueStack()") ;
#endif
	
	while(!values->empty())
	{
		//delete values->top() ;
		values->pop() ;
	}
	
}

//this method clones the given object
//(ideally we should use the copy constructor?)
object *jvm::clone_object(object *s)
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::clone_object()") ;
#endif
	
	object *copy = createObject() ;

	copy->blnNull = s->blnNull ;

	copy->type = s->type ;

	copy->blnArrayObject = s->blnArrayObject ;

	copy->blnPrimitiveArrayObject = s->blnPrimitiveArrayObject ;

	copy->array_component_type = s->array_component_type ;

	copy->primitive_type = s->primitive_type ;

	//code changed 12/8/2005
	//copy->data = s->data ;
	if(s->blnStringObject == true)
		copy->data = s->data ;
	//end of code changed 12/8/2005

	for(int i=0; i<s->items.size(); i++)
		copy->items.push_back(clone_object(s->items.at(i))) ;

	for(int i=0; i<s->primitives.size(); i++)
		copy->primitives.push_back(clone_primitive_value(s->primitives.at(i))) ;

	//no need to copy monitor object
	//or ref_count

	//we are not copying the fields because it would then
	//become an infinite loop, since we would need to clone
	//the values making up the 'fields' map, in which case
	//we will have to clone objects (for values with value_type
	//as REFERENCE)
	//anyway, this method is to be used only for creating arrays
	//of arrays (multinewarray)

	return copy ;
}

bool jvm::bind_native_method(method *m)
{
	
//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::bind_native_method()") ;
	FunctionCallLogger fcl("jvm::bind_native_method()") ;
#endif
//end of code added 6/11/2004	
	
    void *retval ;

    //code added 31/1/04
    if(m->blnBound == true)
    {
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
      //Logger::LogFunctionExit("jvm::bind_native_method()") ;
#endif
//end of code added 13/11/2004
      return true ;
    }
    //end of code added 31/1/04

    //verify that the method is a native method
    assert(m->blnNative == true) ;


	//loop through the native_library_handles till we hit
	//a HINSTANCE that contains the required method

	char buf1[500], buf2[500] ;
	memset(buf1,'\0',500) ;
	memset(buf2,'\0',500) ;

	util::build_native_method_name1(m->type->name, m->name, buf1) ;
	util::build_native_method_name2(m->type->name, m->name, m->parameter_string, buf2) ;

//code added 5/8/2005
#if DEBUG_LEVEL >= 2
	cout << "bind_native_method(): " << buf1 << endl << flush ;
	cout << "bind_native_method(): " << buf2 << endl << flush ;
	cout << "bind_native_method(): " << m->parameter_string << endl << flush ;
#endif
//end of code added 5/8/2005

	for(int ii=0; ii<native_library_handles.size(); ii++)
	  {
	    HINSTANCE htemp1 = native_library_handles.at(ii) ;

#ifdef WIN32
	    retval = GetProcAddress(htemp1, buf1) ;
#else
	    retval = dlsym(htemp1,buf1) ;
#endif

	    if(retval == NULL)
	      {

#ifdef WIN32
		retval = GetProcAddress(htemp1,buf2) ;
#else
		retval = dlsym(htemp1,buf2) ;
#endif
    
		if(retval == NULL)
		    continue ; //we take up the next INSTANCE in the native_library_handles vector
		else
		    break ;

	      }
	    else
		break ;

	  } //end of for()

	//code changed 15/5/2004
	////end of if(htemp == NULL)
	//end of code changed 15/5/2004

    if(retval == NULL)
      {
	throw_exception("java/lang/UnsatisfiedLinkError") ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::bind_native_method() - java/lang/UnsatisfiedLinkError") ;
#endif
//end of code added 13/11/2004	
	return false ;
      }
    //end code changed 13/5/2004

    m->blnBound = true ;

    if(m->return_descriptor.at(0) == 'V')
        m->void_fnPtr = (void (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'B')
        m->jbyte_fnPtr = (jbyte (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'C')
        m->jchar_fnPtr = (jchar (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'D')
        m->jdouble_fnPtr = (jdouble (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'F')
        m->jfloat_fnPtr = (jfloat (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'I')
        m->jint_fnPtr = (jint (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'J')
        m->jlong_fnPtr = (jlong (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'S')
        m->jshort_fnPtr = (jshort (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'Z')
        m->jboolean_fnPtr = (jboolean (*)(JNIEnv *, jobject, ...))retval ;

    if(m->return_descriptor.at(0) == 'L' || m->return_descriptor.at(0) == '[')
        m->jobject_fnPtr = (object* (*)(JNIEnv *, jobject, ...))retval ;

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::bind_native_method()") ;
#endif
//end of code added 6/11/2004

    return true ;
}

//this method is for executing native methods
//code changed 3/8/2005
//void jvm::execute_native_method(method *m, value *obj_val, stack<value *>& values)
void jvm::execute_native_method(method *m, value *obj_val, stack<value *>& values, bool blnStaticMethod)
//end of code changed 3/8/2005
{

#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::execute_native_method()") ;
	FunctionCallLogger fcl("jvm::execute_native_method()") ;
#endif

//code added 1/8/2005
#if DEBUG_LEVEL >= 2	
    cout << "Entering execute_native_method()" << endl << flush ;
    cout << "Method: " << m->type->name << "." << m->name << endl << flush ;
    //cout << "Object: " << endl << flush ;
    //obj_val->display() ;
#endif	
//end of code addded 1/8/2005	
	
    //defined for convenience
    stack<value *> *op_st = &(threads.at(curr_thread)->java_stack.top()->operand_stack) ;

    assert(m->blnNative == true) ;
    assert(m->blnBound == true) ;

    //code added 3/8/2005
    jobject obj ;

    //code changed 24/9/2005
    //if(blnStaticMethod == true)
    //	    obj = obj_val->ref_value->class_pointer ;
    //else
    //	    obj = obj_val->ref_value ;
    obj = obj_val->ref_value ;
    //end of code changed 24/9/2005
    
    //end of code added 3/8/2005

    // we assume that a maximum of ten parameters
    // will be passed to the native method (excluding
    // JNIEnv * and jobject)
    word p[10] ;

    word *v = p ;
    union word64 w64 ;

    //vector<arg_values *> args ;

    int len = 0 ;

    while(!values.empty())
    {
        len++ ;

        value *val = values.top() ;
        values.pop() ;
	    
//code added 1/8/2005
#if DEBUG_LEVEL >= 3
	val->display() ;
#endif
//end of code added 1/8/2005	
	    
	    
        //arg_values a ;

        if(val->value_type == value::FLOAT)
        {
            *(float *)(v++) = val->float_value ;
        }
        else if(val->value_type == value::DOUBLE)
        {
            w64.d = val->double_value ;
            *v++ = w64.w.first ;
            *v++ = w64.w.second ;
        }
        else if(val->value_type == value::LONG)
        {
            w64.l = val->long_value ;
            *v++ = w64.w.first ;
            *v++ = w64.w.second ;
        }
        else
        {
            if(val->value_type == value::BOOLEAN)
                *v++ = (word)val->bool_value ;
            else if(val->value_type == value::_BYTE ||
                    val->value_type == value::SHORT ||
                    val->value_type == value::INT ||
                    val->value_type == value::CHAR)
                *v++ = (word)val->i_value ;
            else if(val->value_type == value::REFERENCE)
                *v++ = (word)val->ref_value ;
            else if(val->value_type == value::RETURN_ADDRESS)
                *v++ = (word)val->ret_add_value ;

            //args.push_back(&a) ;
        }

	//code changed 4/2/2005
        //delete val ;
        //end of code changed 4/2/2005
        
    }  //end of while

    //this evaluates to zero
    //int len = (v-p)/sizeof(word) ;

    //hack; we assume that the number of parameters to the
    //native method will be <= 10 (excluding the JNIEnv *
    //and jobject parameters)
    assert(len <= 10) ;

    for(int i=len; i<10 ; i++)
        //args.push_back(NULL) ;
      //code changed 15/2/2005
      //*v++ = (word)NULL ;
      *v++ = (word)NULL ;
      //end of code changed 15/2/2005

    //JNIEnv env = threads.at(curr_thread)->jni_env ;
    JNIEnv env = jni_env ;
    
    value *val ;

    if(m->return_descriptor.at(0) == 'V')
    {
      m->void_fnPtr(&env,
	            //code changed 3/8/2005
		    //obj_val->ref_value,
	            obj,
	            //end of code changed 3/8/2005
		    p[0], p[1], p[2], p[3], p[4],
		    p[5], p[6], p[7], p[8], p[9]) ;
    }
    else if(m->return_descriptor.at(0) == 'B')
    {
        val = createValue() ;
        val->value_type = value::_BYTE ;
        val->i_value = (jbyte)m->jbyte_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    else if(m->return_descriptor.at(0) == 'C')
    {
        val = createValue() ;
        val->value_type = value::CHAR ;
        val->i_value = (jchar)m->jchar_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    else if(m->return_descriptor.at(0) == 'D')
    {
        val = createValue() ;
        val->value_type = value::DOUBLE ;
        val->double_value = (jdouble)m->jdouble_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    else if(m->return_descriptor.at(0) == 'F')
    {
        val = createValue() ;
        val->value_type = value::FLOAT ;
        val->float_value = (jfloat)m->jfloat_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    else if(m->return_descriptor.at(0) == 'I')
    {
        val = createValue() ;
        val->value_type = value::INT ;
        val->i_value = (jint)m->jint_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    else if(m->return_descriptor.at(0) == 'J')
    {
        val = createValue() ;
        val->value_type = value::LONG ;
        val->long_value = (jlong)m->jlong_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    else if(m->return_descriptor.at(0) == 'S')
    {
        val = createValue() ;
        val->value_type = value::SHORT ;
        val->i_value = (jshort)m->jshort_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    else if(m->return_descriptor.at(0) == 'Z')
    {
        val = createValue() ;
        val->value_type = value::BOOLEAN ;
        val->bool_value = (jboolean)m->jboolean_fnPtr(&env,
					    //code changed 3/8/2005
					    //obj_val->ref_value,
					    obj,
					    //end of code changed 3/8/2005
                                            p[0], p[1], p[2], p[3], p[4],
                                            p[5], p[6], p[7], p[8], p[9]) ;
        op_st->push(val) ;
    }
    //code changed 12/8/2005
    //else if(m->return_descriptor.at(0) == 'L')
    else if(m->return_descriptor.at(0) == 'L' || m->return_descriptor.at(0) == '[')
    //end of code changed 12/8/2005
    {
        val = createValue() ;
        val->value_type = value::REFERENCE ;

        object *ref = m->jobject_fnPtr(&env,
				       //code changed 3/8/2005
				       //obj_val->ref_value,
				       obj,
				       //end of code changed 3/8/2005
                                     p[0], p[1], p[2], p[3], p[4],
                                     p[5], p[6], p[7], p[8], p[9]) ;

        //object *ref = new object(temp) ;

        val->ref_value = ref ;
        op_st->push(val) ;
    }
    //code added 12/8/2005
    else
    {
	    cout << "jvm::execute_native_method(): Invalid method descriptor: " << m->return_descriptor.at(0) << endl << flush ;
	    assert(false) ;
    }
    //code of code added 12/8/2005

//code added 6/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::execute_native_method()") ;
#endif
//end of code added 6/11/2004

//code added 1/8/2005    
#if DEBUG_LEVEL >= 2
        cout << "Exiting execute_native_method()" << endl << flush ;
#endif    
//end of code added 1/8/2005    
    return ;
}

//code added 16/5/2004
//see comment in jvm.h
Class *jvm::load_class_internal(string class_name, Class *init_loader, Class *defining_loader)
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::load_class_internal()") ;
#endif

  int i, j ;

  ClassFile cf ;

  try
    {

      //code changed 6/8/2005
      //cf.load(class_name) ;
      cf.load(class_name, classpath) ;
      //end of code changed 6/8/2005
	    
    }
  catch(NoClassDefFound)
    {
      cout << "Exception NoClassDefFoundError: " << class_name << "\n" ;
      throw ;
    }
  catch(FormatError)
    {
      cout << "Exception ClassFormatError: " << class_name << "\n" ;
      throw ;
    }
  catch(...)
    {
      cout << " Unspecified Error" << endl ;
      throw ;
    }

  /////////////////////////////////////////////
      //inserts element in the 'classes' map
      Class *rf = new Class() ;
    
      rf->name = class_name ;
      if(rf->name != "java/lang/Object")
	rf->super_class_name = cf.get_superclass_name() ;
      else
	rf->super_class_name = "" ;

      int acc_flags = cf.access_flags ;
      if(acc_flags / 0x400 == 1)
	rf->blnAbstract = true ;
      
      acc_flags = acc_flags % 0x400 ;
      if(acc_flags / 0x200 == 1)
	rf->blnInterface = true ;

      acc_flags = acc_flags % 0x200 ;
      if(acc_flags / 0x020 == 1)
	rf->blnSuper = true ;
      
      acc_flags = acc_flags % 0x020 ;
      if(acc_flags / 0x010 == 1)
	rf->blnFinal = true ;

      acc_flags = acc_flags % 0x010 ;
      if(acc_flags == 1)
	rf->blnPublic = true ;

      rf->constant_pool_count = cf.constant_pool_count ;

      rf->defining_loader = defining_loader ;

      rf->initiating_loader = init_loader ;

      if(blnLoadingConstViolated() == true)
	{
	  delete rf ;
	  //cout << "Exception LinkageError: " << class_name << endl ;
	  throw LinkageError() ;
	}

      //
      //this is temporary code; we should use
      //the automatic copy constructors to do this,
      //but something seems wrong
      //anyway, this will become unnecessary when
      //we merge ClassFile and Class

      rf->cp.reserve(rf->constant_pool_count-1) ;

      for(i=0; i<rf->constant_pool_count-1; i++)
	{

	  constant_pool *cp1 = new constant_pool() ;

	  cp1->valid_flag = cf.cp.at(i)->valid_flag ;

	  cp1->tag = cf.cp.at(i)->tag ;

	  //code added 28/2/2005
#if DEBUG_LEVEL >= 3
	  if(cp1->valid_flag == true)
	    assert(cp1->tag == constant_pool::CONSTANT_Class ||
		   cp1->tag == constant_pool::CONSTANT_Fieldref ||
		   cp1->tag == constant_pool::CONSTANT_Methodref ||
		   cp1->tag == constant_pool::CONSTANT_InterfaceMethodref ||
		   cp1->tag == constant_pool::CONSTANT_String ||
		   cp1->tag == constant_pool::CONSTANT_Integer ||
		   cp1->tag == constant_pool::CONSTANT_Float	||
		   cp1->tag == constant_pool::CONSTANT_Long ||
		   cp1->tag == constant_pool::CONSTANT_Double ||
		   cp1->tag == constant_pool::CONSTANT_NameAndType ||
		   cp1->tag == constant_pool::CONSTANT_Utf8) ;
#endif
	  //end of code added 28/2/2005

	  cp1->name_index = cf.cp.at(i)->name_index ;
	  cp1->class_index = cf.cp.at(i)->class_index ;
	  cp1->name_and_type_index = cf.cp.at(i)->name_and_type_index ;
	  cp1->string_index = cf.cp.at(i)->string_index ;
	  cp1->int_bytes = cf.cp.at(i)->int_bytes ;

	  for(j=0;j<4;j++)
	    cp1->float_bytes[j] = cf.cp.at(i)->float_bytes[j] ;

	  for(j=0;j<4;j++)
	    cp1->long_high_bytes[j] = cf.cp.at(i)->long_high_bytes[j] ;

	  for(j=0;j<4;j++)
	    cp1->long_low_bytes[j] = cf.cp.at(i)->long_low_bytes[j] ;

	  for(j=0;j<4;j++)
	    cp1->double_high_bytes[j] = cf.cp.at(i)->double_high_bytes[j] ;

	  for(j=0;j<4;j++)
	    cp1->double_low_bytes[j] = cf.cp.at(i)->double_low_bytes[j] ;

	  cp1->descriptor_index = cf.cp.at(i)->descriptor_index ;
	  cp1->length = cf.cp.at(i)->length ;
	  cp1->Utf8_bytes = cf.cp.at(i)->Utf8_bytes ;

	  rf->cp.insert(rf->cp.begin() + i, cp1) ;
	}

      //end temporary code

      rf->interfaces.reserve(cf.interfaces_count) ;

      for(i=0; i<cf.interfaces_count; i++)
	{
	  Interface *inf = new Interface() ;
	  inf->interface_name = rf->get_name(cf.interfaces[i]-1) ;
	  rf->interfaces.insert(rf->interfaces.begin() + i, inf) ;
	}

      rf->rcp.reserve(rf->constant_pool_count-1) ;

      for(i=0; i<rf->constant_pool_count-1; i++)
	{
	  runtime_constant_pool *r = new runtime_constant_pool() ;

	  r->valid_flag = cf.cp[i]->valid_flag ;

	  r->flag = -1 ;

	  if(r->valid_flag == true)
	    {

//code added 7/11/2004
#if DEBUG_LEVEL >= 3
		assert(cf.cp[i]->tag == constant_pool::CONSTANT_Class ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_Fieldref ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_Methodref ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_InterfaceMethodref ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_String ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_Integer ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_Float	||
		   cf.cp[i]->tag == constant_pool::CONSTANT_Long ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_Double ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_NameAndType ||
		   cf.cp[i]->tag == constant_pool::CONSTANT_Utf8) ;
#endif
//end of code added 7/11/2004


	      if(cf.cp[i]->tag == constant_pool::CONSTANT_Class)
		{
		  r->flag = runtime_constant_pool::CLASS ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("getting class name") ;
#endif
		  r->class_interface_name = rf->get_name(i) ;

#if DEBUG_LEVEL >= 4
		  Logger::Log("class: **** " + r->class_interface_name + " ***") ;
#endif
			
#if DEBUG_LEVEL >= 3			
		  //code added 15/2/2005
		  assert(r->class_interface_name != "") ;
		  //end of code added 15/2/2005
#endif
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_Fieldref)
		{
		  r->flag = runtime_constant_pool::FIELD ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("getting field name") ;
#endif
		  r->field_name = rf->get_name(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("field: **** " + r->field_name + " ***") ;
#endif
		  r->field_descriptor = rf->get_descriptor(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("getting parent class name") ;
#endif
		  r->field_parent_class_name = cf.get_parent_class_name(i) ;

#if DEBUG_LEVEL >= 3
		  assert(r->field_parent_class_name.length() > 0) ;
		  //Logger::Log("parent_class: **** " + r->field_parent_class_name + " ***") ;
#endif
			
#if DEBUG_LEVEL >= 4
		  Logger::Log("parent_class: **** " + r->field_parent_class_name + " ***") ;
#endif			
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_Methodref)
		{
		  r->flag = runtime_constant_pool::CLASS_METHOD ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("getting class method name") ;
#endif
		  r->method_name = rf->get_name(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("class method: **** " + r->method_name + " ***") ;
		  Logger::Log("getting descriptor") ;
#endif
		  r->method_descriptor = rf->get_descriptor(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("getting parent class name") ;
#endif
		  r->method_parent_class_name = cf.get_parent_class_name(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("parent_class: **** " + r->method_parent_class_name + " ***") ;
#endif
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_InterfaceMethodref)
		{
		  r->flag = runtime_constant_pool::INTERFACE_METHOD ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("getting interface name") ;
#endif
		  r->method_name = rf->get_name(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("interface method: **** " + r->method_name + " ***") ;
		  Logger::Log("getting descriptor") ;
#endif
		  r->method_descriptor = rf->get_descriptor(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("getting parent class name") ;
#endif
		  r->method_parent_class_name = cf.get_parent_class_name(i) ;
#if DEBUG_LEVEL >= 4
		  Logger::Log("parent_class: **** " + r->method_parent_class_name + " ***") ;
#endif
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_String)
		{
		  r->flag = runtime_constant_pool::STRING_LITERAL ;
		  r->str_value = rf->get_string_value(i) ;
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_Integer)
		{
		  r->flag = runtime_constant_pool::INTEGER_CONSTANT ;
		  r->int_value = rf->get_int_value(i) ;
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_Float)
		{
		  r->flag = runtime_constant_pool::FLOAT_CONSTANT ;
		  r->double_value = rf->get_float_value(i) ;
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_Long)
		{
		  r->flag = runtime_constant_pool::LONG_CONSTANT ;
		  r->int_value = rf->get_long_value(i) ;
		}

	      if(cf.cp[i]->tag == constant_pool::CONSTANT_Double)
		{
		  r->flag = runtime_constant_pool::DOUBLE_CONSTANT ;
		  r->double_value = rf->get_double_value(i) ;
		}
	    }

	  rf->rcp.insert(rf->rcp.begin() + i, r) ;

	}

      rf->fields.reserve(cf.fields_count) ;

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Populating fields") ;
#endif
//end of code added 7/11/2004

	
      for(i=0;i<cf.fields_count;i++)
	{

	  field *f = new field() ;

	  f->parent_class = rf ;

	  f->access_flags = cf.fi.at(i)->access_flags ;

	  int acc_flags = f->access_flags ;

	  if(acc_flags / 0x080 == 1)
	    f->blnTransient = true ;

	  acc_flags = acc_flags % 0x080 ;
	  if(acc_flags / 0x040 == 1)
	    f->blnVolatile = true ;

	  acc_flags = acc_flags % 0x040 ;
	  if(acc_flags / 0x010 == 1)
	    f->blnFinal = true ;

	  acc_flags = acc_flags % 0x010 ;
	  if(acc_flags / 0x008 == 1)
	    f->blnStatic = true ;

	  acc_flags = acc_flags % 0x008 ;
	  if(acc_flags / 0x004 == 1)
	    f->blnProtected = true ;

	  acc_flags = acc_flags % 0x004 ;
	  if(acc_flags / 0x002 == 1)
	    f->blnPrivate = true ;

	  acc_flags = acc_flags % 0x002 ;
	  if(acc_flags == 1)
	    f->blnPublic = true ;


	  f->name = rf->get_symbol_name(cf.fi.at(i)->name_index) ;

	  //code changed 27/2/2005
	  //f->descriptor = rf->get_symbol_name(cf.fi.at(i)->descriptor_index) ;
	  f->set_descriptor(rf->get_symbol_name(cf.fi.at(i)->descriptor_index)) ;
	  //end of code changed 27/2/2005

	  if(util::is_primitive(f->get_descriptor()) == true)
	    {
	      f->blnPrimitive = true ;

	      if(f->get_descriptor() == "B")
		f->value_type = primitive_value::_BYTE ;
	      else if(f->get_descriptor() == "C")
		f->value_type = primitive_value::CHAR ;
	      else if(f->get_descriptor() == "D")
		f->value_type = primitive_value::DOUBLE ;
	      else if(f->get_descriptor() == "F")
		f->value_type = primitive_value::FLOAT ;
	      else if(f->get_descriptor() == "I")
		f->value_type = primitive_value::INT ;
	      else if(f->get_descriptor() == "J")
		f->value_type = primitive_value::LONG ;
	      else if(f->get_descriptor() == "S")
		f->value_type = primitive_value::SHORT ;
	      else if(f->get_descriptor() == "Z")
		f->value_type = primitive_value::BOOLEAN ;
	      //code added 16/1/2005
	      else
	        assert(false) ;
	      //end of code added 16/1/2005	
	    }
	    //code added 16/1/2005	
	    else
	    {
		f->value_type = value::REFERENCE ;
	    }
	    //end of code added 16/1/2005




	  rf->fields.insert(rf->fields.begin() + i, f) ;

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Creating static fields (if any)") ;
#endif
//end of code added 7/11/2004

	  //create static fields
	  if(f->blnStatic == true)
	    {
	      static_value *v = new static_value() ;
	      v->descriptor = f->get_descriptor() ;
	      if(util::is_primitive(f->get_descriptor()) == true)
		{
		  if(f->get_descriptor() == "B")
		    v->value_type = value::_BYTE ;
		  else if(f->get_descriptor() == "C")
		    v->value_type = value::CHAR ;
		  else if(f->get_descriptor() == "D")
		    v->value_type = value::DOUBLE ;
		  else if(f->get_descriptor() == "F")
		    v->value_type = value::FLOAT ;
		  else if(f->get_descriptor() == "I")
		    v->value_type = value::INT ;
		  else if(f->get_descriptor() == "J")
		    v->value_type = value::LONG ;
		  else if(f->get_descriptor() == "S")
		    v->value_type = value::SHORT ;
		  else if(f->get_descriptor() == "Z")
		    v->value_type = value::BOOLEAN ;
	          //code added 16/1/2005
	          else
	            assert(false) ;
	          //end of code added 16/1/2005	
		}
	      else
		{
		  v->value_type = value::REFERENCE ;
		}
	      
	      rf->static_fields[f->name] = v ;
	      
	    } //end of if(f->blnStatic == true)
	}

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Populating methods") ;
#endif
//end of code added 7/11/2004
	
      for(i=0;i<cf.methods_count;i++)
	{
	  method *m = new method() ;

	  m->type = rf ;
                
	  m->access_flags = cf.mi.at(i)->access_flags ;

	  m->name = rf->get_symbol_name(cf.mi.at(i)->name_index) ;

	  int acc_flags = m->access_flags ;

	  if(acc_flags / 0x800 == 1)
	    m->blnStrict = true ;

	  acc_flags = acc_flags % 0x800 ;
	  if(acc_flags / 0x400 == 1)
	    m->blnAbstract = true ;

	  acc_flags = acc_flags % 0x400 ;
	  if(acc_flags / 0x100 == 1)
	    m->blnNative = true ;

	  acc_flags = acc_flags % 0x100 ;
	  if(acc_flags / 0x020 == 1)
	    m->blnSynchronized = true ;

	  acc_flags = acc_flags % 0x020 ;
	  if(acc_flags / 0x010 == 1)
	    m->blnFinal = true ;
          
	  acc_flags = acc_flags % 0x010 ;
	  if(acc_flags / 0x008 == 1)
	    m->blnStatic = true ;

	  acc_flags = acc_flags % 0x008 ;
	  if(acc_flags / 0x004 == 1)
	    m->blnProtected = true ;

	  acc_flags = acc_flags % 0x004 ;
	  if(acc_flags / 0x002 == 1)
	    m->blnPrivate = true ;

	  acc_flags = acc_flags % 0x002 ;
	  if(acc_flags  == 1)
	    m->blnPublic = true ;

	  m->descriptor = rf->get_symbol_name(cf.mi.at(i)->descriptor_index) ;
	  m->parameter_string = m->descriptor.substr(0,
						     m->descriptor.find(")") + 1) ;
	  m->return_descriptor = m->descriptor.substr(m->parameter_string.size(),
						      m->descriptor.size() - m->parameter_string.size()) ;

	  m->parameters = util::parse_params(m->parameter_string.substr(1,m->parameter_string.length()-2)) ;

	  //find out which attribute is the "Code" attribute
	  //read the code from the code vector of this attribute
	  for(j=0;j<cf.mi.at(i)->attributes_count; j++)
	    {
	      if(cf.mi.at(i)->attributes.at(j)->attribute_name_flag == 
		 attribute_info::CODE)
		{
		  //m->code = cf.mi.at(i)->attributes.at(j)->code ;
		  m->code.byte_code = cf.mi.at(i)->attributes.at(j)->code ;

		  //this explicit copying of the members of the
		  //exception table class is required (rather than direct 
		  //assignment) because the cf variable would go
		  //out of scope once this method ends, thereby 
		  //rendering the pointer members of ex_tbl unusable
		  for(int k=0; k<cf.mi.at(i)->attributes.at(j)->ex_tbl.size(); k++)
		    {
		      exception_table *et = new exception_table() ;

		      et->start_pc = cf.mi.at(i)->attributes.at(j)->ex_tbl.at(k)->start_pc ;
		      et->end_pc = cf.mi.at(i)->attributes.at(j)->ex_tbl.at(k)->end_pc ;
		      et->handler_pc = cf.mi.at(i)->attributes.at(j)->ex_tbl.at(k)->handler_pc ;
		      et->catch_type = cf.mi.at(i)->attributes.at(j)->ex_tbl.at(k)->catch_type ;

		      m->code.ex_tbl.insert(m->code.ex_tbl.begin() + k, et) ;
		    }

		  m->max_locals = cf.mi.at(i)->attributes.at(j)->max_locals ;

		  //Identify the "LineNumberTable" attribute
		  for(int k=0; k<cf.mi.at(i)->attributes.at(j)->attributes.size(); k++)
		    {
		      if(cf.mi.at(i)->attributes.at(j)->attributes.at(k)->attribute_name_flag == 
			 attribute_info::LINE_NUMBER_TABLE)
			{
			  for(int kk=0; kk<cf.mi.at(i)->attributes.at(j)->attributes.at(k)->lnt_info.size(); kk++)
			    {
			      line_number_table *lnt = new line_number_table() ;
			      lnt->start_pc = 	cf.mi.at(i)->attributes.at(j)->attributes.at(k)->lnt_info.at(kk)->start_pc ;
			      lnt->line_number = 	cf.mi.at(i)->attributes.at(j)->attributes.at(k)->lnt_info.at(kk)->line_number ;

			      m->line_numbers.push_back(lnt) ;
			    }
			  break ;
			}
		    }
		  
		  break ;
		}
	    }

	  //TO DO: populate exceptions vector of the 'method'
	  //class with the exceptions thrown by the method
	  //(get this from the "Exceptions" attribute
	  //of the attributes)

	  //code changed 17/11/2004
	  //rf->methods[method_key(m->name, m->descriptor)] = m ;
	  rf->methods[m->name + " %% " + m->descriptor] = m ;	  
	  //end of code changed 17/11/2004
	}

      //get the source file name of the class
      for(i=0; i<cf.ai.size(); i++)
	{

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Populating source file name") ;
#endif
//end of code added 7/11/2004

	  if(cf.ai.at(i)->attribute_name_flag == 
	     attribute_info::SOURCE_FILE)
	    {
	      rf->source_file_name = cf.get_symbol_name(cf.ai.at(i)->source_file_index);
	    }

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Populating inner classes (if any)") ;
#endif
//end of code added 7/11/2004

	  //code added 30/6/2004
	  //to handle inner classes
	  if(cf.ai.at(i)->attribute_name_flag == attribute_info::INNER_CLASSES)
	    {
	      for(int j=0; j<cf.ai.at(i)->number_of_classes; j++)
		{

		  string inner_class_name ;

		  int cp_index = cf.ai.at(i)->classes.at(j)->inner_class_info_index - 1 ;

		  for(int i=0;i < cf.cp.at(cf.cp.at(cp_index)->name_index - 1)->length; i++)
		    inner_class_name += cf.cp.at(cf.cp.at(cp_index)->name_index - 1)->Utf8_bytes.at(i) ;

		  //code changed 16/9/2005
                  //TO DO: this has to be uncommented after we figure
                  //out how to avoid the infinite recursion that happens
                  //when loading the inner class leads to an attempt to 
                  //load the outer class as well (there doesn't seem to
                  //be any impact with this commenting out right now 
                  //[i.e. as of comment date])
		  //if(class_name != inner_class_name)
		  // 	rf->inner_classes.push_back(load_class_internal(inner_class_name, NULL, NULL)) ;
		  //end of code changed 16/9/2005

		}

	      for(int j=0; j<rf->inner_classes.size(); j++)
		rf->inner_classes.at(j)->declaring_class = rf ;
	    }
	  //end of code added 30/6/2004
	  
	}

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Creating class object") ;
#endif
//end of code added 7/11/2004

      //create class object

      object *cls_obj = createObject() ;

      cls_obj->blnClassObject = true ;

      // code added 21/8/02
      cls_obj->blnNull = false ;
      // end code added 21/8/02

      cls_obj->class_pointer = rf ;

      rf->class_object = cls_obj ;

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Resolving superclasses (if any)") ;
#endif
//end of code added 7/11/2004

      //resolve superclass
      if(rf->super_class_name != "")
	{

	  //class cannot be its own superclass
	  if(rf->name == rf->super_class_name)
	    {
	      //cout << "Exception ClassCircularityError: " << class_name << "\n" ;
	      throw ClassCircularityError() ;
	    }

	  try
	    {
	      Class *c = resolve_class(rf, rf->super_class_name) ;

	      //code added 28/2/2005
#if DEBUG_LEVEL >= 3
	      assert(c != NULL) ;
#endif
	      //end of code added 28/2/2005

	      //superclass cannot be an interface
	      if(c->blnInterface == true)
		{
		  //cout << "Exception IncompatibleClassChangeError: " << class_name << "\n" ;
		  throw IncompatibleClassChangeError() ;
		}

	      rf->super_class = c ;

	    }
	  catch(...)
	    {
	      throw ;
	    }
	}

//code added 7/11/2004
#if DEBUG_LEVEL >= 4
	Logger::Log("Resolving superinterfaces (if any)") ;
#endif
//end of code added 7/11/2004

      //resolve superinterfaces (if any)
      for(i=0; i<rf->interfaces.size(); i++)
	{
	  
	  //class cannot be its own superintercface
	  if(rf->name == rf->interfaces[i]->interface_name)
	    {
	      //cout << "Exception ClassCircularityError: " << class_name << "\n" ;
	      throw ClassCircularityError() ;
	    }			
	  
	  try
	    {
	      Class *c = resolve_class(rf, rf->interfaces[i]->interface_name) ;

	      //it is actually an interface
	      if(c->blnInterface == false)
		{
		  //cout << "Exception IncompatibleClassChangeError: " << class_name << "\n" ;
		  throw IncompatibleClassChangeError() ;
		}

	      rf->interfaces[i]->interface_ref = c ;

	    }
	  catch(...)
	    {
	      throw ;
	    }
	}

      //TO DO: handle deletion of the various pointers that
      //get created in the case of any exceptions

      return rf ;

}
//end of code added 16/5/2004

//code added 1/7/2004 and subsequently filled in on 2/7/2004
void jvm::Shutdown(int status)
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::Shutdown()") ;
#endif	
	
  blnShuttingDown = true ;

  //the assumption here is that all the attributes of the threads
  //would have been correctly populated beforehand
  //when the thread was created prior to the call
  //to addShutdownHook()

  //no need to do anything if there are no shutdown hooks registered
  if(shutdown_hooks.size() > 0)
    {

      //we want the execution to start from the 
      //first of the shutdown hooks
      curr_thread = threads.size() ; //might as well hardcode this to 0

      //loop through the 'shutdown_hooks' vector and
      //add these threads to the 'threads' vector
      for(int i=0;i<shutdown_hooks.size(); i++)
	{
	  threads.push_back(shutdown_hooks.at(i)) ;
	}

      //we will start execution at the first thread in the 'shutdown_hooks' vector
      run() ;

    } //end of (shutdown_hooks.size() > 0)

  if(blnRunFinalizersOnExit == true)
    RunFinalisation() ;

  blnShuttingDown = false ; //not really needed (if control comes here, we are pretty much done)

}


void jvm::RunFinalisation()
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::RunFinalisation()") ;
#endif

  //IMPORTANT: the assumption is that this will be called
  //only from Shutdown(), i.e. the finaliser thread
  //will not be called when other threads are running.
  //actually, this code is pretty generic enough to work
  //even in other circumstances (if other threads are also running,
  //this thread will execute separately and terminate. other threads will be blocked
  //while it is running (not a Good Thing (tm)))

  vector<object *> objects_to_be_finalised ;

  //check that there are objects needing finalisation
  for(int i=0; i<heap.size(); i++)
    {
      if(heap.at(i)->blnFinalised == false)
	objects_to_be_finalised.push_back(heap.at(i)) ;
    }

  if(objects_to_be_finalised.size() == 0)
    return ; //no objects need finalisation

  thread *finaliser_thread = new thread() ;
  finaliser_thread->thread_id = threads.size() + 1 ;
  finaliser_thread->name = "finaliser" ;
  finaliser_thread->blnReady = true ;
  finaliser_thread->nof_inst_in_curr_pass = 0 ;

  threads.push_back(finaliser_thread) ;

  curr_thread = threads.size() - 1  ;

  for(int i=0; i<objects_to_be_finalised.size(); i++)
    {

      object *obj = objects_to_be_finalised.at(i) ;

      Class *c = obj->type ;

      //is this required? any other simpler way to get the 'finalize' method?
      method *m = resolve_class_method(c, c->name, "finalize", "()V" ) ;

      //code changed 25/7/2005
      //frame *fm = new frame() ;
      //fm->rcp = &(c->rcp) ;
      //fm->curr_method = m ;
      frame *fm = new frame(m) ;
      //end of code changed 25/7/2005

      fm->local_variables.reserve(m->max_locals + 1) ;

      //set the first local variable to the
      //this object whose finalize
      //method is being executed (in the 'main'
      //thread this is set to the class object, since
      //'main' is a static method)
      LocalVariable *lv = createLocalVariable() ;
      lv->value_type = value::REFERENCE ;
      lv->ref_value = obj ; 
      fm->local_variables.push_back(lv) ;

      for(int it=0; it<m->max_locals; it++)
	fm->local_variables.push_back(NULL) ;


      //no need to populate prev_pc_value
      //as this is the first frame

      finaliser_thread->java_stack.push(fm) ;
      finaliser_thread->pc = 0 ;

      while(finaliser_thread->java_stack.empty() == false)
	execute_instruction() ;

      //when control comes here, the finaliser thread will have no frames

    } //end of for()

  threads.erase(threads.begin() + curr_thread) ; //delete the finaliser thread

  delete finaliser_thread ;

}
//end of code added 1/7/2004

//code added 21/11/2004
//this method populates the 'system_properties' map.
//it also creates the string objects corresponding to the
//key/value pairs so that insertSystemProperties()
//in java.lang.VMRuntime can make use of them
//to invoke setProperty() on the java.util.Properties object
void jvm::PopulateSystemProperty(string key1, string value1)
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::PopulateSystemProperty()") ;
#endif

	object *keyObj = create_string_object(key1) ;

	//code added 2/5/2005
	thread *thrd = threads.at(curr_thread) ;
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 2/5/2005
	
	//code added 26/4/2005
	setup_string_init(keyObj, key1) ;
	
	//code changed 2/5/2005
	//execute_instruction() ;
	while(thrd->java_stack.top() != prev_frame)	
	{
		execute_instruction() ;
	}
	
	//end of code changed 2/5/2005
	//end of code added 26/4/2005
	
	object *valObj = create_string_object(value1) ;

	//code added 2/5/2005
	thrd = threads.at(curr_thread) ;
	prev_frame = thrd->java_stack.top() ;
	//end of code added 2/5/2005
	
	//code added 26/4/2005
	setup_string_init(valObj, value1) ;
	
	//code changed 2/5/2005
	//execute_instruction() ;
	while(thrd->java_stack.top() != prev_frame)	
	{
		execute_instruction() ;
	}
	
	//end of code changed 2/5/2005
	//end of code added 26/4/2005
	
	
	//code changed 26/2/2005	
	//population of 'strings' map is now done by
	//create_string_object() itself

	//strings[key1] = keyObj ;
	//strings[value1] = valObj ;
	//end of code changed 26/2/2005

	system_properties[key1] = value1 ;
	
	return ;
}
//end of code added 21/11/2004


//code added 13/2/2005
object *jvm::create_string_object(string s)
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::create_string_object()") ;
#endif

	typedef map<key, Class *>::const_iterator CI ;

	CI ci = classes.find(key(NULL,"java/lang/String")) ;

	object *strObj = createObject() ;

	strObj->blnStringObject = true ;

	//code added 22/3/2005
	//just for debugging; will be removed later (see object.h)
	strObj->string_val = s ;
	//end of code added 22/3/2005

	strObj->type = classes[ci->first] ;	

	for(int i=0; i<strObj->type->fields.size(); i++)
	{

		value *v = createValue() ;
		v->value_type = strObj->type->fields.at(i)->value_type ;
		if(v->value_type == value::BOOLEAN)
			v->bool_value = false ;
		else if(v->value_type == value::_BYTE ||
			v->value_type == value::SHORT ||
			v->value_type == value::CHAR ||
			v->value_type == value::INT)
			v->i_value = 0 ;
		else if(v->value_type == value::LONG)
			v->long_value = 0 ;
		else if(v->value_type == value::FLOAT)
			v->float_value = 0 ;
		else if(v->value_type == value::DOUBLE)
			v->double_value = 0 ;
		else if(v->value_type == value::REFERENCE)
		{
			object *obj1 = createObject() ;
			obj1->blnNull = true ;
			obj1->parent_object = strObj ;
			v->ref_value = obj1 ;

			//code added 24/2/2005
			if(strObj->type->fields.at(i)->get_descriptor().at(0) = '[')
			  obj1->blnArrayObject = true ;
			//end of code added 24/2/2005
		}
		else
			assert(false) ;

#if DEBUG_LEVEL >= 3
		assert(v != NULL) ;
#endif

		strObj->fields[strObj->type->fields.at(i)->name] = v ;

	}

	for(int i=0; i<s.length(); i++)
		strObj->data.push_back(s.at(i)) ;

	//code added 26/2/2005
	strings[s] = strObj ;
	//end of code added 26/2/2005

	return strObj ;
}
//end of code added 13/2/2005

//code added 13/2/2005
void jvm::create_system_properties()
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::create_system_properties()") ;
#endif	

#if DEBUG_LEVEL >= 2        
	cout << "entering create_system_properties()" << endl << flush ;
#endif        
	
	//PopulateSystemProperty("java.version","1.1") ;
	PopulateSystemProperty("java.vm.name","Vajra") ;

	//code added 30/7/2005
	PopulateSystemProperty("java.tmpdir","/tmp");
	//end of code added 30/7/20005	

	//code changed 3/3/2005
	//PopulateSystemProperty("java.vendor","") ;
	PopulateSystemProperty("java.vendor","Rajesh Jayaprakash") ;
	//end of code changed 3/3/2005

	PopulateSystemProperty("java.vendor.url","") ;
	PopulateSystemProperty("java.home","") ;
	PopulateSystemProperty("java.vm.specification.version","1.0") ;
	PopulateSystemProperty("java.vm.specification.vendor","Sun") ;
	PopulateSystemProperty("java.vm.specification.name","VMSpec") ;
	PopulateSystemProperty("java.vm.version","0.5") ;
	PopulateSystemProperty("java.vm.vendor","Rajesh Jayaprakash") ;
	PopulateSystemProperty("java.vm.name","Vajra") ;
	PopulateSystemProperty("java.specification.version","1.1") ;
	PopulateSystemProperty("java.specification.vendor","Classpath") ;
	PopulateSystemProperty("java.specification.name","Classpath") ;
	PopulateSystemProperty("java.class.version","") ;
	PopulateSystemProperty("java.class.path","") ;
	
	//code changed 30/7/2005
	//PopulateSystemProperty("java.library.path","") ;
	PopulateSystemProperty("java.library.path","/usr/local/classpath/lib/classpath") ;
	//end of code changed 30/7/2005
	
	PopulateSystemProperty("java.io.tmpdir","") ;
	PopulateSystemProperty("java.compiler","") ;
	PopulateSystemProperty("java.ext.dirs","") ;
	PopulateSystemProperty("os.name","Linux") ;
	PopulateSystemProperty("os.arch","x86") ;
	PopulateSystemProperty("os.version","") ;
#ifndef WIN32
	PopulateSystemProperty("file.separator","/") ;
	PopulateSystemProperty("path.separator",":") ;
#else
	PopulateSystemProperty("file.separator","\\") ;
	PopulateSystemProperty("path.separator",";") ;
#endif
	PopulateSystemProperty("line.separator","\n") ;
	PopulateSystemProperty("user.name","") ;
	PopulateSystemProperty("user.home","") ;
	PopulateSystemProperty("user.dir","") ;	

#if DEBUG_LEVEL >= 2
	cout << "exiting create_system_properties()" << endl << flush ;
#endif

}
//end of code added 13/2/2005

//code added 29/5/2005
LocalVariable *jvm::create_local_variable(jvalue *v, string param_type)
{
	
#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::create_local_variable()") ;
#endif	
	LocalVariable *lv = createLocalVariable() ;
		
	if(param_type == "B")
	{
		lv->value_type = value::_BYTE ;
		lv->i_value = v->b ;
	}
	else if(param_type == "C")
	{
		lv->value_type = value::CHAR ;
		lv->i_value = v->c ;
	}
	else if(param_type == "D")
	{
		lv->value_type = value::DOUBLE ;
		lv->double_value = v->d ;
	}
	else if(param_type == "F")
	{
		lv->value_type = value::FLOAT ;
		lv->float_value = v->f ;
	}
	else if(param_type == "I")
	{
		lv->value_type = value::INT ;
		lv->i_value = v->i ;
	}
	else if(param_type == "J")
	{
		lv->value_type = value::LONG ;
		lv->long_value = v->j ;
	}
	else if(param_type[0] == 'L' || param_type[0] == '[')
	{
		lv->value_type = value::REFERENCE ;
		lv->ref_value = (object *)v->l ;
	}
	else if(param_type == "S")
	{
		lv->value_type = value::SHORT ;
		lv->i_value = v->s ;
	}
	else if(param_type == "Z")
	{
		lv->value_type = value::BOOLEAN ;
		if(v->j > 0)
			lv->bool_value =  true ;
		else
			lv->bool_value =  false ;
	}	
	
	return lv ;
}
//end of code added 29/5/2005

//code added 7/8/2005
//this does the job of InitialiseClass() and, in addtion
//executes the <clinit> method as well
void jvm::InitialiseClass1(Class *c)
{

#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::InitialiseClass1()") ;
#endif

#if DEBUG_LEVEL >= 3
	assert(c != NULL) ;
#endif	
	
	//TO DO: implement the procedure in 2.17.5

	if(c->blnInitialised == true)
		return ;
	
	try
	{
		if(c->blnVerified == false)
			verify_class(c) ;
	}
	catch(VerifyError)
	{
		throw_exception("java/lang/VerifyError") ;
		return ;
	}

	try
	{
		if(c->blnPrepared == false)
			prepare_class(c) ;
	}
	catch(LinkageError)
	{
		throw_exception("java/lang/LinkageError") ;
		return ;
	}	

	map<string, method *>::const_iterator p1 = c->methods.find("<clinit> %% ()V") ;	
	
	if(p1 != c->methods.end())
	{
		frame *fm = new frame(c->methods[p1->first]) ;
		
#if DEBUG_LEVEL >= 2
		cout << fm << ":" << fm->curr_method->type->name << ":" ;
		cout << fm->curr_method->name << endl ;
#endif

		fm->local_variables.reserve(c->methods[p1->first]->max_locals + 1) ;

		for(int i=0; i<c->methods[p1->first]->max_locals + 1; i++)
		  fm->local_variables.push_back(NULL) ;

		fm->prev_pc_value = threads.at(curr_thread)->pc ;

		frame *prev_frame = threads.at(curr_thread)->java_stack.top() ;
		
		threads.at(curr_thread)->java_stack.push(fm) ;
		threads.at(curr_thread)->pc = 0 ;

		while(threads.at(curr_thread)->java_stack.top() != prev_frame)	
			execute_instruction() ;		

	}
	
	if(c->super_class_name != "")
	  InitialiseClass1(c->super_class) ;

	c->blnInitialised = true ;

	return ;
	
}
//end of code added 7/8/2005

//code added 9/8/2005
object *jvm::getClassObject(Class *c)
{
#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::getClassObject()") ;
#endif	
        
	if(c->_class_object == NULL)
	{
		c->_class_object = createObject() ; 
		
		typedef map<key, Class *>::const_iterator CI ;
		
		CI ci = classes.find(key(NULL,"java/lang/Class")) ;	      
		
                //code changed 24/9/2005
		//assert(ci != classes.end());
                if(ci == classes.end())
                {
		        load_class("java/lang/Class") ;
        	        ci = classes.find(key(NULL,"java/lang/Class")) ;
                }
                //end of code changed 24/9/2005
	
		Class *jlc = classes[ci->first] ;
		
                //code changed 24/9/2005
		//if(jlc->blnInitialised == false)
		//	InitialiseClass1(jlc) ;
                //end of code changed 24/9/2005
		
		c->_class_object->type = jlc ;
		
		c->_class_object->class_pointer = c ;
	
	}
	return c->_class_object ;			
}
//end of code added 9/8/2005

//code added 13/8/2005
object *jvm::getMethodObject(method *m)
{
#if DEBUG_LEVEL >= 3
	FunctionCallLogger fcl("jvm::getMethodObject()") ;
#endif	
	if(m->method_object == NULL)
	{
		typedef map<key, Class *>::const_iterator CI ;

		string class_name ;
		
		if(m->name == "<init>")
			class_name = "java/lang/reflect/Constructor" ;
		else
			class_name = "java/lang/reflect/Method" ;
		
		CI ci = classes.find(key(NULL,class_name)) ;	      
		
		if(ci == classes.end())
			load_class(class_name) ;
	
		ci = classes.find(key(NULL,class_name)) ;
		
		assert(ci != classes.end()) ;
		
		Class *jlrm = classes[ci->first] ;
		
		if(jlrm->blnInitialised == false)
			InitialiseClass1(jlrm) ;

		m->method_object = createObject(jlrm) ; 
		
		m->method_object->type = jlrm ;

		//if it is a Constructor, populate the private data members
		//'parameterTypes', 'clazz', 'slot' and 'exceptionTypes' 

		if(class_name == "java/lang/reflect/Constructor")
		  {

		    object *parameterTypesObj = createObject() ;
		    parameterTypesObj->blnArrayObject = true ;
		
		    ci = classes.find(key(NULL,"java/lang/Class")) ;
		    assert(ci != classes.end()) ;
		
		    parameterTypesObj->array_component_type = classes[ci->first] ;
		
		    for(int i=0; i<m->parameters->size(); i++)
		      {
			//TO DO: to handle primitives
			
			//strip the leading 'L' and trailing ';'
			string param_class_name ;
			if(m->parameters->at(i)[0] == 'L')
				param_class_name = m->parameters->at(i).substr(1,
									       m->parameters->at(i).length() - 2) ;
			else
				param_class_name = m->parameters->at(i) ;
			
			ci = classes.find(key(NULL,param_class_name)) ;
			if(ci == classes.end())
				load_class(param_class_name) ;
			
			Class *c = util::getClassFromName(this, param_class_name, NULL) ;
			parameterTypesObj->items.push_back(getClassObject(c)) ;
		      }
		
		    value *v = createValue() ;
		    v->value_type = value::REFERENCE ;
		    v->ref_value = parameterTypesObj ;
		
		    m->method_object->fields["parameterTypes"] = v ;

		    object *clazzObj = getClassObject(m->type) ;

		    value *v1 = createValue() ;
		    v1->value_type = value::REFERENCE ;
		    v1->ref_value = clazzObj ;
		
		    m->method_object->fields["clazz"] = v1 ;

		    //TO DO: similar code for slot and exceptionTypes

		  }
		
		m->method_object->method_pointer = m ;
		
	}
	return m->method_object ;			
}
//end of code added 13/8/2005

//code added 14/8/2005
//this method creates an object and populates
//its fields, but does not execute <init>
object *jvm::createObject(Class *cls)
{
	object *obj = createObject() ;

	obj->type = cls ;

	for(int i=0; i<cls->fields.size(); i++)
	{
		value *v = createValue() ;
		v->value_type = cls->fields.at(i)->value_type ;
		if(v->value_type == value::BOOLEAN)
			v->bool_value = false ;
		else if(v->value_type == value::_BYTE ||
				v->value_type == value::SHORT ||
				v->value_type == value::CHAR ||
				v->value_type == value::INT)
			v->i_value = 0 ;
		else if(v->value_type == value::LONG)
			v->long_value = 0 ;
		else if(v->value_type == value::FLOAT)
			v->float_value = 0 ;
		else if(v->value_type == value::DOUBLE)
			v->double_value = 0 ;
		else if(v->value_type == value::REFERENCE)
		{
			object *obj1 = createObject() ;
			obj1->blnNull = true ;
			v->ref_value = obj1 ;
		}

		obj->fields[cls->fields.at(i)->name] = v ;
	}

	//we also need to create entries in the 'fields'
	//map for the fields of superclasses of this class
	Class *t = cls->super_class ;
	
	while(t != NULL)
	{
		for(int i=0; i<t->fields.size(); i++)
		{
			value *v = createValue() ;
			v->value_type = t->fields.at(i)->value_type ;
			if(v->value_type == value::BOOLEAN)
				v->bool_value = false ;
			else if(v->value_type == value::_BYTE ||
				v->value_type == value::SHORT ||
				v->value_type == value::CHAR ||
				v->value_type == value::INT)
				v->i_value = 0 ;
			else if(v->value_type == value::LONG)
				v->long_value = 0 ;
			else if(v->value_type == value::FLOAT)
				v->float_value = 0 ;
			else if(v->value_type == value::DOUBLE)
				v->double_value = 0 ;
			else if(v->value_type == value::REFERENCE)
			{
				object *obj1 = createObject() ;
				obj1->blnNull = true ;
				obj1->parent_object = obj ;
				v->ref_value = obj1 ;
			}
			else
				assert(false) ;
			obj->fields[t->fields.at(i)->name] = v ;
		} //end of for
		
		t = t->super_class ;
		
	}
	return obj ;
}
//end of code added 14/8/2005
