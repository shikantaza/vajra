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

//this file contains JNI-related functions

#include <assert.h>

#include <iostream>

#ifndef WIN32
#include <dlfcn.h>
#endif

//for handling variable-length arguments
#include <cstdarg>

#include "jvm.h"
#include "util.h"

extern jvm *theJVM ;

jint GetVersion(JNIEnv *env)
{
	return 0x13 ; //i.e. 1.3
}

jclass DefineClass(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize buflen)
{

	ClassFile cf ;

	//code added 16/5/2004
	string class_name(name) ;
	//end of code added 16/5/2004

	int i, j ;

	try
	{
	  //code changed 16/5/2004
	  //modified load() of ClassFile to take third parameter
	  //as part of refactoring the two load() methods
	  //cf.load((char *)buf, (int)buflen) ;
	  cf.load((char *)buf, (int)buflen, class_name) ;
	  //end of code changed 16/5/2004
	}
	catch(NoClassDefFound)
	{
		throw ;
	}
	catch(FormatError)
	{
		throw ;
	}
	catch(...)
	{
		cout << " Unspecified Error" << endl ;
		throw ;
	}


	//code changed 16/5/2004
	//a lot of code has been moved to load_class_internal()
	//code physically removed instead of being commented out (to
	//improve readability)

	//TODO: second and third parameters to be changed to loader (jobject)
	//once the definition of Class has been changed from Class *
	Class *rf = theJVM->load_class_internal(class_name, NULL, NULL) ;

	theJVM->classes[key(NULL,rf->name)] = rf ;
	//end code changed 16/5/2004

        //code changed 24/9/2005
	//return rf ;
        return (jclass)theJVM->getClassObject(rf) ;
        //end of code changed 24/9/2005
}

jclass FindClass(JNIEnv *env, const char *name)
{
	string s(name) ;

	//theJVM->load_class(s) ;
        
        typedef map<key, Class *>::const_iterator CI ;
        
        CI ci = theJVM->classes.find(key(NULL,s)) ;	      
		
	if(ci == theJVM->classes.end())
		theJVM->load_class(s) ;
	
	ci = theJVM->classes.find(key(NULL,s)) ;
		
	assert(ci != theJVM->classes.end()) ;        

        //code changed 24/9/2005        
	//return(theJVM->classes[ci->first]) ;
        return((jclass)theJVM->getClassObject(theJVM->classes[ci->first])) ;
        //end of code changed 24/9/2005
}

jclass GetSuperclass(JNIEnv *env, jclass clazz)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object *)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005

	assert(cls != NULL) ;


	if(cls->name == "java/lang/Object")
		return NULL ;
	else
                //code changed 24/9/2005
		//return cls->super_class ;
                return (jclass)(theJVM->getClassObject(cls->super_class)) ;
                //end of code changed 24/9/2005
}

jboolean IsAssignableFrom(JNIEnv *env, jclass class1, jclass class2)
{
        
        //code changed 24/9/2005        
	//Class *cls1 = static_cast<Class *>(class1) ;
        object* obj1 = (object *)class1 ;
        Class* cls1 = obj1->class_pointer ;
        //end of code changed 24/9/2005        
        
	assert(cls1 != NULL) ;

	//code changed 24/9/2005        
	//Class *cls2 = static_cast<Class *>(class1) ;
        object* obj2 = (object *)class2 ;
        Class* cls2 = obj2->class_pointer ;
        //end of code changed 24/9/2005        
        
	assert(cls2 != NULL) ;

	//1) the classes are the same or
	//2) the first class is a subclass of the second class
	//3) the first class has the second class as one of its interfaces
	if(cls1 == cls2 ||
	   theJVM->IsSuperClass(cls2, cls1) == true ||
	   theJVM->ImplementsInterface(cls1, cls2) == true)
		return JNI_TRUE ;
	else
		return JNI_FALSE ;
}

jint Throw(JNIEnv *env, jthrowable obj)
{
	stack<value *> *op_st = &(theJVM->threads.at(theJVM->curr_thread)->java_stack.top()->operand_stack) ;

	op_st->push(theJVM->create_value((object *)obj)) ;
	theJVM->athrow() ;

	return 0 ;
}

jint ThrowNew(JNIEnv *env, jclass clazz, const char *message)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj1 = (object *)clazz ;
        Class* cls = obj1->class_pointer ;
        //end of code changed 24/9/2005

	object *obj = theJVM->createObject() ;

	obj->type = cls ;

	for(int i=0; i<strlen(message); i++)
		obj->data.push_back(message[i]) ;

	stack<value *> *op_st = &(theJVM->threads.at(theJVM->curr_thread)->java_stack.top()->operand_stack) ;

	op_st->push(theJVM->create_value(obj)) ;
	theJVM->athrow() ;

	return 0 ;
}

jthrowable ExceptionOccurred(JNIEnv *env)
{
	if(theJVM->threads.at(theJVM->curr_thread)->blnExceptionBeingThrown == true)
		return((jthrowable)theJVM->threads.at(theJVM->curr_thread)->exception_object) ;
	else
		return NULL ;
}

void ExceptionDescribe(JNIEnv *env)
{
	if(theJVM->threads.at(theJVM->curr_thread)->blnExceptionBeingThrown == true)
	{
		stack<string> temp_stack = theJVM->threads.at(theJVM->curr_thread)->exception_trace ;

		while(!temp_stack.empty())
		{
			cout << temp_stack.top() << endl ;
			temp_stack.pop() ;
		}
	}
}

void ExceptionClear(JNIEnv *env)
{
	if(theJVM->threads.at(theJVM->curr_thread)->blnExceptionBeingThrown == true)
	{
		theJVM->threads.at(theJVM->curr_thread)->blnExceptionBeingThrown = false ;	

		while(!theJVM->threads.at(theJVM->curr_thread)->exception_trace.empty())
		{
			theJVM->threads.at(theJVM->curr_thread)->exception_trace.pop() ;
		}
	}
}

void FatalError(JNIEnv *env, const char *msg)
{
	cout << "Fatal error:" << msg << endl ;
	exit(0) ;
}

jobject NewGlobalRef(JNIEnv *env, jobject obj)
{
	theJVM->heap.push_back((object *)obj) ;
	return obj ;
}

void DeleteGlobalRef(JNIEnv *env, jobject globalref)
{
	//loop through the 'heap' vector, identify the 
	//reference to be deleted and remove this entry
	//from the vector
	for(int i=0; i<theJVM->heap.size() ; i++)
	{
		if(theJVM->heap.at(i) == (object *)globalref)
			theJVM->heap.erase(theJVM->heap.begin() + i) ;
	}

}

void DeleteLocalRef(JNIEnv *env, jobject localref)
{
	//loop through the thread's local reference registry, 
	//identify the reference to be deleted and remove 
	//this entry from the registry
	for(int i=0; i<theJVM->threads.at(theJVM->curr_thread)->local_ref_registry.size() ; i++)
	{
		if(theJVM->threads.at(theJVM->curr_thread)->local_ref_registry.at(i) == (object *)localref)
			theJVM->threads.at(theJVM->curr_thread)->local_ref_registry.erase(theJVM->threads.at(theJVM->curr_thread)->local_ref_registry.begin() + i) ;
	}

}

jobject AllocObject(JNIEnv *env, jclass clazz)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj1 = (object *)clazz ;
        Class* cls = obj1->class_pointer ;
        //end of code changed 24/9/2005

	if(cls->blnInterface == true || cls->blnAbstract == true)
	{
		theJVM->throw_exception("java/lang/InstantiationException") ;
		return NULL ;
	}

	object *obj = theJVM->createObject() ;

	obj->type = cls ;

	return obj ;
}

jobject NewObject(JNIEnv *env, jclass clazz, jmethodID methodID ...)
{

//code added 4/8/2005
#if DEBUG_LEVEL >= 2
	cout << "Entering NewObject()" << endl << flush ;
#endif	
//end of code added 4/8/2005	
	
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj1 = (object*)clazz ;
        Class* cls = obj1->class_pointer ;
        //end of code changed 24/9/2005

	if(cls->blnInterface == true || cls->blnAbstract == true)
	{
		theJVM->throw_exception("java/lang/InstantiationException") ;
		return NULL ;
	}

	LocalVariable *obj_val = theJVM->createLocalVariable() ;
	obj_val->value_type = value::REFERENCE ;

	object *obj = theJVM->createObject() ;


	obj->type = cls ;

	for(int i=0; i<cls->fields.size(); i++)
	{
		value *v = theJVM->createValue() ;
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
			object *obj1 = theJVM->createObject() ;
			obj1->blnNull = true ;
			v->ref_value = obj1 ;
		}

		obj->fields[cls->fields.at(i)->name] = v ;
	}

	//code added 5/8/2005
	//we also need to create entries in the 'fields'
	//map for the fields of superclasses of this class
	Class *t = cls->super_class ;
	
	while(t != NULL)
	{
		for(int i=0; i<t->fields.size(); i++)
		{
			value *v = theJVM->createValue() ;
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
				object *obj1 = theJVM->createObject() ;
				obj1->blnNull = true ;
				obj1->parent_object = obj ;
				v->ref_value = obj1 ;
			}
			else
				assert(false) ;
			obj->fields[t->fields.at(i)->name] = v ;
		} //end of for
		
		t = t->super_class ;
		
	} //end of while
	//end of code adde 5/8/2005
	
	obj_val->ref_value = obj ;

	//construct a new frame for this method and
	//invoke this method

	//code changed 4/8/2005
	//frame *frm = new frame() ;
	//frm->rcp = &(cls->rcp) ;
	frame *frm = new frame((method *)methodID) ;
	//end of code changed 4/8/2005

	frm->local_variables.push_back(obj_val) ;

	//standard code to extract variable parameters
	//(The C++ Programming Language by Stroustrup (3rd Ed) -- pp 155-156)
	va_list ap ;

	va_start(ap, methodID) ;

	//code changed 4/8/2005
        //TO DO: this is to be replaced by a call to populateLocalVariablesForFrame()
	//for(;;)
	//{
	//	
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//
	//	if(val == 0)
	//		break ;
	//	frm->local_variables.push_back(val) ;
	//}
	method *m = (method *)methodID ;
	for(int i=0; i<m->parameters->size(); i++)
	{
		LocalVariable *val = theJVM->createLocalVariable() ;
		if(m->parameters->at(i) == "B")
		{
			val->value_type = value::_BYTE ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "C")
		{
			val->value_type = value::CHAR ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "D")
		{
			val->value_type = value::DOUBLE ;
			val->double_value = va_arg(ap, double) ;
			LocalVariable *dummy = theJVM->createLocalVariable() ;
			dummy->valid_flag = false;
			frm->local_variables.push_back(dummy) ;
		}
		else if(m->parameters->at(i) == "F")
		{
			val->value_type = value::FLOAT ;
			val->float_value = va_arg(ap, double) ;
		}
		else if(m->parameters->at(i) == "I")
		{
			val->value_type = value::INT ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "J")
		{
			val->value_type = value::LONG ;
			val->long_value = va_arg(ap, long) ;
			LocalVariable *dummy = theJVM->createLocalVariable() ;
			dummy->valid_flag = false;
			frm->local_variables.push_back(dummy) ;
		}
		else if(m->parameters->at(i)[0] == 'L')
		{
			val->value_type = value::REFERENCE ;
			val->ref_value = (object *)va_arg(ap, jobject) ;
		}
		else if(m->parameters->at(i) == "S")
		{
			val->value_type = value::SHORT ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "Z")
		{
			val->value_type = value::BOOLEAN ;
			int temp = va_arg(ap, int) ;
			val->i_value = (temp = 0) ? false : true ;
		}		
		else if(m->parameters->at(i)[0] == '[')
		{
			val->value_type = value::REFERENCE ;
			val->ref_value = (object *)va_arg(ap, jobject) ;
		}
		frm->local_variables.push_back(val) ;
	}
	//end of code added 4/8/2005

	va_end(ap) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

//code added 4/8/2005
#if DEBUG_LEVEL >= 2
	cout << "Exiting NewObject()" << endl << flush ;
#endif	
//end of code added 4/8/2005	

	return obj ;
}

jobject NewObjectA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj1 = (object*)clazz ;
        Class* cls = obj1->class_pointer ;
        //end of code changed 24/9/2005

	if(cls->blnInterface == true || cls->blnAbstract == true)
	{
		theJVM->throw_exception("java/lang/InstantiationException") ;
		return NULL ;
	}

	LocalVariable *obj_val = theJVM->createLocalVariable() ;
	obj_val->value_type = value::REFERENCE ;

	object *obj = theJVM->createObject() ;

	obj->type = cls ;

	for(int i=0; i<cls->fields.size(); i++)
	{
		value *v = theJVM->createValue() ;
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
			object *obj1 = theJVM->createObject() ;
			obj1->blnNull = true ;
			v->ref_value = obj1 ;
		}

		obj->fields[cls->fields.at(i)->name] = v ;
	}

	//code added 5/8/2005
	//we also need to create entries in the 'fields'
	//map for the fields of superclasses of this class
	Class *t = cls->super_class ;
	
	while(t != NULL)
	{
		for(int i=0; i<t->fields.size(); i++)
		{
			value *v = theJVM->createValue() ;
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
				object *obj1 = theJVM->createObject() ;
				obj1->blnNull = true ;
				obj1->parent_object = obj ;
				v->ref_value = obj1 ;
			}
			else
				assert(false) ;
			obj->fields[t->fields.at(i)->name] = v ;
		} //end of for
		
		t = t->super_class ;
		
	} //end of while
	//end of code adde 5/8/2005	
	
	obj_val->ref_value = obj ;

	//construct a new frame for this method and
	//invoke this method

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	frm->local_variables.push_back(obj_val) ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i = 0; i < size ; i++)
	{
		frm->local_variables.push_back((LocalVariable *)(args+i)) ;
	}

	frm->curr_method = mthd ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	return obj ;

}

/*
jobject jvm::NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	Class *cls = static_cast<Class *>(clazz) ;

	if(cls->blnInterface == true || cls->blnAbstract == true)
	{
		throw_exception("java/lang/InstantiationException") ;
		return NULL ;
	}

	LocalVariable *obj_val = theJVM->createLocalVariable() ;
	obj_val->value_type = value::REFERENCE ;

	object *obj = theJVM->createObject() ;

	obj->type = cls ;

	for(int i=0; i<cls->fields.size(); i++)
	{
		value *v = theJVM->createValue() ;
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
			object *obj1 = theJVM->createObject() ;
			obj1->blnNull = true ;
			v->ref_value = obj1 ;
		}

		obj->fields[cls->fields.at(i)->name] = v ;
	}

	obj_val->ref_value = obj ;

	//construct a new frame for this method and
	//invoke this method

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	frm->local_variables.push_back(obj_val) ;

	//standard code to extract variable parameters
	//(The C++ Programming Language by Stroustrup (3rd Ed) -- pp 155-156)
	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		frm->local_variables.push_back(val) ;
	}

	va_end(ap) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	while(thrd->java_stack.top() != frm)
	{
		theJVM->execute_instruction() ;
	}

	return obj ;

}
*/

jclass GetObjectClass(JNIEnv *env, jobject obj)
{
#if DEBUG_LEVEL >= 2
	cout << "Entering GetObjectClass()" << endl << flush ;
#endif	
	
	object *obj1 = static_cast<object *>(obj) ;
	
#if DEBUG_LEVEL >= 2
	cout << "GetObjectClass(): object is " << obj1 << endl << flush ;
	cout << "GetObjectClass(): object's type is" << obj1->type << endl << flush ;
	cout << "GetObjectClass(): object's types's name is " << obj1->type->name << endl << flush ;
	cout << "Exiting GetObjectClass()" << endl << flush ;
	cout << "Type id is " << typeid(obj1->type).name() << endl << flush ;
#endif	
	//code changed 9/8/2005
	//return obj1->type ;
	return (jclass)(theJVM->getClassObject(obj1->type)) ;
	//end of code changed 9/8/2005
}

jboolean IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz)
{
	object *obj1 = static_cast<object *>(obj) ;

	//A NULL object can be cast to any class
	if(obj1->blnNull == true)
		return JNI_TRUE ;

        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	if(theJVM->IsAssignmentCompatible(obj1->type, cls) == true)
		return JNI_TRUE ;
	else
		return JNI_FALSE ;

}

jboolean IsSameObject(JNIEnv *env, jobject ref1, jobject ref2)
{
	object *obj1 = static_cast<object *>(ref1) ;
	object *obj2 = static_cast<object *>(ref2) ;

	if(obj1->blnNull == true && obj2->blnNull == true)
		return JNI_TRUE ;
	else
	{
		if(obj1 == obj2)
			return JNI_TRUE ;
		else
			return JNI_FALSE ;
	}

}

jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object *)clazz ;
        Class *cls = obj->class_pointer ;
        //end of code changed 24/9/2005

	if(cls->blnInitialised == false)
		theJVM->InitialiseClass(cls) ;

	//TO DO: if InitialiseClass fails, we need
	//to trap it so that we can exit this method
	//(by returning NULL)

	for(int i=0; i<cls->fields.size(); i++)
	{
		if(cls->fields.at(i)->name == name &&
		   cls->fields.at(i)->get_descriptor() == sig)
			return (jfieldID)cls->fields.at(i) ;
	}

	theJVM->throw_exception("java/lang/NoSuchFieldError") ;

	return NULL ;
}

jobject GetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	//we are using only the field name to get the object (and
	//not the descriptor) -- because the 'fields' map of the
	//'object' class is keyed on the field name (check this -- 
	//we cannot have two fields with the same name,so why do we
	//need the name AND the descriptor to identify a field in GetFieldID?)

	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->ref_value ;

}

jboolean GetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;


	//we assume that 0 is false and 1 is true
	if(obj1->fields[fld->name]->bool_value	== true)
		return 1 ;
	else
		return 0 ;
}

jbyte GetByteField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->i_value ;
}

jchar GetCharField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->i_value ;
}

jshort GetShortField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->i_value ;
}

jint GetIntField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->i_value ;
}

jlong GetLongField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->long_value ;
}

jfloat GetFloatField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->float_value ;
}

jdouble GetDoubleField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	return obj1->fields[fld->name]->double_value ;
}

void SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID, jobject value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::REFERENCE ;
	val->ref_value = static_cast<object *>(value1) ;
	
	obj1->fields[fld->name] = val ;	
}

void SetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::BOOLEAN ;

	//we assume that 0 is false and 1 is true

	if(value1 == 0)
		val->bool_value =  false ;
	else
		val->bool_value =  true ;
	
	obj1->fields[fld->name] = val ;
}

void SetByteField(JNIEnv *env, jobject obj, jfieldID fieldID, jbyte value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::_BYTE ;
	val->i_value = value1 ;
	
	obj1->fields[fld->name] = val ;	
}

void SetCharField(JNIEnv *env, jobject obj, jfieldID fieldID, jchar value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::CHAR ;
	val->i_value = value1 ;
	
	obj1->fields[fld->name] = val ;	
}

void SetShortField(JNIEnv *env, jobject obj, jfieldID fieldID, jshort value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::SHORT ;
	val->i_value = value1 ;
	
	obj1->fields[fld->name] = val ;	
}

void SetIntField(JNIEnv *env, jobject obj, jfieldID fieldID, jint value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::INT ;
	val->i_value = value1 ;
	
	obj1->fields[fld->name] = val ;	
}

void SetLongField(JNIEnv *env, jobject obj, jfieldID fieldID, jlong value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::LONG ;
	val->long_value = value1 ;
	
	obj1->fields[fld->name] = val ;	
}

void SetFloatField(JNIEnv *env, jobject obj, jfieldID fieldID, jfloat value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;


	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::FLOAT ;
	val->float_value = value1 ;
	
	obj1->fields[fld->name] = val ;	
}

void SetDoubleField(JNIEnv *env, jobject obj, jfieldID fieldID, jdouble value1)
{
	object *obj1 = static_cast<object *>(obj) ;
	field *fld = (field *)fieldID ;

	delete obj1->fields[fld->name] ;

	value *val = theJVM->createValue() ;
	val->value_type = value::DOUBLE ;
	val->double_value = value1 ;
	
	obj1->fields[fld->name] = val ;	
}

jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object *)(clazz) ;
        Class *cls = obj->class_pointer ;
        //end of code changed 24/9/2005
	
//code added 1/8/2005
#if DEBUG_LEVEL >= 2
	cout << "GetMethodID(): " << typeid(cls).name() << endl << flush ;
	cout << "GetMethodID(): " << cls->name << endl << flush ;
#endif	
//end of code added 1/8/2005	

	if(cls->blnInitialised == false)
		theJVM->InitialiseClass(cls) ;

	//TO DO: handle errors from InitialiseClass

	//check if the method exists in the class
	
	//code changed 17/11/2004
	//typedef map<method_key, method *>::const_iterator CI ;
	typedef map<string, method *>::const_iterator CI ;	
	//end of code changed 17/11/2004
	
	CI p ;

	//code changed 17/11/2004
	//p = cls->methods.find(method_key(name, sig)) ;
	char name1[100] ;
	char sig1[100] ;
	char glue[] = " %% " ;
	
	memset(name1, '\0', 100) ;
	memset(sig1, '\0', 100) ;	
	
	strcpy(name1, name) ;
	strcpy(sig1, sig) ;
	
	p = cls->methods.find(strcat(strcat(name1,glue),sig1)) ;	
	//end of code changde 17/11/2004

	if(p != cls->methods.end())
	{
		return((jmethodID)cls->methods[p->first]) ;
	}
	else
	{
		//check if the method is defined in the superclasses
		Class *t = cls ;

		while(t->super_class_name != "")
		{
			//code changed 17/11/2004
			//p = t->super_class->methods.find(method_key(name, sig)) ;
			
			char name1[100] ;
			char sig1[100] ;
			char glue[] = " %% " ;
			
			memset(name1, '\0', 100) ;
			memset(sig1, '\0', 100) ;	
			
			strcpy(name1, name) ;
			strcpy(sig1, sig) ;			
			
			p = t->super_class->methods.find(strcat(strcat(name1,glue),sig)) ;			
			//end of code changed 17/11/2004

			//code changed 14/8/2005
			//if(p != t->methods.end())
			//	return((jmethodID)t->methods[p->first]) ;
			if(p != t->super_class->methods.end())
				return((jmethodID)t->super_class->methods[p->first]) ;
			//end of code added 14/8/2005
			else
				t = t->super_class ;
		}
	}

	theJVM->throw_exception("java/lang/NoSuchMethodError") ;

	return NULL ;	
}

void CallVoidMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

}

void CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;
        
	va_end(ap) ;

	CallVoidMethodInternal(env, obj, methodID, &v) ;

}

void CallVoidMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	CallVoidMethodInternal(env, obj, methodID, &v) ;
}

/*
void jvm::CallVoidMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	CallVoidMethodInternal(env, obj, methodID, &v) ;

}
*/

jobject CallObjectMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//since v already contains obj, it is wrong
	//to once again push obj (in the form of
	//the first local variable)
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005
	

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//see comment above
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	//just testing
	//cout << "CallObjectMethodInternal(): " << v->size() << endl << flush ;
	//for(int i=0; i<v->size(); i++)
	//	cout << v->at(i)->ref_value << " " ;
	//cout << endl << flush ;
	//end of just testing 
	
	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	
	//code changed 10/2/2005
	//return frm->operand_stack.top()->ref_value ;
	return thrd->java_stack.top()->operand_stack.top()->ref_value ;
	//end of code changed 10/2/2005
}

jobject CallObjectMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallObjectMethodInternal(env, obj, methodID, &v) ;

}

jobject CallObjectMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	//code added 23/11/2004
	value *v1 = theJVM->createValue() ;
	v1->value_type = value::REFERENCE ;
	
	object *obj1 = static_cast<object *>(obj) ; 
	
#if DEBUG_LEVEL >= 3
	assert(obj1->type != NULL) ;
#endif		
	
	v1->ref_value = obj1 ;
	
	v.push_back(theJVM->create_local_variable(v1)) ;
	//end of code added 23/11/2004

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallObjectMethodInternal(env, obj, methodID, &v) ;
}

/*
jobject jvm::CallObjectMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallObjectMethodInternal(env, obj, methodID, &v) ;

}
*/

jboolean CallBooleanMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//if(frm->operand_stack.top()->bool_value == true)
	if(thrd->java_stack.top()->operand_stack.top()->bool_value == true)
	//end of code changed 12/2/2005
		return 1 ;
	else
		return 0 ;
}

jboolean CallBooleanMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallBooleanMethodInternal(env, obj, methodID, &v) ;

}

jboolean CallBooleanMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallBooleanMethodInternal(env, obj, methodID, &v) ;
}

/*
jboolean jvm::CallBooleanMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallBooleanMethodInternal(env, obj, methodID, &v) ;

}
*/

jbyte CallByteMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;


	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;
	//end of code changed 12/2/2005
}

jbyte CallByteMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallByteMethodInternal(env, obj, methodID, &v) ;

}

jbyte CallByteMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;


	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallByteMethodInternal(env, obj, methodID, &v) ;
}

/*
jbyte jvm::CallByteMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallByteMethodInternal(env, obj, methodID, &v) ;

}
*/

jchar CallCharMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005
	

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;
	//end of code changed 12/2/2005
}

jchar CallCharMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallCharMethodInternal(env, obj, methodID, &v) ;

}

jchar CallCharMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallCharMethodInternal(env, obj, methodID, &v) ;
}

/*
jchar jvm::CallCharMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallCharMethodInternal(env, obj, methodID, &v) ;

}
*/

jshort CallShortMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;
	//end of code changed 12/2/2005
}

jshort CallShortMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallShortMethodInternal(env, obj, methodID, &v) ;

}

jshort CallShortMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallShortMethodInternal(env, obj, methodID, &v) ;
}

/*
jshort jvm::CallShortMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallShortMethodInternal(env, obj, methodID, &v) ;

}
*/

jint CallIntMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;
	//end of code changed 12/2/2005
}

jint CallIntMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallIntMethodInternal(env, obj, methodID, &v) ;

}

jint CallIntMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallIntMethodInternal(env, obj, methodID, &v) ;
}

/*
jint jvm::CallIntMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallIntMethodInternal(env, obj, methodID, &v) ;

}
*/

jlong CallLongMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->long_value ;
	return thrd->java_stack.top()->operand_stack.top()->long_value ;
	//end of code changed 12/2/2005
}

jlong CallLongMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallLongMethodInternal(env, obj, methodID, &v) ;

}

jlong CallLongMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallLongMethodInternal(env, obj, methodID, &v) ;
}

/*
jlong jvm::CallLongMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallLongMethodInternal(env, obj, methodID, &v) ;

}
*/

jfloat CallFloatMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->float_value ;
	return thrd->java_stack.top()->operand_stack.top()->float_value ;
	//end of code changed 12/2/2005
}

jfloat CallFloatMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;


	return CallFloatMethodInternal(env, obj, methodID, &v) ;

}

jfloat CallFloatMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallFloatMethodInternal(env, obj, methodID, &v) ;
}

/*
jfloat jvm::CallFloatMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallFloatMethodInternal(env, obj, methodID, &v) ;

}
*/

jdouble CallDoubleMethodInternal(JNIEnv *env, jobject obj, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(obj1->type->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005
	
	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->double_value ;
	return thrd->java_stack.top()->operand_stack.top()->double_value ;
	//end of code changed 12/2/2005
}

jdouble CallDoubleMethod(JNIEnv *env, jobject obj, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallDoubleMethodInternal(env, obj, methodID, &v) ;

}

jdouble CallDoubleMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallDoubleMethodInternal(env, obj, methodID, &v) ;
}

/*
jdouble jvm::CallDoubleMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallDoubleMethodInternal(env, obj, methodID, &v) ;

}
*/

void CallNonvirtualVoidMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;


	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

}

void CallNonvirtualVoidMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	CallNonvirtualVoidMethodInternal(env, obj, clazz, methodID, &v) ;

}

void CallNonvirtualVoidMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	CallNonvirtualVoidMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
void jvm::CallNonvirtualVoidMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	CallNonvirtualVoidMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jobject CallNonvirtualObjectMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;

        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;


	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->ref_value ;
	return thrd->java_stack.top()->operand_stack.top()->ref_value ;
	//end of code changed 12/2/2005
}

jobject CallNonvirtualObjectMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualObjectMethodInternal(env, obj, clazz, methodID, &v) ;

}

jobject CallNonvirtualObjectMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualObjectMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jobject jvm::CallNonvirtualObjectMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualObjectMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jboolean CallNonvirtualBooleanMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//if(frm->operand_stack.top()->bool_value == true)
	if(thrd->java_stack.top()->operand_stack.top()->bool_value == true)
	//end of code changed 12/2/2005
		return 1 ;
	else
		return 0 ;
}

jboolean CallNonvirtualBooleanMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualBooleanMethodInternal(env, obj, clazz, methodID, &v) ;

}

jboolean CallNonvirtualBooleanMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualBooleanMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jboolean jvm::CallNonvirtualBooleanMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualBooleanMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jbyte CallNonvirtualByteMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;
	//end of code changed 12/2/2005
}

jbyte CallNonvirtualByteMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualByteMethodInternal(env, obj, clazz, methodID, &v) ;

}

jbyte CallNonvirtualByteMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualByteMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jbyte jvm::CallNonvirtualByteMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualByteMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jchar CallNonvirtualCharMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;	
	//end of code changed 12/2/2005
}

jchar CallNonvirtualCharMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualCharMethodInternal(env, obj, clazz, methodID, &v) ;

}

jchar CallNonvirtualCharMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualCharMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jchar jvm::CallNonvirtualCharMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualCharMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jshort CallNonvirtualShortMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;	
	//end of code changed 12/2/2005
}

jshort CallNonvirtualShortMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualShortMethodInternal(env, obj, clazz, methodID, &v) ;

}

jshort CallNonvirtualShortMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualShortMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jshort jvm::CallNonvirtualShortMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualShortMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jint CallNonvirtualIntMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;	
	//end of code changed 12/2/2005
}

jint CallNonvirtualIntMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualIntMethodInternal(env, obj, clazz, methodID, &v) ;

}

jint CallNonvirtualIntMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualIntMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jint jvm::CallNonvirtualIntMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualIntMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jlong CallNonvirtualLongMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->long_value ;
	return thrd->java_stack.top()->operand_stack.top()->long_value ;	
	//end of code changed 12/2/2005
}

jlong CallNonvirtualLongMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;


	return CallNonvirtualLongMethodInternal(env, obj, clazz, methodID, &v) ;

}

jlong CallNonvirtualLongMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualLongMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jlong jvm::CallNonvirtualLongMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualLongMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jfloat CallNonvirtualFloatMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->float_value ;
	return thrd->java_stack.top()->operand_stack.top()->float_value ;	
	//end of code changed 12/2/2005
}

jfloat CallNonvirtualFloatMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualFloatMethodInternal(env, obj, clazz, methodID, &v) ;

}

jfloat CallNonvirtualFloatMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualFloatMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jfloat jvm::CallNonvirtualFloatMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualFloatMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jdouble CallNonvirtualDoubleMethodInternal(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
	object *obj1 = static_cast<object *>(obj) ;
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->double_value ;
	return thrd->java_stack.top()->operand_stack.top()->double_value ;	
	//end of code changed 12/2/2005
}

jdouble CallNonvirtualDoubleMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallNonvirtualDoubleMethodInternal(env, obj, clazz, methodID, &v) ;

}

jdouble CallNonvirtualDoubleMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallNonvirtualDoubleMethodInternal(env, obj, clazz, methodID, &v) ;
}

/*
jdouble jvm::CallNonvirtualDoubleMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list ap)

{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallNonvirtualDoubleMethodInternal(env, obj, clazz, methodID, &v) ;

}
*/

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
	//since we don't differentiate between static fields
	//and instance fields in the 'Class' class
	//(both are stored in the vector 'fields'),
	//we can simply reuse GetFieldID
        
        return GetFieldID(env, clazz, name, sig) ;
}

jobject GetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->ref_value ;
}

jboolean GetStaticBooleanField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	//we assume that 0 is false and 1 is true
	if(cls->static_fields[fld->name]->bool_value == true)
		return 1 ;
	else
		return 0 ;

}

jbyte GetStaticByteField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->i_value ;
}

jchar GetStaticCharField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->i_value ;
}

jshort GetStaticShortField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->i_value ;
}

jint GetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->i_value ;
}

jlong GetStaticLongField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->long_value ;
}

jfloat GetStaticFloatField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->float_value ;
}

jdouble GetStaticDoubleField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	return cls->static_fields[fld->name]->double_value ;
}

void SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class *cls = obj->class_pointer ;
        //end of code changed 24/9/2005

	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::REFERENCE ;
	val->ref_value = static_cast<object *>(value1) ;
	
	cls->static_fields[fld->name] = val ;	
}

void SetStaticBooleanField(JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::BOOLEAN ;

	//we assume that 0 is false and 1 is true

	if(value1 == 0)
		val->bool_value =  false ;
	else
		val->bool_value =  true ;
	
	cls->static_fields[fld->name] = val ;
}

void SetStaticByteField(JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::_BYTE ;
	val->i_value = value1 ;
	
	cls->static_fields[fld->name] = val ;	
}

void SetStaticCharField(JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::CHAR ;
	val->i_value = value1 ;
	
	cls->static_fields[fld->name] = val ;	
}

void SetStaticShortField(JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::SHORT ;
	val->i_value = value1 ;
	
	cls->static_fields[fld->name] = val ;	
}

void SetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID, jint value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::INT ;
	val->i_value = value1 ;
	
	cls->static_fields[fld->name] = val ;	
}

void SetStaticLongField(JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::LONG ;
	val->long_value = value1 ;
	
	cls->static_fields[fld->name] = val ;	
}

void SetStaticFloatField(JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::FLOAT ;
	val->float_value = value1 ;

	
	cls->static_fields[fld->name] = val ;	
}

void SetStaticDoubleField(JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value1)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	field *fld = (field *)fieldID ;

	delete cls->static_fields[fld->name] ;

	static_value *val = new static_value() ;
	val->type = cls ;
	val->value_type = value::DOUBLE ;
	val->double_value = value1 ;
	
	cls->static_fields[fld->name] = val ;	
}

jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{

        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005

	if(cls->blnInitialised == false)
		theJVM->InitialiseClass(cls) ;

	//TO DO: handle errors from InitialiseClass

	//check if the method exists in the class

	//code changed 17/11/2004
	//typedef map<method_key, method *>::const_iterator CI ;
	typedef map<string, method *>::const_iterator CI ;	
	//end of code changed 17/11/2004
	
	CI p ;

	//code changed 17/11/2004
	//p = cls->methods.find(method_key(name, sig)) ;

	char name1[100] ;
	char sig1[100] ;
	char glue[] = " %% " ;
	
	memset(name1, '\0', 100) ;
	memset(sig1, '\0', 100) ;	
	
	strcpy(name1, name) ;
	strcpy(sig1, sig) ;

	p = cls->methods.find(strcat(strcat(name1,glue),sig)) ;	
	//end of code changed 17/11/2004

	if(p != cls->methods.end())
		return((jmethodID)cls->methods[p->first]) ;

	theJVM->throw_exception("java/lang/NoSuchMethodError") ;

	return NULL ;	
}

void CallStaticVoidMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj = (object*)clazz ;
        Class* cls = obj->class_pointer ;
        //end of code changed 24/9/2005
        
	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

}

void CallStaticVoidMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	CallStaticVoidMethodInternal(env, clazz, methodID, &v) ;

}

void CallStaticVoidMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	CallStaticVoidMethodInternal(env, clazz, methodID, &v) ;
}

/*
void jvm::CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	CallStaticVoidMethodInternal(env, clazz, methodID, &v) ;

}
*/

jobject CallStaticObjectMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005
        
	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->ref_value ;
	return thrd->java_stack.top()->operand_stack.top()->ref_value ;	
	//end of code changed 12/2/2005
}

jobject CallStaticObjectMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallStaticObjectMethodInternal(env, clazz, methodID, &v) ;

}

jobject CallStaticObjectMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}


	return CallStaticObjectMethodInternal(env, clazz, methodID, &v) ;
}

/*
jobject jvm::CallStaticObjectMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticObjectMethodInternal(env, clazz, methodID, &v) ;

}
*/

jboolean CallStaticBooleanMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005
        
	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//if(frm->operand_stack.top()->bool_value == true)
	if(thrd->java_stack.top()->operand_stack.top()->bool_value == true)	
	//end of code changed 12/2/2005
		return 1 ;
	else
		return 0 ;
}

jboolean CallStaticBooleanMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

        //code changed 6/9/2005
	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
	method *m = (method *)methodID ;
	for(int i=0; i<m->parameters->size(); i++)
	{
		LocalVariable *val = theJVM->createLocalVariable() ;
		if(m->parameters->at(i) == "B")
		{
			val->value_type = value::_BYTE ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "C")
		{
			val->value_type = value::CHAR ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "D")
		{
			val->value_type = value::DOUBLE ;
			val->double_value = va_arg(ap, double) ;
			LocalVariable *dummy = theJVM->createLocalVariable() ;
			dummy->valid_flag = false;
			v.push_back(dummy) ;
		}
		else if(m->parameters->at(i) == "F")
		{
			val->value_type = value::FLOAT ;
			val->float_value = va_arg(ap, double) ;
		}
		else if(m->parameters->at(i) == "I")
		{
			val->value_type = value::INT ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "J")
		{
			val->value_type = value::LONG ;
			val->long_value = va_arg(ap, long) ;
			LocalVariable *dummy = theJVM->createLocalVariable() ;
			dummy->valid_flag = false;
			v.push_back(dummy) ;
		}
		else if(m->parameters->at(i)[0] == 'L')
		{
			val->value_type = value::REFERENCE ;
			val->ref_value = (object *)va_arg(ap, jobject) ;
		}
		else if(m->parameters->at(i) == "S")
		{
			val->value_type = value::SHORT ;
			val->i_value = va_arg(ap, int) ;
		}
		else if(m->parameters->at(i) == "Z")
		{
			val->value_type = value::BOOLEAN ;
			int temp = va_arg(ap, int) ;
			val->i_value = (temp = 0) ? false : true ;
		}		
		else if(m->parameters->at(i)[0] == '[')
		{
			val->value_type = value::REFERENCE ;
			val->ref_value = (object *)va_arg(ap, jobject) ;
		}
		v.push_back(val) ;
	}
        //end of code changed 6/9/2005
        
	va_end(ap) ;

	return CallStaticBooleanMethodInternal(env, clazz, methodID, &v) ;

}

jboolean CallStaticBooleanMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticBooleanMethodInternal(env, clazz, methodID, &v) ;
}

/*
jboolean jvm::CallStaticBooleanMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticBooleanMethodInternal(env, clazz, methodID, &v) ;

}
*/

jbyte CallStaticByteMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005
        
	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;


	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;	
	//end of code changed 12/2/2005
}

jbyte CallStaticByteMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallStaticByteMethodInternal(env, clazz, methodID, &v) ;

}

jbyte CallStaticByteMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticByteMethodInternal(env, clazz, methodID, &v) ;
}

/*
jbyte jvm::CallStaticByteMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;


	return CallStaticByteMethodInternal(env, clazz, methodID, &v) ;

}
*/

jchar CallStaticCharMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005 
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;	
	//end of code changed 12/2/2005
}

jchar CallStaticCharMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallStaticCharMethodInternal(env, clazz, methodID, &v) ;

}

jchar CallStaticCharMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticCharMethodInternal(env, clazz, methodID, &v) ;
}

/*
jchar jvm::CallStaticCharMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticCharMethodInternal(env, clazz, methodID, &v) ;

}
*/

jshort CallStaticShortMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;
	//end of code changed 12/2/2005
}

jshort CallStaticShortMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallStaticShortMethodInternal(env, clazz, methodID, &v) ;

}

jshort CallStaticShortMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticShortMethodInternal(env, clazz, methodID, &v) ;
}

/*
jshort jvm::CallStaticShortMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticShortMethodInternal(env, clazz, methodID, &v) ;

}
*/

jint CallStaticIntMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->i_value ;
	return thrd->java_stack.top()->operand_stack.top()->i_value ;
	//end of code changed 12/2/2005
}

jint CallStaticIntMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallStaticIntMethodInternal(env, clazz, methodID, &v) ;

}

jint CallStaticIntMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)

	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticIntMethodInternal(env, clazz, methodID, &v) ;
}

/*
jint CallStaticIntMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticIntMethodInternal(env, clazz, methodID, &v) ;

}
*/

jlong CallStaticLongMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->long_value ;
	return thrd->java_stack.top()->operand_stack.top()->long_value ;	
	//end of code changed 12/2/2005
}

jlong CallStaticLongMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;


	return CallStaticLongMethodInternal(env, clazz, methodID, &v) ;

}

jlong CallStaticLongMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticLongMethodInternal(env, clazz, methodID, &v) ;
}

/*
jlong jvm::CallStaticLongMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticLongMethodInternal(env, clazz, methodID, &v) ;

}
*/

jfloat CallStaticFloatMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	object *obj1 = cls->class_object ;

	//code changed 23/5/2005
	//see comment in CallObjectMethodInternal()
	/*
	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;

	obj_val->ref_value = obj1 ;
	*/
	//end of code changed 23/5/2005

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	//code changed 23/5/2005
	//frm->local_variables.push_back(obj_val) ;
	//end of code changed 23/5/2005

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//code changed 12/2/2005
	//return frm->operand_stack.top()->float_value ;
	return thrd->java_stack.top()->operand_stack.top()->float_value ;	
	//end of code changed 12/2/2005
}

jfloat CallStaticFloatMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallStaticFloatMethodInternal(env, clazz, methodID, &v) ;

}

jfloat CallStaticFloatMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticFloatMethodInternal(env, clazz, methodID, &v) ;
}

/*
jfloat jvm::CallStaticFloatMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticFloatMethodInternal(env, clazz, methodID, &v) ;

}
*/

jdouble CallStaticDoubleMethodInternal(JNIEnv *env, jclass clazz, jmethodID methodID, vector<LocalVariable *> *v)
{
        //code changed 24/9/2005
	//Class *cls = static_cast<Class *>(clazz) ;
        object* obj2 = (object*)clazz ;
        Class* cls = obj2->class_pointer ;
        //end of code changed 24/9/2005

	object *obj1 = cls->class_object ;

	LocalVariable *obj_val = theJVM->createLocalVariable() ;

	obj_val->value_type = value::REFERENCE ;


	obj_val->ref_value = obj1 ;

	frame *frm = new frame() ;
	frm->rcp = &(cls->rcp) ;

	frm->local_variables.push_back(obj_val) ;

	for(int i=0; i<v->size(); i++)
		frm->local_variables.push_back(v->at(i)) ;

	frm->curr_method = (method *)methodID ;

	thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;

	//code added 21/11/2004
	frame *prev_frame = thrd->java_stack.top() ;
	//end of code added 21/11/2004

	thrd->java_stack.push(frm) ;

	//don't think this is really required, as the previous
	//method is a native method
	frm->prev_pc_value = thrd->pc ;

	//this is applicable only if the
	//called method is a non-native method
	thrd->pc = 0 ;

	//we run the JVM till we return back to the newly created frame
	//code changed 21/11/2004
	//while(thrd->java_stack.top() != frm)
	while(thrd->java_stack.top() != prev_frame)	
	//end of code changed 21/11/2004
	{
		theJVM->execute_instruction() ;
	}

	//this would have been placed on the frame
	//by the called method
	//end of code changed 12/2/2005
	//return frm->operand_stack.top()->double_value ;
	return thrd->java_stack.top()->operand_stack.top()->double_value ;	
	//end of code changed 12/2/2005
}

jdouble CallStaticDoubleMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{

	vector<LocalVariable *> v ;

	va_list ap ;

	va_start(ap, methodID) ;

	//for(;;)
	//{
	//	//check where we need to cast from a jvalue
	//	LocalVariable *val = va_arg(ap, LocalVariable *) ;
	//	if(val == 0)
	//		break ;
	//	v.push_back(val) ;
	//}
        theJVM->populateLocalVariablesForFrame(&methodID, &ap, v) ;

	va_end(ap) ;

	return CallStaticDoubleMethodInternal(env, clazz, methodID, &v) ;

}

jdouble CallStaticDoubleMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, jvalue *args)
{
	vector<LocalVariable *> v ;

	method *mthd = (method *)methodID ;

	int size = mthd->parameters->size() ;

	for(int i=0; i<size; i++)
	{
		//code changed 29/5/2005
		//v.push_back((LocalVariable *)(args+i)) ;
		v.push_back(theJVM->create_local_variable(args+i, mthd->parameters->at(i))) ;
		//end of code changed 29/5/2005
	}

	return CallStaticDoubleMethodInternal(env, clazz, methodID, &v) ;
}

/*
jdouble jvm::CallStaticDoubleMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list ap)
{
	vector<LocalVariable *> v ;

	va_start(ap, methodID) ;

	for(;;)
	{
		//check where we need to cast from a jvalue
		LocalVariable *val = va_arg(ap, LocalVariable *) ;
		if(val == 0)
			break ;
		v.push_back(val) ;
	}

	va_end(ap) ;

	return CallStaticDoubleMethodInternal(env, clazz, methodID, &v) ;

}
*/

jstring NewString(JNIEnv *env, const jchar *unicodeChars, jsize len)
{

	//hack; unsigned short (jchar) is
	//being cast to char. this will
	//result in trucations
	char *c = new char[len+1] ;

	for(int i=0; i<len; i++)
		c[i] = (char)unicodeChars[i] ;

	c[len] = '\0' ;

	string s(c) ;

	typedef map<string, object *>::const_iterator CI ;
	CI p ;

	p = theJVM->strings.find(s) ;

	if(p != theJVM->strings.end())
	{
		delete [] c ;
		return((jstring)theJVM->strings[p->first]) ;
	}
	else
	{
		//code changed 30/7/2005
		/*
		object *obj = theJVM->createObject() ;

		//code added 23/11/2004
		obj->blnStringObject = true ;
		//end of code added 23/11/2004

		for(int i=0; i<s.length(); i++)
			obj->data.push_back(s.at(i)) ;
		*/
		object *obj = theJVM->create_string_object(s) ;
		//end of code changed 30/7/2005
		

		//code added 30/7/2005
		thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;
	
		frame *prev_frame = thrd->java_stack.top() ;

		theJVM->setup_string_init(obj, s) ;		
		
		thrd->pc = 0 ;
	
		while(thrd->java_stack.top() != prev_frame)	
			theJVM->execute_instruction() ;
		//end of code added 30/7/2005
		
		theJVM->strings[s] = obj ;

		delete [] c ;
		return (jstring)obj ;
	}
}

jsize GetStringLength(JNIEnv *env, jstring string1)
{
	object *obj = (object *)(string1) ;

	return obj->data.size() ;
}

const jchar * GetStringChars(JNIEnv *env, jstring string1, jboolean *isCopy)
{
	object *obj = (object *)(string1) ;

	int len = obj->data.size() ;


	jchar *copy = new jchar[len] ;

	for(int i=0; i<len; i++)
		copy[i] = obj->data.at(i) ;

	*isCopy = JNI_TRUE ;

	return copy ;
}

void ReleaseStringChars(JNIEnv *env, jstring string1, const jchar *chars)
{
	delete [] chars ;
}

jstring NewStringUTF(JNIEnv *env, const char *bytes)
{

	int len = strlen(bytes) ;

	char *c = new char[len+1] ;

	for(int i=0; i<len; i++)
		c[i] = bytes[i] ;

	//code changed 17/2/2005
	//c[len] = NULL ;
	c[len] = (char)NULL ;
	//end of code changed 17/2/2005

	string s(c) ;

	typedef map<string, object *>::const_iterator CI ;
	CI p ;

	p = theJVM->strings.find(s) ;


	if(p != theJVM->strings.end())
	{
		delete [] c ;
		return((jstring)theJVM->strings[p->first]) ;
	}
	else
	{
		//code changed 30/7/2005
		/*
		object *obj = theJVM->createObject() ;

		//code added 23/11/2004
		obj->blnStringObject = true ;
		//end of code added 23/11/2004

		for(int i=0; i<s.length(); i++)
			obj->data.push_back(s.at(i)) ;
		*/
		object *obj = theJVM->create_string_object(s) ;
		//end of code changed 30/7/2005


		//code added 30/7/2005
		thread *thrd = theJVM->threads.at(theJVM->curr_thread) ;
	
		frame *prev_frame = thrd->java_stack.top() ;

		theJVM->setup_string_init(obj, s) ;		
		
		thrd->pc = 0 ;
	
		while(thrd->java_stack.top() != prev_frame)	
			theJVM->execute_instruction() ;
		//end of code added 30/7/2005
		
		theJVM->strings[s] = obj ;

		delete [] c ;
		return (jstring)obj ;
	}
}

jsize GetStringUTFLength(JNIEnv *env, jstring string1)
{
	object *obj = (object *)(string1) ;

	return obj->data.size() ;
}

const char *GetStringUTFChars(JNIEnv *env, jstring string1, jboolean *isCopy)
{

	object *obj = (object *)(string1) ;
        
//cout << obj << " " << obj->type->name << endl << flush ;

#if DEBUG_LEVEL >= 3
	assert(obj->blnStringObject == true) ;
#endif
	
	//code changed 27/3/2005
	//temp hack; correct this when we confirm
	//out whether 'data' in the 'object' class
	//is going to be used to store string data
	/*
	int len = obj->data.size() ;

	char *copy = new char[len] ;

	for(int i=0; i<len; i++)
		copy[i] = obj->data.at(i) ;

        copy[len] = '\0' ;

        if(isCopy != NULL)
    	        *isCopy = JNI_TRUE ;

	return copy ;
	*/
	//obj->display();
        
#if DEBUG_LEVEL >= 2
	cout << "GetStringUTFChars(): " << obj->string_val.c_str() << endl << flush ;
#endif

	return obj->string_val.c_str() ;
	//end of code changed 27/3/2005

}

void ReleaseStringUTFChars(JNIEnv *env, jstring string1, const char *utf)
{
  //TO DO: find out the right way to release the string
  //delete [] utf ;
}

jsize GetArrayLength(JNIEnv *env, jarray array)
{
    object *obj = (object *)(array) ;

    return obj->items.size() ;    
}

jobjectArray NewObjectArray(JNIEnv *env, jsize length, jclass elementClass, jobject initialElement)
{
    //code changed 24/9/2005
    //Class *cls = static_cast<Class *>(elementClass) ;
    object* obj2 = (object*)elementClass ;
    Class* cls = obj2->class_pointer ;
    //end of code changed 24/9/2005
        

    object *obj = static_cast<object *>(initialElement) ;

    object *newobj = theJVM->createObject() ;

    newobj->blnArrayObject = true ;
    newobj->array_component_type = cls ;

    //TO DO: should we make copies of initialElement,
    //or should the same object be stored
    //in all the positions?

    for(int i=0; i<length; i++)
	//code changed 12/8/2005
        //newobj->items.push_back(theJVM->clone_object(obj)) ;
        newobj->items.push_back(obj) ;
        //end of code changed 12/8/2005

    return (jobjectArray)newobj ;
}

jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index)
{
    object *obj = (object *)array ;

    if(index < 0 || index >= obj->items.size())
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return NULL ;
    }

    return obj->items.at(index) ;
}

void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject value)
{
    object *array_obj = (object *)array ;
    object *val = (object *)value ;

    if(index < 0 || index >= array_obj->items.size())
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    //code added 12/8/2005
    if(array_obj->array_component_type != val->type &&
       !(array_obj->array_component_type->name == "java/lang/reflect/Constructor" &&
         val->type->name == "java/lang/reflect/Method")
      )
    {
    //end of code added 12/8/2005
	    
	    if(theJVM->IsSuperClass(array_obj->array_component_type,val->type) == false)
	    {
		cout << array_obj->array_component_type->name << endl << flush ;
		cout << val->type->name << endl << flush ;
		theJVM->throw_exception("java/lang/ArrayStoreException") ;
		return ;
	    }
    //code added 12/8/2005
    }
    //end of code added 12/8/2005
  
    //code changed 12/8/2005    
    //delete array_obj->items.at(index) ;
    //end of code changed 12/8/2005
    
    //code changed 12/8/2005
    //array_obj->items.insert(array_obj->items.begin()+index, val) ;
    array_obj->items.at(index) =  val ;
    //end of code changed 12/8/2005

    return ;
}

jbooleanArray NewBooleanArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::BOOLEAN ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::BOOLEAN ;
        pv->bool_value = false ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jbooleanArray)array_obj ;
}


jbyteArray NewByteArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::_BYTE ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::_BYTE ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jbyteArray)array_obj ;
}

jcharArray NewCharArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::CHAR ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::CHAR ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jcharArray)array_obj ;
}

jshortArray NewShortArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::SHORT ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::SHORT ;
        pv->bool_value = false ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jshortArray)array_obj ;
}

jintArray NewIntArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::INT ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::INT ;
        pv->bool_value = false ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jintArray)array_obj ;
}

jlongArray NewLongArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::LONG ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::LONG ;
        pv->bool_value = false ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jlongArray)array_obj ;
}

jfloatArray NewFloatArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::FLOAT ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::FLOAT ;
        pv->bool_value = false ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jfloatArray)array_obj ;
}

jdoubleArray NewDoubleArray(JNIEnv *env, jsize length)
{
    object *array_obj = theJVM->createObject() ;

    array_obj->blnArrayObject = true ;

    array_obj->blnPrimitiveArrayObject = true ;

    array_obj->primitive_type = primitive_value::DOUBLE ;

    for(int i=0; i<length; i++)
    {
        primitive_value *pv = theJVM->createPrimitiveValue() ;
        pv->value_type = primitive_value::DOUBLE ;
        pv->bool_value = false ;
        array_obj->primitives.push_back(pv) ;
    }

    return (jdoubleArray)array_obj ;
}

jboolean *GetBooleanArrayElements(JNIEnv *env, jbooleanArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the
    //array elements
    *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jboolean *elems = new jboolean[array_size] ;

    for(int i=0; i<array_size; i++)
    {
        if(array_obj->primitives.at(i)->bool_value == true)
            elems[i] = JNI_TRUE ;
        else
            elems[i] = JNI_FALSE ;
    }

    return elems ;
}

jbyte *GetByteArrayElements(JNIEnv *env, jbyteArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the
    //array elements
    if(isCopy != NULL)
        *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jbyte *elems = new jbyte[array_size] ;

    for(int i=0; i<array_size; i++)
        elems[i] = array_obj->primitives.at(i)->i_value ;

    return elems ;
}

jchar *GetCharArrayElements(JNIEnv *env, jcharArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the
    //array elements
    *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jchar *elems = new jchar[array_size] ;

    for(int i=0; i<array_size; i++)
        elems[i] = array_obj->primitives.at(i)->i_value ;
        
    return elems ;
}

jshort *GetShortArrayElements(JNIEnv *env, jshortArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the
    //array elements
    *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jshort *elems = new jshort[array_size] ;

    for(int i=0; i<array_size; i++)
        elems[i] = array_obj->primitives.at(i)->i_value ;

    return elems ;
}

jint *GetIntArrayElements(JNIEnv *env, jintArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the
    //array elements
    *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jint *elems = new jint[array_size] ;

    for(int i=0; i<array_size; i++)
        elems[i] = array_obj->primitives.at(i)->i_value ;

    return elems ;
}

jlong *GetLongArrayElements(JNIEnv *env, jlongArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the
    //array elements
    *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jlong *elems = new jlong[array_size] ;

    for(int i=0; i<array_size; i++)
        elems[i] = array_obj->primitives.at(i)->long_value ;

    return elems ;
}

jfloat *GetFloatArrayElements(JNIEnv *env, jfloatArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the

    //array elements
    *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jfloat *elems = new jfloat[array_size] ;

    for(int i=0; i<array_size; i++)
        elems[i] = array_obj->primitives.at(i)->float_value ;

    return elems ;
}

jdouble *GetDoubleArrayElements(JNIEnv *env, jdoubleArray array, jboolean *isCopy)
{
    object *array_obj = (object *)array ;

    //we always make a copy of the
    //array elements
    *isCopy = JNI_TRUE ;

    int array_size = array_obj->primitives.size() ;

    jdouble *elems = new jdouble[array_size] ;

    for(int i=0; i<array_size; i++)
        elems[i] = array_obj->primitives.at(i)->double_value ;

    return elems ;
}

void ReleaseBooleanArrayElements(JNIEnv *env, jbooleanArray array, jboolean *elems, jint mode)
{
    object *array_obj = (object *)array ;

    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
        {
            if(elems[i] ==JNI_TRUE)
                array_obj->primitives.at(i)->bool_value = true ;
            else
                array_obj->primitives.at(i)->bool_value = false ;
        }

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void ReleaseByteArrayElements(JNIEnv *env, jbyteArray array, jbyte *elems, jint mode)
{
    object *array_obj = (object *)array ;

    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
            array_obj->primitives.at(i)->i_value = elems[i] ;

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void ReleaseCharArrayElements(JNIEnv *env, jcharArray array, jchar *elems, jint mode)
{
    object *array_obj = (object *)array ;

    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
            array_obj->primitives.at(i)->i_value = elems[i] ;

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void ReleaseShortArrayElements(JNIEnv *env, jshortArray array, jshort *elems, jint mode)
{
    object *array_obj = (object *)array ;

    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
            array_obj->primitives.at(i)->i_value = elems[i] ;

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void ReleaseIntArrayElements(JNIEnv *env, jintArray array, jint *elems, jint mode)
{
    object *array_obj = (object *)array ;

    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
            array_obj->primitives.at(i)->i_value = elems[i] ;

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void ReleaseLongArrayElements(JNIEnv *env, jlongArray array, jlong *elems, jint mode)
{
    object *array_obj = (object *)array ;

    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
            array_obj->primitives.at(i)->long_value = elems[i] ;

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void ReleaseFloatArrayElements(JNIEnv *env, jfloatArray array, jfloat *elems, jint mode)
{
    object *array_obj = (object *)array ;


    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
            array_obj->primitives.at(i)->float_value = elems[i] ;

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void ReleaseDoubleArrayElements(JNIEnv *env, jdoubleArray array, jdouble *elems, jint mode)
{
    object *array_obj = (object *)array ;

    if(mode == JNI_ABORT)
    {
        //free the buffer without copying back the
        //possible changes
        delete [] elems ;
        return ;
    }
    else
    {
        //copy the contents back
        for(int i=0; i<array_obj->primitives.size(); i++)
            array_obj->primitives.at(i)->double_value = elems[i] ;

        if(mode == 0)
        {
            //free the buffer
            delete [] elems ;
        }

        return ;

    }
}

void GetBooleanArrayRegion(JNIEnv *env, jbooleanArray array, jsize start, jsize len, jboolean *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
    {
        if(array_obj->primitives.at(i)->bool_value == true)
            buf[i-start] = JNI_TRUE ;
        else
            buf[i-start] = JNI_FALSE ;
    }

}

void GetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf)
{
    object *array_obj = (object *)array ;


    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        buf[i-start] = array_obj->primitives.at(i)->i_value ;
}

void GetCharArrayRegion(JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        buf[i-start] = array_obj->primitives.at(i)->i_value ;
}

void GetShortArrayRegion(JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        buf[i-start] = array_obj->primitives.at(i)->i_value ;
}

void GetIntArrayRegion(JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        buf[i-start] = array_obj->primitives.at(i)->i_value ;
}

void GetLongArrayRegion(JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        buf[i-start] = array_obj->primitives.at(i)->long_value ;
}

void GetFloatArrayRegion(JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }


    for(int i=start; i<len; i++)
        buf[i-start] = array_obj->primitives.at(i)->float_value ;
}

void GetDoubleArrayRegion(JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        buf[i-start] = array_obj->primitives.at(i)->double_value ;
}

void SetBooleanArrayRegion(JNIEnv *env, jbooleanArray array, jsize start, jsize len, jboolean *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
    {
        if(buf[i-start] == JNI_TRUE)
            array_obj->primitives.at(i)->bool_value = true  ;
        else
            array_obj->primitives.at(i)->bool_value = false  ;
    }

}

void SetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        array_obj->primitives.at(i)->i_value = buf[i-start];
}

void SetCharArrayRegion(JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        array_obj->primitives.at(i)->i_value = buf[i-start];
}

void SetShortArrayRegion(JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        array_obj->primitives.at(i)->i_value = buf[i-start];
}

void SetIntArrayRegion(JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        array_obj->primitives.at(i)->i_value = buf[i-start];
}

void SetLongArrayRegion(JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        array_obj->primitives.at(i)->long_value = buf[i-start];
}

void SetFloatArrayRegion(JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf)
{
    object *array_obj = (object *)array ;

    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        array_obj->primitives.at(i)->float_value = buf[i-start];
}

void SetDoubleArrayRegion(JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf)
{

    object *array_obj = (object *)array ;


    if(start < 0 && start >= array_obj->primitives.size() ||
       start+len >= array_obj->primitives.size() )
    {
        theJVM->throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
        return ;
    }

    for(int i=start; i<len; i++)
        array_obj->primitives.at(i)->double_value = buf[i-start];
}

jint RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint nMethods)
{

    //code changed 24/9/2005
    //Class *cls = static_cast<Class *>(clazz) ;
    object* obj2 = (object*)clazz ;
    Class* cls = obj2->class_pointer ;
    //end of code changed 24/9/2005

    //code changed 17/11/20004
    //typedef map<method_key, method *>::const_iterator CI ;
    typedef map<string, method *>::const_iterator CI ;    
    //end of code changed 17/11/2004

    CI p ;

    for(int i=0; i<nMethods; i++)
    {
        string s1((methods+i)->name) ;
        string s2((methods+i)->signature) ;

        //code changed 17/11/2004
    	//p = cls->methods.find(method_key(s1,s2)) ;
    	p = cls->methods.find(s1 + " %% " + s2) ;    	
    	//end of code changed 17/1//2004

	    if(p != cls->methods.end())
    	{
            theJVM->throw_exception("java/lang/NoSuchMethodError") ;
            return -1 ;
        }
        else
        {
            if(cls->methods[p->first]->blnNative == false)
    	    {
                theJVM->throw_exception("java/lang/NoSuchMethodError") ;
                return -1 ;
            }

	    //code added 31/1/04
	    cls->methods[p->first]->blnBound = true ;
	    //end of code added 31/1/04

            if(cls->methods[p->first]->return_descriptor.at(0) == 'V')
                cls->methods[p->first]->void_fnPtr = (void (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'B')
                cls->methods[p->first]->jbyte_fnPtr = (jbyte (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'C')
                cls->methods[p->first]->jchar_fnPtr = (jchar (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'D')
                cls->methods[p->first]->jdouble_fnPtr = (jdouble (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'F')
                cls->methods[p->first]->jfloat_fnPtr = (jfloat (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'I')
                cls->methods[p->first]->jint_fnPtr = (jint (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'J')
                cls->methods[p->first]->jlong_fnPtr = (jlong (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'S')
                cls->methods[p->first]->jshort_fnPtr = (jshort (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;


            if(cls->methods[p->first]->return_descriptor.at(0) == 'Z')
                cls->methods[p->first]->jboolean_fnPtr = (jboolean (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

            if(cls->methods[p->first]->return_descriptor.at(0) == 'L' || cls->methods[p->first]->return_descriptor.at(0) == '[')
                cls->methods[p->first]->jobject_fnPtr = (object* (*)(JNIEnv *, jobject, ...))(methods+i)->fnPtr ;

        }
    }

    return 0 ;
}

jint UnregisterNatives(JNIEnv *env, jclass clazz)
{

    //code changed 24/9/2005
    //Class *cls = static_cast<Class *>(clazz) ;
    object* obj2 = (object*)clazz ;
    Class* cls = obj2->class_pointer ;
    //end of code changed 24/9/2005

    //code changed 17/11/2004
    //typedef map<method_key, method *>::const_iterator CI ;
    typedef map<string, method *>::const_iterator CI ;    
    //end of code changed 17/11/2004

    for(CI p=cls->methods.begin(); p!= cls->methods.end(); ++p)
    {
        if(cls->methods[p->first]->blnNative == true)
        {
            //don't think we need to set the function
            //pointers to NULL; just in case
            cls->methods[p->first]->void_fnPtr = NULL ;
            cls->methods[p->first]->jobject_fnPtr = NULL ;
            cls->methods[p->first]->jboolean_fnPtr = NULL ;
            cls->methods[p->first]->jbyte_fnPtr = NULL ;
            cls->methods[p->first]->jchar_fnPtr = NULL ;
            cls->methods[p->first]->jshort_fnPtr = NULL ;
            cls->methods[p->first]->jint_fnPtr = NULL ;
            cls->methods[p->first]->jlong_fnPtr = NULL ;
            cls->methods[p->first]->jfloat_fnPtr = NULL ;
            cls->methods[p->first]->jdouble_fnPtr = NULL ;

        }
    }

    return 0 ;
}

jint MonitorEnter(JNIEnv *env, jobject obj)
{
    int c = theJVM->curr_thread ;

    object *object1 = static_cast<object *>(obj) ;

	if(object1->mon.blnFree == true)
	{
		object1->mon.owner_thread_id = theJVM->threads.at(c)->thread_id ;
		object1->mon.increment_count() ;
	}
	else
	{
		if(object1->mon.owner_thread_id == theJVM->threads.at(c)->thread_id)
			object1->mon.increment_count() ;
		else
		{
			if(theJVM->curr_thread != theJVM->threads.size()-1)
				theJVM->curr_thread++ ;
			else
				theJVM->curr_thread = 0 ;
		}
	}

    return 0 ;
}

jint MonitorExit(JNIEnv *env, jobject obj)
{
        int c = theJVM->curr_thread ;

        object *object1 = static_cast<object *>(obj) ;

	if(theJVM->threads.at(c)->thread_id != object1->mon.owner_thread_id)
	{
		theJVM->throw_exception("java/lang/IllegalMonitorStateException") ;
		return -1 ;
	}

	object1->mon.decrement_count() ;

	return 0 ;
}

jint GetJavaVM(JNIEnv *env, JavaVM **vm)
{
    *vm = &(theJVM->java_vm) ;
    return 0 ;    
}

jint DestroyJavaVM(JavaVM *vm)
{
    delete vm ;

    return 0 ;
}

jint AttachCurrentThread(JavaVM *vm, void **p_env, void *thr_args)
{
    //jvm *vm1 = (jvm *)vm ;

    //*p_env = (void *)&(vm1->threads.at(curr_thread)->jni_env) ;
    *p_env = (void *)&(theJVM->jni_env) ;

    //we aren't making use of the thread arguments now

    return 0 ; 
}

jint DetachCurrentThread(JavaVM *vm)
{
    jvm *vm1 = (jvm *)vm ;

    for(int i=0; i<vm1->threads.at(vm1->curr_thread)->monitors.size(); i++)
        vm1->threads.at(vm1->curr_thread)->monitors.at(i).decrement_count() ;

    //how to notify all the threads that
    //are waiting for this thread to die?

    return 0 ;
}

//code changed 13/2/2005, again on 2/3/2005, again on 6/8/2005
//jvm::jvm():nof_instructions(0),curr_thread(0)
//jvm::jvm():nof_instructions(0),curr_thread(0),blnStringClassInitialised(false)
//jvm::jvm():nof_instructions(0),curr_thread(0),blnStringClassInitialised(false),blnSystemPropertiesCreated(false)
jvm::jvm(string cp):nof_instructions(0),curr_thread(0),blnStringClassInitialised(false),blnSystemPropertiesCreated(false)
//end of code changed 13/2/2005, again on 2/3/2005, again on 6/8/2005
{


	//code added 15/9/2005
	blnTraceInstructions = false ;
	blnTraceMethodCalls = false ;
	blnRunFinalizersOnExit = false ;
	blnShuttingDown = false ;
	blnStringClassInitialised = false ;
	blnSystemPropertiesCreated = false ;
	//end of code added 15/9/2005

	//code added 6/8/2005
	classpath = cp ;
	//end of code added 6/8/2005
	
        intf.reserved0 = NULL ;
        intf.reserved1 = NULL ;
        intf.reserved2 = NULL ;
        intf.reserved3 = NULL ;
        intf.GetVersion = &GetVersion ;

        intf.DefineClass = &DefineClass ;
        intf.FindClass = &FindClass ;
        intf.FromReflectedMethod = NULL ;
        intf.FromReflectedField = NULL ;
        intf.ToReflectedMethod = NULL ;
        intf.GetSuperclass = &GetSuperclass ;
        intf.IsAssignableFrom = &IsAssignableFrom ;
        intf.ToReflectedField = NULL ;

        intf.Throw = &Throw ;
        intf.ThrowNew = &ThrowNew ;
        intf.ExceptionOccurred = &ExceptionOccurred ;
        intf.ExceptionDescribe = &ExceptionDescribe ;
        intf.ExceptionClear = &ExceptionClear ;
        intf.FatalError = &FatalError ;
        intf.PushLocalFrame = NULL ;
        intf.PopLocalFrame = NULL ;

        intf.NewGlobalRef = &NewGlobalRef ;
        intf.DeleteGlobalRef = &DeleteGlobalRef ;
        intf.DeleteLocalRef = &DeleteLocalRef ;
        intf.IsSameObject = &IsSameObject ;
        intf.NewLocalRef = NULL ;
        intf.EnsureLocalCapacity = NULL ;

        intf.AllocObject = &AllocObject ;
        intf.NewObject = &NewObject ;
        intf.NewObjectV = (jobject (*)(JNIEnv*,jclass,jmethodID,va_list))&NewObject ;
        intf.NewObjectA = &NewObjectA ;

        intf.GetObjectClass = &GetObjectClass ;
        intf.IsInstanceOf = &IsInstanceOf ;

        intf.GetMethodID = &GetMethodID ;

        intf.CallObjectMethod = &CallObjectMethod ;
        intf.CallObjectMethodV = (jobject (*)(JNIEnv*,jobject,jmethodID,va_list))&CallObjectMethod ;
        intf.CallObjectMethodA = &CallObjectMethodA ;
        intf.CallBooleanMethod = &CallBooleanMethod ;
        intf.CallBooleanMethodV = (jboolean (*)(JNIEnv*,jobject,jmethodID,va_list))&CallBooleanMethod ;
        intf.CallBooleanMethodA = &CallBooleanMethodA ;
        intf.CallByteMethod = &CallByteMethod ;
        intf.CallByteMethodV = (jbyte (*)(JNIEnv*,jobject,jmethodID,va_list))&CallByteMethod ;
        intf.CallByteMethodA = &CallByteMethodA ;
        intf.CallCharMethod = &CallCharMethod ;
        intf.CallCharMethodV = (jchar (*)(JNIEnv*,jobject,jmethodID,va_list))&CallCharMethod ;
        intf.CallCharMethodA = &CallCharMethodA ;
        intf.CallShortMethod = &CallShortMethod ;
        intf.CallShortMethodV = (jshort (*)(JNIEnv*,jobject,jmethodID,va_list))&CallShortMethod ;
        intf.CallShortMethodA = &CallShortMethodA ;
        intf.CallIntMethod = &CallIntMethod ;
        intf.CallIntMethodV = (jint (*)(JNIEnv*,jobject,jmethodID,va_list))&CallIntMethod ;
        intf.CallIntMethodA = &CallIntMethodA ;
        intf.CallLongMethod = &CallLongMethod ;
        intf.CallLongMethodV = (jlong (*)(JNIEnv*,jobject,jmethodID,va_list))&CallLongMethod ;
        intf.CallLongMethodA = &CallLongMethodA ;
        intf.CallFloatMethod = &CallFloatMethod ;
        intf.CallFloatMethodV = (jfloat (*)(JNIEnv*,jobject,jmethodID,va_list))&CallFloatMethod ;
        intf.CallFloatMethodA = &CallFloatMethodA ;
        intf.CallDoubleMethod = &CallDoubleMethod ;
        intf.CallDoubleMethodV = (jdouble (*)(JNIEnv*,jobject,jmethodID,va_list))&CallDoubleMethod ;
        intf.CallDoubleMethodA = &CallDoubleMethodA ;
        intf.CallVoidMethod = &CallVoidMethod ;
        intf.CallVoidMethodV = (void (*)(JNIEnv*,jobject,jmethodID,va_list))&CallVoidMethod ;
        intf.CallVoidMethodA = &CallVoidMethodA ;

        intf.CallNonvirtualObjectMethod = &CallNonvirtualObjectMethod ;
        intf.CallNonvirtualObjectMethodV = (jobject (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualObjectMethod ;
        intf.CallNonvirtualObjectMethodA = &CallNonvirtualObjectMethodA ;
        intf.CallNonvirtualBooleanMethod = &CallNonvirtualBooleanMethod ;
        intf.CallNonvirtualBooleanMethodV = (jboolean (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualBooleanMethod ;
        intf.CallNonvirtualBooleanMethodA = &CallNonvirtualBooleanMethodA ;
        intf.CallNonvirtualByteMethod = &CallNonvirtualByteMethod ;
        intf.CallNonvirtualByteMethodV = (jbyte (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualByteMethod ;
        intf.CallNonvirtualByteMethodA = &CallNonvirtualByteMethodA ;
        intf.CallNonvirtualCharMethod = &CallNonvirtualCharMethod ;
        intf.CallNonvirtualCharMethodV = (jchar (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualCharMethod ;
        intf.CallNonvirtualCharMethodA = &CallNonvirtualCharMethodA ;
        intf.CallNonvirtualShortMethod = &CallNonvirtualShortMethod ;
        intf.CallNonvirtualShortMethodV = (jshort (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualShortMethod ;
        intf.CallNonvirtualShortMethodA = &CallNonvirtualShortMethodA ;
        intf.CallNonvirtualIntMethod = &CallNonvirtualIntMethod ;
        intf.CallNonvirtualIntMethodV = (jint (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualIntMethod ;
        intf.CallNonvirtualIntMethodA = &CallNonvirtualIntMethodA ;
        intf.CallNonvirtualLongMethod = &CallNonvirtualLongMethod ;
        intf.CallNonvirtualLongMethodV = (jlong (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualLongMethod ;
        intf.CallNonvirtualLongMethodA = &CallNonvirtualLongMethodA ;
        intf.CallNonvirtualFloatMethod = &CallNonvirtualFloatMethod ;
        intf.CallNonvirtualFloatMethodV = (jfloat (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualFloatMethod ;
        intf.CallNonvirtualFloatMethodA = &CallNonvirtualFloatMethodA ;
        intf.CallNonvirtualDoubleMethod = &CallNonvirtualDoubleMethod ;
        intf.CallNonvirtualDoubleMethodV = (jdouble (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualDoubleMethod ;
        intf.CallNonvirtualDoubleMethodA = &CallNonvirtualDoubleMethodA ;
        intf.CallNonvirtualVoidMethod = &CallNonvirtualVoidMethod ;
        intf.CallNonvirtualVoidMethodV = (void (*)(JNIEnv*,jobject,jclass,jmethodID,va_list))&CallNonvirtualVoidMethod ;
        intf.CallNonvirtualVoidMethodA = &CallNonvirtualVoidMethodA ;

        intf.GetFieldID = &GetFieldID ;

        intf.GetObjectField = &GetObjectField ;
        intf.GetBooleanField = &GetBooleanField ;
        intf.GetByteField = &GetByteField ;
        intf.GetCharField = &GetCharField ;
        intf.GetShortField = &GetShortField ;
        intf.GetIntField = &GetIntField ;
        intf.GetLongField = &GetLongField ;
        intf.GetFloatField = &GetFloatField ;
        intf.GetDoubleField = &GetDoubleField ;
        intf.SetObjectField = &SetObjectField ;
        intf.SetBooleanField = &SetBooleanField ;
        intf.SetByteField = &SetByteField ;
        intf.SetCharField = &SetCharField ;
        intf.SetShortField = &SetShortField ;
        intf.SetIntField = &SetIntField ;
        intf.SetLongField = &SetLongField ;
        intf.SetFloatField = &SetFloatField ;
        intf.SetDoubleField = &SetDoubleField ;

        intf.GetStaticMethodID = &GetStaticMethodID ;

        intf.CallStaticObjectMethod = &CallStaticObjectMethod ;
        intf.CallStaticObjectMethodV = (jobject (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticObjectMethod ;
        intf.CallStaticObjectMethodA = &CallStaticObjectMethodA ;
        intf.CallStaticBooleanMethod = &CallStaticBooleanMethod ;
        intf.CallStaticBooleanMethodV = (jboolean (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticBooleanMethod ;
        intf.CallStaticBooleanMethodA = &CallStaticBooleanMethodA ;
        intf.CallStaticByteMethod = &CallStaticByteMethod ;
        intf.CallStaticByteMethodV = (jbyte (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticByteMethod ;
        intf.CallStaticByteMethodA = &CallStaticByteMethodA ;
        intf.CallStaticCharMethod = &CallStaticCharMethod ;
        intf.CallStaticCharMethodV = (jchar (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticCharMethod ;
        intf.CallStaticCharMethodA = &CallStaticCharMethodA ;
        intf.CallStaticShortMethod = &CallStaticShortMethod ;
        intf.CallStaticShortMethodV = (jshort (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticShortMethod ;
        intf.CallStaticShortMethodA = &CallStaticShortMethodA ;
        intf.CallStaticIntMethod = &CallStaticIntMethod ;
        intf.CallStaticIntMethodV = (jint (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticIntMethod ;
        intf.CallStaticIntMethodA = &CallStaticIntMethodA ;
        intf.CallStaticLongMethod = &CallStaticLongMethod ;
        intf.CallStaticLongMethodV = (jlong (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticLongMethod ;
        intf.CallStaticLongMethodA = &CallStaticLongMethodA ;
        intf.CallStaticFloatMethod = &CallStaticFloatMethod ;
        intf.CallStaticFloatMethodV = (jfloat (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticFloatMethod ;
        intf.CallStaticFloatMethodA = &CallStaticFloatMethodA ;
        intf.CallStaticDoubleMethod = &CallStaticDoubleMethod ;
        intf.CallStaticDoubleMethodV = (jdouble (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticDoubleMethod ;
        intf.CallStaticDoubleMethodA = &CallStaticDoubleMethodA ;
        intf.CallStaticVoidMethod = &CallStaticVoidMethod ;
        intf.CallStaticVoidMethodV = (void (*)(JNIEnv*,jclass,jmethodID,va_list))&CallStaticVoidMethod ;
        intf.CallStaticVoidMethodA = &CallStaticVoidMethodA ;

        intf.GetStaticFieldID = &GetStaticFieldID ;

        intf.GetStaticObjectField = &GetStaticObjectField ;
        intf.GetStaticBooleanField = &GetStaticBooleanField ;
        intf.GetStaticByteField = &GetStaticByteField ;
        intf.GetStaticCharField = &GetStaticCharField ;
        intf.GetStaticShortField = &GetStaticShortField ;
        intf.GetStaticIntField = &GetStaticIntField ;
        intf.GetStaticLongField = &GetStaticLongField ;
        intf.GetStaticFloatField = &GetStaticFloatField ;
        intf.GetStaticDoubleField = &GetStaticDoubleField ;

        intf.SetStaticObjectField = &SetStaticObjectField ;
        intf.SetStaticBooleanField = &SetStaticBooleanField ;
        intf.SetStaticByteField = &SetStaticByteField ;
        intf.SetStaticCharField = &SetStaticCharField ;
        intf.SetStaticShortField = &SetStaticShortField ;
        intf.SetStaticIntField = &SetStaticIntField ;
        intf.SetStaticLongField = &SetStaticLongField ;
        intf.SetStaticFloatField = &SetStaticFloatField ;
        intf.SetStaticDoubleField = &SetStaticDoubleField ;

        intf.NewString = &NewString ;
        intf.GetStringLength = &GetStringLength ;

        intf.GetStringChars = &GetStringChars ;
        intf.ReleaseStringChars = &ReleaseStringChars ;

        intf.NewStringUTF = &NewStringUTF ;
        intf.GetStringUTFLength = &GetStringUTFLength ;
        intf.GetStringUTFChars = &GetStringUTFChars ;
        intf.ReleaseStringUTFChars = &ReleaseStringUTFChars ;

        intf.GetArrayLength = &GetArrayLength ;

        intf.NewObjectArray= &NewObjectArray ;
        intf.GetObjectArrayElement = &GetObjectArrayElement ;
        intf.SetObjectArrayElement = &SetObjectArrayElement ;

        intf.NewBooleanArray = &NewBooleanArray ;
        intf.NewByteArray = &NewByteArray ;
        intf.NewCharArray = &NewCharArray ;
        intf.NewShortArray = &NewShortArray ;
        intf.NewIntArray = &NewIntArray ;
        intf.NewLongArray = &NewLongArray ;
        intf.NewFloatArray = &NewFloatArray ;
        intf.NewDoubleArray = &NewDoubleArray ;

        intf.GetBooleanArrayElements = &GetBooleanArrayElements ;
        intf.GetByteArrayElements = &GetByteArrayElements ;
        intf.GetCharArrayElements = &GetCharArrayElements ;
        intf.GetShortArrayElements = &GetShortArrayElements ;
        intf.GetIntArrayElements = &GetIntArrayElements ;
        intf.GetLongArrayElements = &GetLongArrayElements ;
        intf.GetFloatArrayElements = &GetFloatArrayElements ;
        intf.GetDoubleArrayElements = &GetDoubleArrayElements ;

        intf.ReleaseBooleanArrayElements = &ReleaseBooleanArrayElements ;
        intf.ReleaseByteArrayElements = &ReleaseByteArrayElements ;
        intf.ReleaseCharArrayElements = &ReleaseCharArrayElements ;
        intf.ReleaseShortArrayElements = &ReleaseShortArrayElements ;
        intf.ReleaseIntArrayElements = &ReleaseIntArrayElements ;
        intf.ReleaseLongArrayElements = &ReleaseLongArrayElements ;
        intf.ReleaseFloatArrayElements = &ReleaseFloatArrayElements ;
        intf.ReleaseDoubleArrayElements = &ReleaseDoubleArrayElements ;

        intf.GetBooleanArrayRegion = &GetBooleanArrayRegion ;
        intf.GetByteArrayRegion = &GetByteArrayRegion ;
        intf.GetCharArrayRegion = &GetCharArrayRegion ;
        intf.GetShortArrayRegion = &GetShortArrayRegion ;
        intf.GetIntArrayRegion = &GetIntArrayRegion ;
        intf.GetLongArrayRegion = &GetLongArrayRegion ;
        intf.GetFloatArrayRegion = &GetFloatArrayRegion ;
        intf.GetDoubleArrayRegion = &GetDoubleArrayRegion ;
        intf.SetBooleanArrayRegion = &SetBooleanArrayRegion ;
        intf.SetByteArrayRegion = &SetByteArrayRegion ;
        intf.SetCharArrayRegion = &SetCharArrayRegion ;
        intf.SetShortArrayRegion = &SetShortArrayRegion ;
        intf.SetIntArrayRegion = &SetIntArrayRegion ;
        intf.SetLongArrayRegion = &SetLongArrayRegion ;
        intf.SetFloatArrayRegion = &SetFloatArrayRegion ;
        intf.SetDoubleArrayRegion = &SetDoubleArrayRegion ;

        intf.RegisterNatives = &RegisterNatives ;
        intf.UnregisterNatives = &UnregisterNatives ;

        intf.MonitorEnter = &MonitorEnter ;
        intf.MonitorExit = &MonitorExit ;

        intf.GetJavaVM = &GetJavaVM ;

	//code added 13/5/2004

        //code changed 10/9/2005
        /*
	//bootstrap: we have to load com_vajra_Runtime.so
	//This will not be required once we integrate Vajra
	//with Classpath
	HINSTANCE runtime_handle ;

#ifdef WIN32
	runtime_handle = LoadLibrary("com_vajra_Runtime") ;
#else
	runtime_handle = dlopen("com_vajra_Runtime.so",RTLD_LAZY) ;
#endif

	
	native_library_handles.push_back(runtime_handle) ;	
        */
        //end of code changed 10/9/2005
/*		
	HINSTANCE object_handle ;

#ifdef WIN32
	object_handle = LoadLibrary("java_lang_Object") ;
#else
	object_handle = dlopen("java_lang_Object.so",RTLD_LAZY) ;
#endif

	
	native_library_handles.push_back(object_handle) ;	

	//temp hack; this should be done by calling
	//com.vajra.System.loadLibrary() from the static block
	//of the com.vajra.Console class; for some reason,
	//doing so produces a segmentation fault
	HINSTANCE console_handle ;

#ifdef WIN32
	console_handle = LoadLibrary("com_vajra_Console") ;
#else
	console_handle = dlopen("com_vajra_Console.so",RTLD_LAZY) ;
#endif

	
	native_library_handles.push_back(console_handle) ;	
	//end of temporary hack
*/

	//code added 14/11/2004
	//to load the .so's required for Classpath
	
	HINSTANCE gtkpeer_handle ;
	HINSTANCE javaawt_handle ;
	HINSTANCE javaio_handle ;
	HINSTANCE javalangreflect_handle ;
	HINSTANCE javalang_handle ;
	HINSTANCE javanet_handle ;
	HINSTANCE javanio_handle ;
	HINSTANCE javautil_handle ;

#ifdef WIN32
	gtkpeer_handle = LoadLibrary("libgtkpeer") ;
	javaawt_handle = LoadLibrary("libjavaawt") ;
	javaio_handle = LoadLibrary("libjavaio") ;
	javalangreflect_handle = LoadLibrary("libjavalangreflect") ;

	//code changed 27/3/2005
	//this gets loaded in java.lang.System's <clinit>
	//javalang_handle = LoadLibrary("libjavalang") ;
	//end of code changed 27/3/2005

	javanet_handle = LoadLibrary("libjavanet") ;
	javanio_handle = LoadLibrary("libjavanio") ;
	javautil_handle = LoadLibrary("libjavautil") ;
#else
	gtkpeer_handle = dlopen("libgtkpeer.so",RTLD_LAZY) ;
	javaawt_handle = dlopen("libjavaawt.so",RTLD_LAZY) ;
	javaio_handle = dlopen("libjavaio.so",RTLD_LAZY) ;
	javalangreflect_handle = dlopen("libjavalangreflect.so",RTLD_LAZY) ;

	//code changed 27/3/2005
	//this gets loaded in java.lang.System's <clinit>
	//javalang_handle = dlopen("libjavalang.so",RTLD_LAZY) ;
	//end of code changed 27/3/2005

	javanet_handle = dlopen("libjavanet.so",RTLD_LAZY) ;
	javanio_handle = dlopen("libjavanio.so",RTLD_LAZY) ;
	javautil_handle = dlopen("libjavautil.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
	assert(gtkpeer_handle != NULL) ;
	assert(javaawt_handle != NULL) ;
	assert(javaio_handle != NULL) ;
	assert(javalangreflect_handle != NULL) ;

	//code changed 27/3/2005
	//assert(javalang_handle != NULL) ;
	//end of code changed 27/3/2005

	assert(javanet_handle != NULL) ;
	assert(javanio_handle != NULL) ;
	assert(javautil_handle != NULL) ;	
#endif

	native_library_handles.push_back(gtkpeer_handle) ;
	native_library_handles.push_back(javaawt_handle) ;	
	native_library_handles.push_back(javaio_handle) ;	
	native_library_handles.push_back(javalangreflect_handle) ;	

	//code changed 27/3/2005
	//native_library_handles.push_back(javalang_handle) ;	
	//end of code changed 27/3/2005

	native_library_handles.push_back(javanet_handle) ;	
	native_library_handles.push_back(javanio_handle) ;	
	native_library_handles.push_back(javautil_handle) ;								
	//end of code added 14/11/2004

	//code added 15/11/2004
        
	HINSTANCE javalangVMRuntime_handle ;

#ifdef WIN32
	javalangVMRuntime_handle = LoadLibrary("java_lang_VMRuntime") ;
#else
	javalangVMRuntime_handle = dlopen("java_lang_VMRuntime.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
	assert(javalangVMRuntime_handle != NULL) ;
#endif

	native_library_handles.push_back(javalangVMRuntime_handle) ;
	//end of coded 15/11/2004 

	//code added 17/2/2005
	HINSTANCE javalangVMSecurityManager_handle ;

#ifdef WIN32
	javalangVMSecurityManager_handle = LoadLibrary("java_lang_VMSecurityManager") ;
#else
	javalangVMSecurityManager_handle = dlopen("java_lang_VMSecurityManager.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
        if(javalangVMSecurityManager_handle == NULL)
                cout << dlerror() << endl << flush ;
	assert(javalangVMSecurityManager_handle != NULL) ;
#endif

	native_library_handles.push_back(javalangVMSecurityManager_handle) ;
	//end of coded 17/2/2005

	//code added 26/2/2005
	HINSTANCE javalangVMSystem_handle ;

#ifdef WIN32
	javalangVMSystem_handle = LoadLibrary("java_lang_VMSystem") ;
#else
	javalangVMSystem_handle = dlopen("java_lang_VMSystem.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
        if(javalangVMSystem_handle == NULL)
	        cout << dlerror() << endl << flush ;
	assert(javalangVMSystem_handle != NULL) ;
#endif

	native_library_handles.push_back(javalangVMSystem_handle) ;
	//end of coded 26/2/2005

	//code added 31/7/2005
	HINSTANCE javalangVMObject_handle ;

#ifdef WIN32
	javalangVMObject_handle = LoadLibrary("java_lang_VMObject") ;
#else
	javalangVMObject_handle = dlopen("java_lang_VMObject.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
	if(javalangVMObject_handle == NULL)
                cout << dlerror() << endl << flush ;
	assert(javalangVMObject_handle != NULL) ;
#endif

	native_library_handles.push_back(javalangVMObject_handle) ;
	//end of coded 31/7/2005

	//code added 11/8/2005
	HINSTANCE javalangVMClass_handle ;

#ifdef WIN32
	javalangVMClass_handle = LoadLibrary("java_lang_VMClass") ;
#else
	javalangVMClass_handle = dlopen("java_lang_VMClass.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
        if(javalangVMClass_handle == NULL)
	        cout << dlerror() << endl << flush ;
	assert(javalangVMClass_handle != NULL) ;
#endif

	native_library_handles.push_back(javalangVMClass_handle) ;
	//end of coded 11/8/2005

        
	//code added 13/8/2005
	/*
	HINSTANCE javalangreflectConstructor_handle ;

#ifdef WIN32
	javalangreflectConstructor_handle = LoadLibrary("java_lang_reflect_Constructor") ;
#else
	javalangreflectConstructor_handle = dlopen("java_lang_reflect_Constructor.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
	if(javalangreflectConstructor_handle == NULL)
		cout << dlerror() << endl << flush ;
	assert(javalangreflectConstructor_handle != NULL) ;
#endif

	native_library_handles.push_back(javalangreflectConstructor_handle) ;
	*/
	//end of coded 13/8/2005

        //at present vajra.so contains only
        //the native methods of VMClassLoader
	//and VMThread. Plan to move all 
	//native methods into it.
	HINSTANCE vajra_native_libs_handle ;

#ifdef WIN32
	vajra_native_libs_handle = LoadLibrary("vajra") ;
#else
	vajra_native_libs_handle = dlopen("vajra.so",RTLD_LAZY) ;
#endif

#if DEBUG_LEVEL >= 3
	if(vajra_native_libs_handle == NULL)
		cout << dlerror() << endl << flush ;
	assert(vajra_native_libs_handle != NULL) ;
#endif

	native_library_handles.push_back(vajra_native_libs_handle) ;

	//code added 12/2/2005
	load_class("java/lang/String") ;
	//end of code added 12/2/2005

	//end of bootstrap code

	//TO DO: this can be knocked off since
	//we have included jvm_ptr itself
	intf.native_library_handles = &native_library_handles ;
	//end code added 13/5/2004

	//code added 24/6/2004
	//for implementing Classpath's classes' native methods
	intf.jvm_ptr = this ;
	//end of code added 24/6/2004


        jni_env.functions = &intf ;

        //not setting other attributes
        init_args.version = 0x00010003 ;


        invoke_intf.reserved0 = NULL ;
        invoke_intf.reserved1 = NULL ;
        invoke_intf.reserved2 = NULL ;

        invoke_intf.DestroyJavaVM = &DestroyJavaVM ;
        invoke_intf.AttachCurrentThread = &AttachCurrentThread ;
        invoke_intf.DetachCurrentThread = &DetachCurrentThread ;

        invoke_intf.GetEnv = NULL ; //this function is defined in jni.h,
                                    //but documentation omits this

        java_vm.functions = &invoke_intf ;

}

//code added 17/9/2005
void jvm::populateLocalVariablesForFrame(jmethodID* methodID, va_list* ap, vector<LocalVariable *>& v)
{
	method *m = (method *)methodID ;
	for(int i=0; i<m->parameters->size(); i++)
	{
		LocalVariable *val = theJVM->createLocalVariable() ;
		if(m->parameters->at(i) == "B")
		{
			val->value_type = value::_BYTE ;
			val->i_value = va_arg(*ap, int) ;
		}
		else if(m->parameters->at(i) == "C")
		{
			val->value_type = value::CHAR ;
			val->i_value = va_arg(*ap, int) ;
		}
		else if(m->parameters->at(i) == "D")
		{
			val->value_type = value::DOUBLE ;
			val->double_value = va_arg(*ap, double) ;
			LocalVariable *dummy = theJVM->createLocalVariable() ;
			dummy->valid_flag = false;
			v.push_back(dummy) ;
		}
		else if(m->parameters->at(i) == "F")
		{
			val->value_type = value::FLOAT ;
			val->float_value = va_arg(*ap, double) ;
		}
		else if(m->parameters->at(i) == "I")
		{
			val->value_type = value::INT ;
			val->i_value = va_arg(*ap, int) ;
		}
		else if(m->parameters->at(i) == "J")
		{
			val->value_type = value::LONG ;
			val->long_value = va_arg(*ap, long) ;
			LocalVariable *dummy = theJVM->createLocalVariable() ;
			dummy->valid_flag = false;
			v.push_back(dummy) ;
		}
		else if(m->parameters->at(i)[0] == 'L')
		{
			val->value_type = value::REFERENCE ;
			val->ref_value = (object *)va_arg(*ap, jobject) ;
		}
		else if(m->parameters->at(i) == "S")
		{
			val->value_type = value::SHORT ;
			val->i_value = va_arg(*ap, int) ;
		}
		else if(m->parameters->at(i) == "Z")
		{
			val->value_type = value::BOOLEAN ;
			int temp = va_arg(*ap, int) ;
			val->i_value = (temp = 0) ? false : true ;
		}		
		else if(m->parameters->at(i)[0] == '[')
		{
			val->value_type = value::REFERENCE ;
			val->ref_value = (object *)va_arg(*ap, jobject) ;
		}
		v.push_back(val) ;
	}        
}
//end of code added 17/9/2005
