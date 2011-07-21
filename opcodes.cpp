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

 /* this file contains only the execute_instruction()
 function (for the sake of convenience and ease
 of maintenance) */

 #include "jvm.h"
 #include "util.h"

 #include <limits.h>
 #include <cstdlib>
 #include <cassert>

 #include <iostream>

 //code added 6/11/2004
 #include "logger.h"
 #include <sstream>
 //end of code added 6/11/2004

 //this class is used for the
 //lookupswitch instruction
 class match_offset
 {
 public:
	 int match ;
	 int offset ;
 } ;
 
 void jvm::execute_instruction()
 {

 //#if DEBUG_LEVEL >= 3
 //	Logger::LogFunctionEntry("jvm::execute_instruction()") ;
 //#endif 	

	 //just for convenience's sake
	 int c = curr_thread ;
	 stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;

	 //thread *curr_thread = threads.at(c) ;
	 int prog_counter = threads.at(c)->pc ;
	 frame *curr_frame = threads.at(c)->java_stack.top() ;
	 method *current_method = curr_frame->curr_method ;

	 Class *this_class = current_method->type ;

 	int instruction_number = current_method->code.byte_code.at(prog_counter);
 #if DEBUG_LEVEL >= 3
 	//standard way to convert an int to a string
 	//from http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-38.1
 	std::ostringstream o ;
 	o <<  "Frame ID: " << curr_frame << "; Instruction: " << util::getInstruction(instruction_number) << "(" << instruction_number <<")" ;

	//code changed 25/3/2005
 	//string message = "Class: " + this_class->name + "; " +
	//	 	"Method: " + current_method->name + "; " + o.str()  ;
 	string message = this_class->name + "." +
		 	 current_method->name + ";" + o.str()  ;
	//end of code changed 25/3/2005

	//Logger::LogFunctionEntry("jvm::execute_instruction(): " + message) ; 
        FunctionCallLogger fcl("jvm::execute_instruction(): " + message) ;
 #endif 	

	//code added 2/3/2005
	//if(blnSystemPropertiesCreated != true)
	//{
	//  blnSystemPropertiesCreated = true ;
	//  create_system_properties() ;
	//}
	//end of code added 2/3/3005

	 switch(instruction_number)
	 {
 	
		 case 50: //aaload: load reference from array
		 {
			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }

//code added 30/1/2005
#if DEBUG_LEVEL >= 3
			 assert(arrayref->value_type == value::REFERENCE) ;
			 assert(arrayref->ref_value->blnArrayObject == true) ;
#endif
//end of code added 30/1/2005
			 
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    index->i_value >= arrayref->ref_value->items.size())
			 {
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 value *obj_val = createValue() ;
			 obj_val->value_type = value::REFERENCE ;

//code added 11/2/2005
#if DEBUG_LEVEL >= 3
			assert(arrayref->ref_value->items.at(index->i_value) != NULL) ;
#endif
//end of code added 11/2/2005

			 obj_val->ref_value = arrayref->ref_value->items.at(index->i_value) ;
			 op_st->push(obj_val) ;

//code added 28/5/2005
#if DEBUG_LEVEL >= 3
			 cout << "aaload(): " << "arrayref = " << arrayref->ref_value << "; " ;
			 cout << "index = " << index->i_value << "; " ;
			 cout << "value: " << obj_val << endl << flush ;
			 obj_val->display() ;
#endif			 
//end of code added 28/5/2005			 
			 
			 
			 //code changed 9/2/2005
			 //delete index ;
			 //end of code changed 9/2/2005

			 //code changed 3/2/2005
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 83: //aastore: store into reference array
		 {
			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;

//code added 13/2/2005
#if DEBUG_LEVEL >= 3
			assert(obj_val->value_type == value::REFERENCE) ;
			assert(index->value_type == value::INT) ;			
			assert(arrayref->value_type == value::REFERENCE) ;
			assert(arrayref->ref_value->blnArrayObject == true) ;
			//assert(arrayref->ref_value->type != NULL) ;
#endif
//end of code added 13/2/2005			 
			 
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete obj_val ;
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    index->i_value >= arrayref->ref_value->items.size())
			 {
				 delete obj_val ;
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 //check for assignment compatibility
			 Class *S = obj_val->ref_value->type ;

			 //code changed 13/2/2005
			 //Class *T = arrayref->ref_value->type->array_class_ref ;
			 Class *T = arrayref->ref_value->array_component_type ;
			 //end of code changed 13/2/2005

			 if(IsAssignmentCompatible(S,T) == false)
			 {
				 delete obj_val ;
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayStoreException") ;
				 return ;
			 }
			 //end check for assignment compatibility

			 //code changed 29/7/2005
			 //delete arrayref->ref_value->items.at(index->i_value) ;
			 //end of code changed 29/7/2005
			 
			 arrayref->ref_value->items.at(index->i_value) = obj_val->ref_value ;

			 //code changed 3/2/2005
			 //delete obj_val ;
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005
//code added 28/5/2005			 
#if DEBUG_LEVEL >= 2
			 cout << "aastore(): " ;
			 cout << "arrayref: " << arrayref->ref_value << "; " ;
			 cout << "index: " << index->i_value << "; " ;
			 cout << "value: " << obj_val << endl << flush ;
#endif			 
//end of code added 28/5/2005
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 1: //aconst_null: push null
		 {
			 value *null_val = createValue() ;
			 
			 //code added 6/2/2005
			 null_val->value_type = value::REFERENCE ;
			 //end of code added 6/2/2005
			 
			 object *null_obj = createObject() ;
			 null_obj->blnNull = true ;
			 null_val->ref_value = null_obj ;
			 op_st->push(null_val) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 25: //aload: load reference from local variable
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;

			 aload(index) ;

			 threads.at(c)->pc += 2 ; //2 because the index of the
			 break ;		  //local variable is also stored
						  //in the method's code
		 }

		 case 42: //aload_0: load reference from local variable
		 {
			 aload(0) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 43: //aload_1: load reference from local variable
		 {
			 aload(1) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 44: //aload_2: load reference from local variable
		 {
			 aload(2) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 45: //aload_3: load reference from local variable
		 {
			 aload(3) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 189: //anewarray: create new array of reference
		 {
			 int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			 int rcp_index = index1 * 256 + index2 ;

			 value *count_val = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			assert(count_val->value_type == value::INT) ;
#endif
//end of code added 4/2/2005

			 int count = count_val->i_value ;

			 if(count < 0)
			 {
				 delete count_val ;
				 throw_exception("java/lang/NegativeArraySizeException") ;
				 return ;
			 }

			 try
			 {
				 Class *cl ;
				 if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
					 cl = curr_frame->rcp->at(rcp_index-1)->resolved_class ;
				 else
				 {
					 cl = resolve_class(this_class,
							   curr_frame->rcp->at(rcp_index-1)->class_interface_name) ;

					 //code added 28/2/2005
#if DEBUG_LEVEL >= 3
					 assert(cl != NULL) ;
#endif
					 //end of code added 28/2/2005

					 curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					 curr_frame->rcp->at(rcp_index-1)->resolved_class = cl ;
				 }

				 object *obj = createObject() ;
				 obj->blnArrayObject = true ;

				//code added 22/1/2005
				obj->array_component_type = cl ;
				//end of code added 22/1/2005

				 for(int i=0; i<count; i++)
				 {
					 object *item = createObject() ;
					 item->type = cl ;
					 item->blnNull = true ;
					 obj->items.push_back(item) ;
				 }

				 value *arrayref = createValue() ;

				 arrayref->value_type = value::REFERENCE ;

//code added 11/2/2005
#if DEBUG_LEVEL >= 3
			assert(obj != NULL) ;
#endif
//end of code added 11/2/2005

				 arrayref->ref_value = obj ;

				 op_st->push(arrayref) ;

//code added 28/5/2005
#if DEBUG_LEVEL >= 2
			 	cout << "anewarray(): " ;
			 	cout << "arrayref: " << arrayref->ref_value << "; " ;
			 	cout << "size: " << count << "; " ;
			 	cout << "values: " << endl << flush ;
			 	for(int i=0; i< arrayref->ref_value->items.size(); i++)
					cout << arrayref->ref_value->items.at(i) << endl << flush ;
#endif			 
//end of code added 28/5/2005			 
				 
			 }
			 catch(LinkageError)
			 {
				 delete count_val ;
				 throw_exception("java/lang/LinkageError") ;
				 return ;
			 }
			 catch(NoClassDefFound)
			 {
				 delete count_val ;
				 throw_exception("java/lang/NoClassDefFoundError") ;
				 return ;
			 }
			 catch(FormatError)
			 {
				 delete count_val ;
				 throw_exception("java/lang/FormatError") ;
				 return ;
			 }
			 catch(ClassCircularityError)
			 {
				 delete count_val ;
				 throw_exception("java/lang/ClassCircularityError") ;
				 return ;
			 }
			 catch(IncompatibleClassChangeError)
			 {
				 delete count_val ;
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }
			 catch(IllegalAccessError)
			 {
				 delete count_val ;
				 throw_exception("java/lang/IllegalAccessError") ;
				 return ;
			 }


			 //code changed 9/2/2005
			 //delete count_val ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc += 3 ;
			 break ;

		 }

		 case 176: //areturn: return reference from method
		 {

			 value *obj_val = op_st->top() ;

//code added 6/2/2005
#if DEBUG_LEVEL >= 3
			assert(obj_val != NULL) ;
			assert(obj_val->value_type == value::REFERENCE) ;
			assert(obj_val->ref_value != NULL) ;
#endif
//end of code added 6/2/2005

			 //if the method is a synchronised method,
			 //the object's monitor should be owned
			 //by the current thread
			 
			//code added 15/1/2005
			//if(current_method->blnSynchronized == true)			
			if(current_method->blnSynchronized == true && curr_frame->mon_obj != NULL)
			//end of code added 15/1/2005
			 {
				 object *mon_obj = curr_frame->mon_obj ;

				 if(mon_obj->mon.owner_thread_id !=
				    threads.at(c)->thread_id)
				 {
					 throw_exception("java/lang/IllegalMonitorStateException") ;
					 return ;
				 }

				 mon_obj->mon.decrement_count() ;
			 }

			 //code added 29/1/2005
			 //to preserve the value to be returned
			 op_st->pop() ;
			 //end of code added 29/1/2005


			 //pop the operand stack of the
			 //current frame
			 while(!op_st->empty())
			 {
				 delete op_st->top() ;
				 op_st->pop() ;
			 }

			 //pop the current frame from the
			 //currrent thread's stack
			 threads.at(c)->java_stack.pop() ;

			 //get the next instruction to 
			 //execute (this was stored when
			 //the frame was created)
			 threads.at(c)->pc = curr_frame->prev_pc_value ;

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << "areturn(" << obj_val->ref_value << ")" << "(" << curr_frame << ")" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 
			 
			 //discard the current frame
#if DEBUG_LEVEL >= 2
			 cout << "deleting frame " << curr_frame << endl ;
#endif

			 delete curr_frame ;

			 //push the object reference to the 
			 //invoking frame
			 frame *invoking_frame = threads.at(c)->java_stack.top() ;
			 invoking_frame->operand_stack.push(obj_val) ;

			 //temp code
			 //if(current_method->name == "getProperty" || current_method->name == "get")
			 //  {
			 //    if(obj_val->ref_value->blnNull == true)
			 //      cout << "getProperties()/get() returns a null object" << endl << flush ;
			 //    //obj_val->ref_value->display() ;  
			 //  }
			 //end of temp code

			 break ;

		 }

		 case 190: //arraylength: get length of array
		 {
			 value *arrayref = op_st->top() ;
			 op_st->pop();

//code added 15/1/2005
#if DEBUG_LEVEL >= 3
			assert(arrayref != NULL) ;

			assert(arrayref->value_type == value::REFERENCE) ;
			assert(arrayref->ref_value != NULL) ;
			assert(arrayref->ref_value->blnArrayObject == true) ;
#endif
//end of code added 15/1/2005 

			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete arrayref ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }

			 value *length = createValue() ;
			 length->value_type = value::INT ;

			 //code changed 26/3/2005
			 //length->i_value = arrayref->ref_value->items.size() ;
			 if(arrayref->ref_value->blnPrimitiveArrayObject == true)
			   length->i_value = arrayref->ref_value->primitives.size() ;
			 else
			   length->i_value = arrayref->ref_value->items.size() ;
			 //end of code changed 26/3/2005

			 op_st->push(length) ;

			 //code changed 3/2/2005
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;

			 break ;
		 }

		 case 58: //astore: store reference into local variable
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;

			 astore(index) ;

			 threads.at(c)->pc += 2 ; //2 because the index of the
			 break ;		  //local variable is also stored
						  //in the method's code
		 }

		 case 75: //astore_0: store reference into local variable
		 {
			 astore(0) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 76: //astore_1: store reference into local variable
		 {
			 astore(1) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 77: //astore_2: store reference into local variable
		 {
			 astore(2) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 78: //astore_3: store reference into local variable
		 {
			 astore(3) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 191: //athrow: throw exception or error
		 {
			 athrow() ;
			 break ;
		 }

		 case 51: //baload: load byte or boolean from array
		 {
			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 value *obj_val = createValue() ;

			 if(arrayref->ref_value->primitive_type == primitive_value::_BYTE)
			 {
				 obj_val->value_type = value::_BYTE ;
				 obj_val->i_value = arrayref->ref_value->primitives.at(index->i_value)->i_value ;
			 }
			 else //it HAS to be a boolean
			 {
				 obj_val->value_type = value::BOOLEAN ;
				 obj_val->bool_value = arrayref->ref_value->primitives.at(index->i_value)->bool_value ;
			 }

			 op_st->push(obj_val) ;

			 //code changed 3/2/2005
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 84: //bastore: store into byte or boolean array
		 {
			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete obj_val ;
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete obj_val ;
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 primitive_value *pv = createPrimitiveValue() ;

			 if(arrayref->ref_value->primitive_type == primitive_value::_BYTE)
			 {
				 pv->value_type = primitive_value::_BYTE ;
				 pv->i_value = obj_val->i_value ;
			 }
			 else //it HAS to be a boolean
			 {
				 pv->value_type = primitive_value::BOOLEAN ;
				 pv->i_value = obj_val->bool_value ;
			 }

			 //delete arrayref->ref_value->primitives.at(index->i_value) ;
			 arrayref->ref_value->primitives.at(index->i_value) = pv;

			 //code changed 3/2/2005
			 //delete obj_val ;
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 16: //bipush: push byte
		 {
			 int byte = current_method->code.byte_code.at(prog_counter+1) ;

			 value *byte_val = createValue() ;

			 //code changed 4/2/2005
			 //the byte value should be sign-extended to an int value
			 //byte_val->value_type = value::_BYTE ;
			 byte_val->value_type = value::INT ;			 
			 //end of code changed 4/2/2005

			 byte_val->i_value = byte ;

			 op_st->push(byte_val) ;

			 threads.at(c)->pc+= 2 ;
			 break ;
		 }

		 case 52: //caload: load char from array
		 {
			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;

//code added 6/2/2006
#if DEBUG_LEVEL >= 3
                         assert(index->value_type == value::INT) ;
                         assert(arrayref->value_type == value::REFERENCE) ;
#endif                         
//end of code added 6/2/2006                         
                         
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 value *obj_val = createValue() ;

			 obj_val->value_type = value::CHAR ;
			 obj_val->i_value = arrayref->ref_value->primitives.at(index->i_value)->i_value ;

			 op_st->push(obj_val) ;

			 //code changed 3/2/2005
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 85: //castore: store into char array
		 {
			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;

			 //code added 6/3/2005
#if DEBUG_LEVEL >= 3
                         //code changed 5/9/2005
                         //looks like this assert is not needed (there
                         //are other places too where similar commenting out
                         //has been done)
			 //assert(obj_val->value_type == value::INT) ;
                         //end of code changed 5/9/2005
			 assert(index->value_type == value::INT) ;
			 assert(arrayref->value_type == value::REFERENCE) ;
			 assert(arrayref->ref_value->blnArrayObject == true) ;
			 assert(arrayref->ref_value->primitives.size() > 0) ;
#endif			 
			 //end of code added 6/3/2005

			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete obj_val ;
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete obj_val ;
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 primitive_value *pv = createPrimitiveValue() ;

			 pv->value_type = primitive_value::CHAR ;
			 pv->i_value = obj_val->i_value ;

			 //delete arrayref->ref_value->primitives.at(index->i_value) ;
			 arrayref->ref_value->primitives.at(index->i_value) = pv;

			 //code changed 3/2/2005
			 //delete obj_val ;
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }	

		 case 192: //checkcast: check whether object is of given type
		 {
			 int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			 int rcp_index = index1 * 256 + index2 ;

			 value *objref = op_st->top() ;

			 if(objref->ref_value->blnNull == true)
			 {
			 	 //code added 13/2/2005
			 	 threads.at(c)->pc+= 3 ;
			 	 //end of code adde 13/2/2005
				 return ;
			 }

			 Class *cl ;

			 try
			 {
				 if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
					 cl = curr_frame->rcp->at(rcp_index-1)->resolved_class ;
				 else
				 {
					 cl = resolve_class(this_class,
							    curr_frame->rcp->at(rcp_index-1)->class_interface_name) ;

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
					 assert(cl != NULL) ;
#endif
//end of code added 28/2/2005

					 curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					 curr_frame->rcp->at(rcp_index-1)->resolved_class = cl ;
				 }
			 }
			 catch(LinkageError)
			 {
				 throw_exception("java/lang/LinkageError") ;
				 return ;
			 }
			 catch(NoClassDefFound)
			 {
				 throw_exception("java/lang/NoClassDefFoundError") ;
				 return ;
			 }
			 catch(FormatError)
			 {
				 throw_exception("java/lang/FormatError") ;
				 return ;
			 }
			 catch(ClassCircularityError)
			 {
				 throw_exception("java/lang/ClassCircularityError") ;
				 return ;
			 }
			 catch(IncompatibleClassChangeError)
			 {
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }
			 catch(IllegalAccessError)
			 {
				 throw_exception("java/lang/IllegalAccessError") ;
				 return ;
			 }

			 if(IsAssignmentCompatible(objref->ref_value->type,cl) == false)
			 {
				 throw_exception("java/lang/ClassCastException") ;
				 return ;
			 }

			 threads.at(c)->pc+= 3 ;
			 break ;
		 }

		 case 144: //d2f: convert double to float
		 {

			 value *d_val = op_st->top() ;
			 op_st->pop() ;

			 value *f_val = createValue() ;
			 f_val->value_type = value::FLOAT ;
			 f_val->float_value = d_val->double_value ;
			 f_val->IsNaN = d_val->IsNaN ;
			 f_val->IsInfinity = d_val->IsInfinity ;
			 f_val->IsPositive = d_val->IsPositive ;

			 op_st->push(f_val) ;

			 delete d_val ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 142: //d2i: convert double to int
		 {

			 value *d_val = op_st->top() ;
			 op_st->pop() ;

			 value *i_val = createValue() ;
			 i_val->value_type = value::INT ;

			 if(d_val->IsNaN == true)
				 i_val->i_value = 0 ;
			 else
			   //get rid of those &*$# compiler warnings
			   //code changed 19/2/2005
			   //i_val->i_value = d_val->double_value ;
			   i_val->i_value = (int)d_val->double_value ;
			   //end of code changed 19/2/2005

			 op_st->push(i_val) ;

			 //code changed 9/2/2005
			 //delete d_val ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 143: //d2l: convert double to long
		 {

			 value *d_val = op_st->top() ;
			 op_st->pop() ;

			 value *l_val = createValue() ;
			 l_val->value_type = value::LONG ;

			 if(d_val->IsNaN == true)
				 l_val->long_value = 0 ;
			 else
			   //code changed 19/2/2005
			   //l_val->long_value = d_val->double_value ;
			   l_val->long_value = (long)d_val->double_value ;
			   //end of code changed 19/2/2005

			 op_st->push(l_val) ;

			 //code changed 9/2/2005
			 //delete d_val ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 99: //dadd: add double
		 {
			 value *val2 = op_st->top() ;
			 op_st->pop() ;
			 value *val1 = op_st->top() ;
			 op_st->pop() ;

#if DEBUG_LEVEL >= 3
			 assert(val2->value_type == value::DOUBLE) ;
			 assert(val1->value_type == value::DOUBLE) ;
#endif

			 value *res = createValue() ;

			 res->value_type = value::DOUBLE ;

			 if(val1->IsNaN == true || val2->IsNaN == true)
				 res->IsNaN = true ;
			 else
			 {
				 bool SameSign ;

				 if(val1->IsPositive == val2->IsPositive)
					 SameSign = true ;
				 else
					 SameSign = false ;

				 if(val1->IsInfinity == true && val1->IsInfinity == true)
				 {
					 if(SameSign == true)
					 {
						 res->IsInfinity = true ;
						 res->IsPositive = val1->IsPositive ; //we could use val2 here, too
					 }
					 else
						 res->IsNaN = true ;
				 }
				 else if(val1->IsInfinity == true)
				 {
					 res->IsInfinity = true ;
					 res->IsPositive = val1->IsPositive ;
				 }
				 else if(val2->IsInfinity == true)
				 {
					 res->IsInfinity = true ;
					 res->IsPositive = val2->IsPositive ;
				 }
				 else if(val1->double_value == 0 && val2->double_value == 0)
				 {
					 if(SameSign == true)
					 {
						 res->double_value = 0 ;
						 res->IsPositive = val1->IsPositive ; //we could use val2 here, too
					 }
					 else
					 {
						 res->double_value = 0 ;
						 res->IsPositive = true ;
					 }
				 }
				 else
					 res->double_value = val1->double_value + val2->double_value ;

			 }

			 op_st->push(res) ;

			 //code changed 9/2/2005
			 //delete val1 ;
			 //delete val2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 49: //daload: load double from array
		 {
			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 value *obj_val = createValue() ;

			 obj_val->value_type = value::DOUBLE ;
			 obj_val->double_value = arrayref->ref_value->primitives.at(index->i_value)->double_value ;
			 obj_val->IsNaN = arrayref->ref_value->primitives.at(index->i_value)->IsNaN ;
			 obj_val->IsInfinity = arrayref->ref_value->primitives.at(index->i_value)->IsInfinity ;
			 obj_val->IsPositive = arrayref->ref_value->primitives.at(index->i_value)->IsPositive ;

			 op_st->push(obj_val) ;

			 //code changed 3/2/2005
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 82: //dastore: store into double array
		 {
			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete obj_val ;
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete obj_val ;
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 primitive_value *pv = createPrimitiveValue() ;

			 pv->value_type = primitive_value::DOUBLE ;
			 pv->double_value = obj_val->double_value ;
			 pv->IsNaN = obj_val->IsNaN ;
			 pv->IsInfinity = obj_val->IsInfinity ;
			 pv->IsPositive = obj_val->IsPositive ;

			 //delete arrayref->ref_value->primitives.at(index->i_value) ;
			 arrayref->ref_value->primitives.at(index->i_value) = pv;

			 //code changed 3/2/2005
			 //delete obj_val ;
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 //we are not handling them separately
		 //because this is a simple implementation;
		 //no FP-strictness, value-sets, etc.
		 case 152: //dcmpg: compare double
		 case 151: //dcmpl: compare double
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *i_val = createValue() ;
			 i_val->value_type = value::INT ;

			 if(v1->double_value > v2->double_value)
				 i_val->i_value = 1 ;
			 else if(v1->double_value == v2->double_value)
				 i_val->i_value = 0 ;
			 else
				 i_val->i_value = -1 ;

			 op_st->push(i_val) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 14: //dconst_0: push double (value 0.0)
		 {
			 value *d = createValue() ;
			 d->value_type = value::DOUBLE ;
			 d->double_value = 0 ;
			 d->IsPositive = true ;

			 op_st->push(d) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 15: //dconst_1: push double (value 1.0)
		 {
			 value *d = createValue() ;
			 d->value_type = value::DOUBLE ;
			 d->double_value = 1 ;

			 op_st->push(d) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 111: //ddiv: divide double
		 {
			 value *val2 = op_st->top() ;
			 op_st->pop() ;
			 value *val1 = op_st->top() ;
			 op_st->pop() ;

			 value *res = createValue() ;

			 res->value_type = value::DOUBLE ;

			 if(val1->IsNaN == true || val2->IsNaN == true)
				 res->IsNaN = true ;
			 else
			 {
				 if(val1->IsPositive == val2->IsPositive)
					 res->IsPositive = true ;
				 else
					 res->IsPositive = false ;

				 if(val1->IsInfinity == true && val2->IsInfinity == true)
					 res->IsNaN = true ;
				 else if(val1->IsInfinity == true && val2->IsInfinity == false)
					 res->IsInfinity = true ;
				 else if(val1->IsInfinity == false && val2->IsInfinity == true)
					 res->double_value = 0 ;
				 else if(val1->double_value == 0 && val2->double_value == 0)
					 res->IsNaN = true ;
				 else if(val1->double_value == 0)
					 res->double_value = 0 ;
				 else if(val2->double_value == 0)
					 res->IsInfinity = true ;
				 else
					 res->double_value = val1->double_value / val2->double_value ;

			 }


			 op_st->push(res) ;

			 //code changed 9/2/2005
			 //delete val1 ;
			 //delete val2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 24: //dload: load double from local variable
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;

			 dload(index) ;

			 threads.at(c)->pc += 2 ; //2 because the index of the
			 break ;		  //local variable is also stored
						  //in the method's code
		 }

		 case 38: //dload_0: load double from local variable
		 {
			 dload(0) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 39: //dload_1: load double from local variable
		 {
			 dload(1) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 40: //dload_2: load double from local variable
		 {
			 dload(2) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 41: //dload_3: load double from local variable
		 {
			 dload(3) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 107: //dmul: multiply double
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::DOUBLE ;

			 if(v1->IsNaN == true || v2->IsNaN == true)
				 r->IsNaN = true ;
			 else
			 {
				 if(v1->IsPositive == v2->IsPositive == true)
					 r->IsPositive = true ;
				 else
					 r->IsPositive = false ;

				 if(v1->IsInfinity == true && v2->double_value == 0 ||
				    v2->IsInfinity == true && v1->double_value == 0)
					 r->IsNaN = true ;
				 else if(v1->IsInfinity == true || v2->IsInfinity == true)
					 r->IsInfinity = true ;
				 else
					 r->double_value = v1->double_value * v2->double_value ;

			 }

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 119: //dneg: negate double
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::DOUBLE ;

			 if(v->IsNaN == true)
				 r->IsNaN = true ;
			 else if(v->IsInfinity == true)
			 {
				 r->IsInfinity = true ;
				 r->IsPositive = !(v->IsPositive) ;
			 }
			 else if(v->double_value == 0)
			 {
				 r->double_value = 0 ;
				 r->IsPositive = !(v->IsPositive) ;
			 }
			 else
				 r->double_value = (-1.0) * v->double_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 115: //drem: remainder double
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::DOUBLE ;

			 if(v1->IsNaN == true || v2->IsNaN == true)
				 r->IsNaN = true ;
			 else
			 {
				 r->IsPositive = v1->IsPositive ;

				 if(v1->IsInfinity == true || v2->double_value == 0)
					 r->IsNaN = true ;
				 else if(v1->IsInfinity == false && v2->IsInfinity == true)
					 r->double_value = v1->double_value ;
				 else if(v1->double_value == 0 && v2->IsInfinity == false)
					 r->double_value = v1->double_value ;
				 else
				 {
				   //code changed 19/2/2005
				   //int q = v1->double_value / v2->double_value ;
				   int q = (int)(v1->double_value / v2->double_value) ;
				   //end of code changed 19/2/2005
				   r->double_value = v1->double_value - (v2->double_value * q) ;
				 }
			 }

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 175: //dreturn: return double from method
		 {
			 value *d_val = op_st->top() ;

			 //if the method is a synchronised method,
			 //the object's monitor should be owned
			 //by the current thread

			//code added 15/1/2005
			//if(current_method->blnSynchronized == true)			
			if(current_method->blnSynchronized == true && curr_frame->mon_obj != NULL)
			//end of code added 15/1/2005
			 {
				 object *mon_obj = curr_frame->mon_obj ;

				 if(mon_obj->mon.owner_thread_id !=
				    threads.at(c)->thread_id)
				 {
					 throw_exception("java/lang/IllegalMonitorStateException") ;
					 return ;
				 }

				 mon_obj->mon.decrement_count() ;
			 }

			 //code added 29/1/2005
			 //to preserve the value to be returned
			 op_st->pop() ;
			 //end of code added 29/1/2005

			 //pop the operand stack of the
			 //current frame
			 while(!op_st->empty())
			 {
				 delete op_st->top() ;
				 op_st->pop() ;
			 }

			 //pop the current frame from the
			 //currrent thread's stack
			 threads.at(c)->java_stack.pop() ;

			 //get the next instruction to 
			 //execute (this was stored when
			 //the frame was created)
			 threads.at(c)->pc = curr_frame->prev_pc_value ;

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << "dreturn" << "(" << curr_frame << ")" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 			   			 			 

			 //discard the current frame
			 delete curr_frame ;

			 //push the object reference to the 
			 //invoking frame
			 frame *invoking_frame = threads.at(c)->java_stack.top() ;
			 invoking_frame->operand_stack.push(d_val) ;
			 
			 break ;
		 }

		 case 57: //dstore: store double into local variable
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;

			 dstore(index) ;

			 threads.at(c)->pc += 2 ; //2 because the index of the
			 break ;		  //local variable is also stored
						  //in the method's code
		 }

		 case 71: //dstore_0: store double into local variable
		 {
			 dstore(0) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 72: //dstore_1: store double into local variable
		 {
			 dstore(1) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 73: //dstore_2: store double into local variable
		 {
			 dstore(2) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 74: //dstore_3: store double into local variable
		 {
			 dstore(3) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 103: //dsub: subtract double
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::DOUBLE ;
			 r->double_value = v1->double_value - v2->double_value ;


			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 89: //dup: duplicate the top operand stack value
		 {
			 value *v = op_st->top() ;
			 op_st->push(duplicate(v)) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 90: //dup_x1: duplicate the top operand stack value 
			  //and insert two values down
		 {
			 value *v1 = op_st->top() ; 
			 op_st->pop() ;
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 op_st->push(duplicate(v1)) ;
			 op_st->push(v2) ;
			 op_st->push(v1) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 91: //dup_x2: duplicate the top operand stack value 
			  //and insert two or three values down
		 {
			 value *v1 = op_st->top() ; 
			 op_st->pop() ;
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v3 = op_st->top() ;

			 if(IsCategory1(v1) == true &&
			    IsCategory2(v2) == true)
			 {
				 op_st->push(duplicate(v1)) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }
			 else if(IsCategory1(v1) == true &&
				 IsCategory1(v2) == true &&
				 IsCategory1(v3) == true)
			 {
				 op_st->pop() ;

				 op_st->push(duplicate(v1)) ;
				 op_st->push(v3) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;

			 }
			 else //leave things the way they are
			 {
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 92: //dup2: duplicate the top one or two operand stack values
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *v2 = op_st->top() ;

			 if(IsCategory2(v1) == true)
			 {
				 op_st->push(duplicate(v1)) ;
				 op_st->push(v1) ;
			 }
			 else if(IsCategory1(v1) == true &&
				 IsCategory1(v2) == true)
			 {
				 op_st->pop() ;

				 op_st->push(duplicate(v2)) ;
				 op_st->push(duplicate(v1)) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }
			 else //leave things the way they are
			 {
				 op_st->push(v1) ;
			 }

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 93: //duplicate the top one or two operand stack values
			  //and insert two or three values down
		 {

			 value *v1 = op_st->top() ; 
			 op_st->pop() ;
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v3 = op_st->top() ;

			 if(IsCategory2(v1) == true &&
			    IsCategory1(v2) == true)
			 {
				 op_st->push(duplicate(v1)) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }
			 else if(IsCategory1(v1) == true &&
				 IsCategory1(v2) == true &&
				 IsCategory1(v3) == true)
			 {
				 op_st->pop() ;

				 op_st->push(duplicate(v2)) ;
				 op_st->push(duplicate(v1)) ;
				 op_st->push(v3) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;

			 }
			 else //leave things the way they are
			 {
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 94: //dup2_x2: duplicate the top one or two operand stack
			  //values and insert two, three or four values down
		 {
			 value *v1 = op_st->top() ; 
			 op_st->pop() ;
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v3 = op_st->top() ;

			 if(IsCategory2(v1) == true &&
			    IsCategory2(v2) == true)
			 {
				 op_st->push(duplicate(v1)) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }
			 else if(IsCategory1(v1) == true &&
				 IsCategory1(v2) == true &&
				 IsCategory2(v3) == true)
			 {

				 op_st->pop() ;

				 op_st->push(duplicate(v2)) ;
				 op_st->push(duplicate(v1)) ;
				 op_st->push(v3) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }
			 else if(IsCategory2(v1) == true &&
				 IsCategory1(v2) == true &&
				 IsCategory1(v3) == true)
			 {

				 op_st->pop() ;

				 op_st->push(duplicate(v1)) ;
				 op_st->push(v3) ;
				 op_st->push(v2) ;
				 op_st->push(v1) ;
			 }
			 else
			 {
				 op_st->pop() ; //we need to pop v3 to get v4

				 value *v4 = op_st->top() ;

				 if(IsCategory1(v1) == true &&
				    IsCategory1(v2) == true &&
				    IsCategory1(v3) == true &&
				    IsCategory1(v4) == true)
				 {
					 op_st->pop() ;

					 op_st->push(duplicate(v2)) ;
					 op_st->push(duplicate(v1)) ;
					 op_st->push(v4) ;
					 op_st->push(v3) ;
					 op_st->push(v2) ;
					 op_st->push(v1) ;

				 }
				 else //we have to push v3 to revert to previous status
				 {
					 op_st->push(v3) ;
					 op_st->push(v2) ;
					 op_st->push(v1) ;
				 }
			 }
			 //TO DO: does this take care of all possibilities?

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 141: //f2d: convert float to double
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::DOUBLE ;
			 r->double_value = v->float_value ;
			 r->IsNaN = v->IsNaN ;
			 r->IsInfinity = v->IsInfinity ;
			 r->IsPositive = v->IsPositive ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 139: //f2i: convert float to int
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			assert(v->value_type == value::FLOAT) ;
#endif
//end of code added 4/2/2005

			 value *r = createValue() ;
			 r->value_type = value::INT ;

			 if(v->IsNaN == true)
				 r->i_value = 0 ;
			 else if(v->IsInfinity == true)
			 {
				 if(v->IsPositive == true)
					 r->i_value = INT_MAX ;
				 else
					 r->i_value = INT_MIN ;
			 }
			 else
			   //code changed 19/2/2005
			   //r->i_value = v->float_value ;
			   r->i_value = (int)v->float_value ;
			   //end of code adede 19/2/2005


			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 140: //f2l: convert float to long
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::LONG ;

			 if(v->IsNaN == true)
				 r->long_value = 0 ;
			 else if(v->IsInfinity == true)
			 {
				 if(v->IsPositive == true)
 #ifdef WIN32
					 r->long_value = _I64_MAX ;
 #else
					 r->long_value = LONG_LONG_MAX ;
 #endif
				 else
 #ifdef WIN32
					 r->long_value = _I64_MIN ;
 #else
					 r->long_value = LONG_LONG_MIN ;
 #endif
			 }
			 else
			   //code changed 19/2/2005
			   //r->long_value = v->float_value ;
			   r->long_value = (long)v->float_value ;
			   //end of code added 19/2/2005


			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 98: //fadd: add float
		 {
			 value *val2 = op_st->top() ;
			 op_st->pop() ;
			 value *val1 = op_st->top() ;
			 op_st->pop() ;

			 //code added 6/3/2005
#if DEBUG_LEVEL >= 3
			 assert(val2->value_type == value::FLOAT) ;
			 assert(val1->value_type == value::FLOAT) ;
#endif
			 //end of code added 6/3/2005

			 value *res = createValue() ;

			 res->value_type = value::FLOAT ;

			 if(val1->IsNaN == true || val2->IsNaN == true)
				 res->IsNaN = true ;
			 else
			 {
				 bool SameSign ;

				 if(val1->IsPositive == val2->IsPositive)
					 SameSign = true ;
				 else
					 SameSign = false ;

				 if(val1->IsInfinity == true && val1->IsInfinity == true)
				 {
					 if(SameSign == true)
					 {
						 res->IsInfinity = true ;
						 res->IsPositive = val1->IsPositive ; //we could use val2 here, too
					 }
					 else
						 res->IsNaN = true ;
				 }
				 else if(val1->IsInfinity == true)
				 {
					 res->IsInfinity = true ;
					 res->IsPositive = val1->IsPositive ;
				 }
				 else if(val2->IsInfinity == true)
				 {
					 res->IsInfinity = true ;
					 res->IsPositive = val2->IsPositive ;
				 }
				 else if(val1->float_value == 0 && val2->float_value == 0)
				 {
					 if(SameSign == true)
					 {
						 res->float_value = 0 ;
						 res->IsPositive = val1->IsPositive ; //we could use val2 here, too
					 }
					 else
					 {
						 res->float_value = 0 ;
						 res->IsPositive = true ;
					 }
				 }
				 else
					 res->float_value = val1->float_value + val2->float_value ;


			 }

			 op_st->push(res) ;


			 //code changed 9/2/2005
			 //delete val1 ;
			 //delete val2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 48: //faload: load float from array
		 {
			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 value *obj_val = createValue() ;

			 obj_val->value_type = value::FLOAT ;
			 obj_val->float_value = arrayref->ref_value->primitives.at(index->i_value)->float_value ;
			 obj_val->IsNaN = arrayref->ref_value->primitives.at(index->i_value)->IsNaN ;
			 obj_val->IsInfinity = arrayref->ref_value->primitives.at(index->i_value)->IsInfinity ;
			 obj_val->IsPositive = arrayref->ref_value->primitives.at(index->i_value)->IsPositive ;

			 op_st->push(obj_val) ;

			 //code changed 3/2/2005
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 81: //fastore: store into float array
		 {
			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete obj_val ;
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete obj_val ;
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 primitive_value *pv = createPrimitiveValue() ;

			 pv->value_type = primitive_value::FLOAT ;
			 pv->float_value = obj_val->float_value ;
			 pv->IsNaN = obj_val->IsNaN ;
			 pv->IsInfinity = obj_val->IsInfinity ;
			 pv->IsPositive = obj_val->IsPositive ;

			 //delete arrayref->ref_value->primitives.at(index->i_value) ;
			 arrayref->ref_value->primitives.at(index->i_value) = pv;

			 //code changed 3/2/2005
			 //delete obj_val ;
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 //we are not handling them separately
		 //because this is a simple implementation;
		 //no FP-strictness, value-sets, etc.
		 case 150: //fcmpg: compare float
		 case 149: //fcmpl: compare float
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			assert(v2->value_type == value::FLOAT) ;
			assert(v1->value_type == value::FLOAT) ;
#endif
//end of code added 4/2/2005

			 value *i_val = createValue() ;
			 i_val->value_type = value::INT ;

			 if(v1->float_value > v2->float_value)
				 i_val->i_value = 1 ;
			 else if(v1->float_value == v2->float_value)
				 i_val->i_value = 0 ;
			 else
				 i_val->i_value = -1 ;

			 op_st->push(i_val) ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 11: //fconst_0: push float (value 0.0)
		 {
			 value *f = createValue() ;
			 f->value_type = value::FLOAT ;
			 f->float_value = 0 ;
			 f->IsPositive = true ;

			 op_st->push(f) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 12: //fconst_1: push float (value 1.0)
		 {
			 value *f = createValue() ;
			 f->value_type = value::FLOAT ;
			 f->float_value = 1 ;
			 f->IsPositive = true ;

			 op_st->push(f) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 13: //fconst_0: push float (value 2.0)
		 {
			 value *f = createValue() ;
			 f->value_type = value::FLOAT ;
			 f->float_value = 2 ;
			 f->IsPositive = true ;

			 op_st->push(f) ;

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 110: //fdiv: divide float
		 {
			 value *val2 = op_st->top() ;
			 op_st->pop() ;
			 value *val1 = op_st->top() ;
			 op_st->pop() ;

			 value *res = createValue() ;

			 res->value_type = value::FLOAT ;

			 if(val1->IsNaN == true || val2->IsNaN == true)
				 res->IsNaN = true ;
			 else
			 {
				 if(val1->IsPositive == val2->IsPositive)
					 res->IsPositive = true ;
				 else
					 res->IsPositive = false ;

				 if(val1->IsInfinity == true && val2->IsInfinity == true)
					 res->IsNaN = true ;
				 else if(val1->IsInfinity == true && val2->IsInfinity == false)
					 res->IsInfinity = true ;
				 else if(val1->IsInfinity == false && val2->IsInfinity == true)
					 res->float_value = 0 ;
				 else if(val1->float_value == 0 && val2->float_value == 0)
					 res->IsNaN = true ;
				 else if(val1->float_value == 0)
					 res->float_value = 0 ;
				 else if(val2->float_value == 0)
					 res->IsInfinity = true ;
				 else
					 res->float_value = val1->float_value / val2->float_value ;

			 }


			 op_st->push(res) ;

			 //code changed 9/2/2005
			 //delete val1 ;
			 //delete val2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 23: //fload: load float from local variable
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;

			 fload(index) ;

			 threads.at(c)->pc += 2 ; //2 because the index of the
			 break ;		  //local variable is also stored
						  //in the method's code
		 }

		 case 34: //fload_0: load float from local variable
		 {
			 fload(0) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 35: //fload_1: load float from local variable
		 {
			 fload(1) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 36: //fload_2: load float from local variable
		 {
			 fload(2) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 37: //fload_3: load float from local variable
		 {
			 fload(3) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 106: //fmul: multiply float
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *v2 = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			assert(v1->value_type == value::FLOAT) ;
			assert(v2->value_type == value::FLOAT) ;
#endif
//end of code added 4/2/2005

			 value *r = createValue() ;
			 r->value_type = value::FLOAT ;

			 if(v1->IsNaN == true || v2->IsNaN == true)
				 r->IsNaN = true ;
			 else
			 {
				 if(v1->IsPositive == v2->IsPositive == true)
					 r->IsPositive = true ;
				 else
					 r->IsPositive = false ;

				 if(v1->IsInfinity == true && v2->double_value == 0 ||
				    v2->IsInfinity == true && v1->double_value == 0)
					 r->IsNaN = true ;
				 else if(v1->IsInfinity == true || v2->IsInfinity == true)
					 r->IsInfinity = true ;
				 else
					 r->float_value = v1->float_value * v2->float_value ;

			 }

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 118: //fneg: negate float
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::FLOAT ;

			 if(v->IsNaN == true)
				 r->IsNaN = true ;
			 else if(v->IsInfinity == true)
			 {
				 r->IsInfinity = true ;
				 r->IsPositive = !(v->IsPositive) ;
			 }
			 else if(v->float_value == 0)
			 {
				 r->float_value = 0 ;
				 r->IsPositive = !(v->IsPositive) ;
			 }
			 else
				 r->float_value = (-1.0) * v->float_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 114: //frem: remainder float
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::FLOAT ;

			 if(v1->IsNaN == true || v2->IsNaN == true)
				 r->IsNaN = true ;
			 else
			 {
				 r->IsPositive = v1->IsPositive ;

				 if(v1->IsInfinity == true || v2->float_value == 0)
					 r->IsNaN = true ;
				 else if(v1->IsInfinity == false && v2->IsInfinity == true)
					 r->float_value = v1->float_value ;
				 else if(v1->float_value == 0 && v2->IsInfinity == false)
					 r->float_value = v1->float_value ;
				 else
				 {
				   //code changed 19/2/2005
				   //int q = v1->float_value / v2->float_value ;
				   int q = (int)(v1->float_value / v2->float_value) ;
				   //end of code changed 19/2/2005
				   r->float_value = v1->float_value - (v2->float_value * q) ;
				 }
			 }

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 174: //freturn: return float from method
		 {
			 value *f_val = op_st->top() ;

			 //if the method is a synchronised method,
			 //the object's monitor should be owned
			 //by the current thread

			//code added 15/1/2005
			//if(current_method->blnSynchronized == true)			
			if(current_method->blnSynchronized == true && curr_frame->mon_obj != NULL)
			//end of code added 15/1/2005
			 {
				 object *mon_obj = curr_frame->mon_obj ;

				 if(mon_obj->mon.owner_thread_id !=
				    threads.at(c)->thread_id)
				 {
					 throw_exception("java/lang/IllegalMonitorStateException") ;
					 return ;
				 }

				 mon_obj->mon.decrement_count() ;
			 }

			 //code added 29/1/2005
			 //to preserve the value to be returned
			 op_st->pop() ;
			 //end of code added 29/1/2005

			 //pop the operand stack of the
			 //current frame
			 while(!op_st->empty())
			 {
				 delete op_st->top() ;
				 op_st->pop() ;
			 }

			 //pop the current frame from the
			 //currrent thread's stack
			 threads.at(c)->java_stack.pop() ;

			 //get the next instruction to 
			 //execute (this was stored when
			 //the frame was created)
			 threads.at(c)->pc = curr_frame->prev_pc_value ;

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << "freturn" << "(" << curr_frame << ")" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 			   			 			 
			 
			 //discard the current frame
			 delete curr_frame ;

			 //push the object reference to the 
			 //invoking frame
			 frame *invoking_frame = threads.at(c)->java_stack.top() ;
			 invoking_frame->operand_stack.push(f_val) ;

			 
			 break ;
		 }

		 case 56: //fstore: store float into local variable
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;

			 fstore(index) ;

			 threads.at(c)->pc += 2 ; //2 because the index of the
			 break ;		  //local variable is also stored
						  //in the method's code
		 }

		 case 67: //fstore_0: store float into local variable
		 {
			 fstore(0) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 68: //fstore_1: store float into local variable
		 {
			 fstore(1) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 69: //fstore_2: store float into local variable
		 {
			 fstore(2) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 70: //fstore_3: store float into local variable
		 {
			 fstore(3) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 102: //fsub: subtract float
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::FLOAT ;
			 r->float_value = v1->float_value - v2->float_value ;

			 op_st->push(r) ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 180: //getfield: fetch field from object
		 {
			 int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			 int rcp_index = index1 * 256 + index2 ;

			 value *obj = op_st->top() ;
			 op_st->pop() ;
			 
#if DEBUG_LEVEL >= 2
			 cout << "getfield(): " << curr_frame->rcp->at(rcp_index-1)->field_descriptor << endl << flush ;
			 cout << "getfield(): " << curr_frame->rcp->at(rcp_index-1)->field_name << " " << obj->ref_value << endl << flush ;
			 obj->ref_value->display() ;
#endif
			 
			 if(obj->ref_value->blnNull == true)
			 {
				 delete obj ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }

			 field *f ;

			 try
			 {
				 f = resolve_field(this_class,
						   curr_frame->rcp->at(rcp_index-1)->field_parent_class_name,			
						   curr_frame->rcp->at(rcp_index-1)->field_name,
						   curr_frame->rcp->at(rcp_index-1)->field_descriptor,
						   true) ;
			 }
			 catch(LinkageError)
			 {
				 delete obj ;
				 throw_exception("java/lang/LinkageError") ;
				 return ;
			 }
			 catch(NoClassDefFound)
			 {
				 delete obj ;
				 throw_exception("java/lang/NoClassDefFoundError") ;
				 return ;
			 }
			 catch(FormatError)
			 {
				 delete obj ;
				 throw_exception("java/lang/FormatError") ;
				 return ;
			 }
			 catch(ClassCircularityError)
			 {
				 delete obj ;
				 throw_exception("java/lang/ClassCircularityError") ;
				 return ;
			 }
			 catch(IncompatibleClassChangeError)
			 {
				 delete obj ;
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }
			 catch(IllegalAccessError)
			 {
				 delete obj ;
				 throw_exception("java/lang/IllegalAccessError") ;
				 return ;
			 }
			 catch(NoSuchFieldError)
			 {
				 delete obj ;
				 throw_exception("java/lang/NoSuchFieldError") ;
				 return ;
			 }

//code added 25/11/2004
#if DEBUG_LEVEL >= 3
			Logger::Log(curr_frame->rcp->at(rcp_index-1)->field_parent_class_name + ":" + f->name) ;
#endif
//end of code added 25/11/2004

			 if(f->blnStatic == true)
			 {
				 delete obj ;
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }

//code added 20/2/2005
#if DEBUG_LEVEL >= 3
			 assert(obj->ref_value->fields.size() > 0) ;
#endif
//end of code added 20/2/2005

			 value *v = obj->ref_value->fields[curr_frame->rcp->at(rcp_index-1)->field_name] ;
			 
			//code added 29/11/2004
			//to handle inner class objects
			//which need to access the owning object's fields
			
			//if(v == NULL)
			//{
			//	object *owning_obj = obj->ref_value->fields["this"]->ref_value ;
			//	v = owning_obj->fields[curr_frame->rcp->at(rcp_index-1)->field_name] ;
			//}
			//end of code added 29/11/2004

			//code added 14/1/2005
			//if(v == NULL)
			//{
			//	cout << "try our luck in owning object..." << endl ;
			//	assert(obj->ref_value != NULL) ;
			//	object *owning_obj = obj->ref_value->parent_object ;
			//	assert(owning_obj != NULL) ;
			//	cout << "---" << owning_obj->type << "---" << endl << flush ;
			//	v = owning_obj->fields[curr_frame->rcp->at(rcp_index-1)->field_name] ;				
			//}
			//end of code added 14/1/2005

//code added 24/11/2004
#if DEBUG_LEVEL >= 3

			assert(v != NULL) ;
			if(v->value_type == value::REFERENCE)
				assert(v->ref_value != NULL) ;
#endif
//end of code added 24/11/2004

//code added 28/7/2005
#if DEBUG_LEVEL >= 3
			 cout << "getfield(): field details ~~~~~" << endl << flush ;
			 //v->display() ;
			 cout << "getfield(): end of field details ~~~~~" << endl << flush ;
#endif			 
//end of code added 28/7/2005			
			
			 op_st->push(v) ;

			 //code changed 3/2/2005
			 //delete obj ;
			 //end of code changed 3/2/2005
			
			 threads.at(c)->pc+= 3 ;
			 break ;
		 }

		 case 178: //getstatic: get static field from class
		 {
			 int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			 int rcp_index = index1 * 256 + index2 ;

			 field *f ;

			 try
			 {
				 f = resolve_field(this_class,
						   curr_frame->rcp->at(rcp_index-1)->field_parent_class_name,			
						   curr_frame->rcp->at(rcp_index-1)->field_name,
						   curr_frame->rcp->at(rcp_index-1)->field_descriptor,
						   true) ;
			 }
			 catch(LinkageError)
			 {
				 throw_exception("java/lang/LinkageError") ;
				 return ;
			 }
			 catch(NoClassDefFound)
			 {
				 throw_exception("java/lang/NoClassDefFoundError") ;
				 return ;
			 }
			 catch(FormatError)
			 {
				 throw_exception("java/lang/FormatError") ;
				 return ;
			 }
			 catch(ClassCircularityError)
			 {
				 throw_exception("java/lang/ClassCircularityError") ;
				 return ;
			 }
			 catch(IncompatibleClassChangeError)
			 {
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }
			 catch(IllegalAccessError)
			 {
				 throw_exception("java/lang/IllegalAccessError") ;
				 return ;
			 }
			 catch(NoSuchFieldError)
			 {
				 throw_exception("java/lang/NoSuchFieldError") ;
				 return ;
			 }

			 if(f->blnStatic == false)
			 {
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }

			 if(f->parent_class->blnInitialised == false)
			 {
				 InitialiseClass(f->parent_class) ;
				 break ;
			 }

			 static_value *sv = f->parent_class->static_fields[curr_frame->rcp->at(rcp_index-1)->field_name] ;

			 value *v = createValue() ;


			 //code changed 11/2/2005
			 //v->value_type = value::REFERENCE ;
			 v->value_type = sv->value_type ;
			 //end of code changed 11/2/2005
			 
			 v->ref_value = sv->ref_value ;

			 //code added 11/2/2005
			 v->IsNaN = sv->IsNaN ;
			 v->IsInfinity = sv->IsInfinity ;
			 v->IsPositive = sv->IsPositive ;

			 v->bool_value = sv->bool_value ;
			 v->i_value = sv->i_value ;
			 v->long_value = sv->long_value ;
			 v->float_value = sv->float_value ;
			 v->double_value = sv->double_value ;
			 v->ret_add_value = sv->ret_add_value ;
			 //end of code added 11/2/2005

			 //don't think this is required,
			 //as sv->ref_value will already have its type
			 //v->ref_value->type = sv->type ;

			 op_st->push(v) ;

			 threads.at(c)->pc+= 3 ;
			 break ;
		 }

		 case 167: //goto: branch always
		 {
			 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

			 short g = (branchbyte1 << 8) | branchbyte2 ;

			 threads.at(c)->pc += g ;
			 break ;
		 }

		 case 200: //goto_w: branch always (wide index)
		 {
			 int bb1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int bb2 = current_method->code.byte_code.at(prog_counter+2) ;
			 int bb3 = current_method->code.byte_code.at(prog_counter+3) ;
			 int bb4 = current_method->code.byte_code.at(prog_counter+4) ;


			 int g = (bb1 << 24) | (bb2 << 16) | (bb3 << 8) | bb4 ;

			 threads.at(c)->pc += g ;
			 break ;
		 }

		 case 145: //i2b: convert int to byte
		 {
			 //what we are doing is redundant;
			 //since we are storing byte, char,
			 //short and int values in the same
			 //variable (i_value), we can just
			 //read the operand stack's top value
			 //and change the value's type from
			 //INT to BYTE
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::_BYTE ;
			 r->i_value = v->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;	
			 //end of code changed 9/2/2005
			 
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 146: //i2c: convert int to char
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::CHAR ;
			 r->i_value = v->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;	
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 135: //i2d: convert int to double
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::DOUBLE ;
			 r->double_value = v->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;	
			 //end of code changed 9/2/2005
			 
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 134: //i2f: convert int to float
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			assert(v->value_type == value::INT) ;
#endif
//end of code added 4/2/2005

			 value *r = createValue() ;
			 r->value_type = value::FLOAT ;
			 r->float_value = v->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;	
			 //end of code changed 9/2/2005
			 
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 133: //i2l: convert int to long
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::LONG ;
			 r->long_value = v->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;	
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 147: //i2s: convert int to short
		 {
			 value *v = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::SHORT ;
			 r->i_value = v->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v ;	
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 96: //iadd: add int
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 6/3/2005
#if DEBUG_LEVEL >= 3
			 //code changed 26/3/2005
			 //looks like iadd is being used with chars
			 //assert(v2->value_type == value::INT) ;
			 //assert(v1->value_type == value::INT) ;
			 //end of code changed 26/3/2005
#endif
//end of code added 6/3/2005

			 value *r = createValue() ;
			 r->value_type = value::INT ;
			 r->i_value = v1->i_value + v2->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 46: //iaload: load int from array
		 {
			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 value *obj_val = createValue() ;

			 obj_val->value_type = value::INT ;
			 obj_val->i_value = arrayref->ref_value->primitives.at(index->i_value)->i_value ;

			 op_st->push(obj_val) ;

			 //code changed 3/2/2005
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 126: //iand: boolean AND int
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue();
			 r->value_type = value::INT ;
			 r->i_value = v1->i_value & v2->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 79: //iastore: store into int array
		 {
			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

			 value *index = op_st->top() ;
			 op_st->pop() ;

			 value *arrayref = op_st->top() ;
			 if(arrayref->ref_value->blnNull == true)
			 {
				 delete obj_val ;
				 delete index ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }
			 op_st->pop() ;

			 if(index->i_value < 0 ||
			    //code changed 6/3/2005
			    //index->i_value >= arrayref->ref_value->items.size())
			    index->i_value >= arrayref->ref_value->primitives.size())
			    //end of code changed 6/3/2005
			 {
				 delete obj_val ;
				 delete index ;
				 delete arrayref ;
				 throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				 return ;
			 }

			 primitive_value *pv = createPrimitiveValue() ;

			 pv->value_type = primitive_value::INT ;
			 pv->i_value = obj_val->i_value ;

			 //delete arrayref->ref_value->primitives.at(index->i_value) ;
			 arrayref->ref_value->primitives.at(index->i_value) = pv;

			 //code changed 3/2/2005
			 //delete obj_val ;
			 //delete index ;
			 //delete arrayref ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 2: //iconst_m1: push int const (-1)
		 {
			 value *v = createValue() ;
			 v->value_type = value::INT ;
			 v->i_value = -1 ;

			 op_st->push(v) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 3: //iconst_0: push int const (0)
		 {
			 value *v = createValue() ;
			 v->value_type = value::INT ;
			 v->i_value = 0 ;

			 op_st->push(v) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 4: //iconst_1: push int const (1)
		 {
			 value *v = createValue() ;
			 v->value_type = value::INT ;
			 v->i_value = 1 ;

			 op_st->push(v) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 5: //iconst_2: push int const (2)
		 {
			 value *v = createValue() ;
			 v->value_type = value::INT ;
			 v->i_value = 2 ;

			 op_st->push(v) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 6: //iconst_3: push int const (3)
		 {
			 value *v = createValue() ;
			 v->value_type = value::INT ;
			 v->i_value = 3 ;

			 op_st->push(v) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 7: //iconst_4: push int const (4)
		 {
			 value *v = createValue() ;
			 v->value_type = value::INT ;
			 v->i_value = 4 ;

			 op_st->push(v) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 8: //iconst_5: push int const (5)
		 {
			 value *v = createValue() ;
			 v->value_type = value::INT ;
			 v->i_value = 5 ;

			 op_st->push(v) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 108: //idiv: divide int
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 if(v2->i_value == 0)
			 {
				 //delete v1 ;
				 //delete v2 ;
				 throw_exception("java/lang/ArithmeticException") ;
				 return ;
			 }

			 value *r = createValue() ;
			 r->value_type = value::INT ;
			 r->i_value = v1->i_value / v2->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 165: //if_acmpeq: branch if reference comparison succeeds (eq)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->ref_value == v2->ref_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 166: //if_acmpne: branch if reference comparison succeeds (ne)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->ref_value != v2->ref_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 159: //if_icmpeq: branch if int comparison succeeds (eq)
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			//code changed 26/3/2005
			//see comment in iadd
			//assert(v2->value_type == value::INT) ;
			//assert(v1->value_type == value::INT) ;
			//end of code changed 26/3/2005
#endif
//end of code added 4/2/2005

			 if(v1->i_value == v2->i_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 160: //if_icmpne: branch if int comparison succeeds (ne)
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->i_value != v2->i_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 161: //if_icmplt: branch if int comparison succeeds (lt)
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->i_value < v2->i_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 162: //if_icmpge: branch if int comparison succeeds (ge)
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->i_value >= v2->i_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 163: //if_icmpgt: branch if int comparison succeeds (gt)
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->i_value > v2->i_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 164: //if_icmple: branch if int comparison succeeds (le)
		 {
			 value *v2 = op_st->top() ;
			 op_st->pop() ;
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 5/2/2005 (Happy Birthday!)
#if DEBUG_LEVEL >= 3
			assert(v2->value_type == value::INT) ;
			assert(v1->value_type == value::INT) ;
#endif
//end of code adde 5/2/2005

			 if(v1->i_value <= v2->i_value)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 5/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 5/2/2005
			 break ;
		 }

		 case 153: //ifeq: branch if int comparison with zero succeeds (eq)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 6/8/2005
#if DEBUG_LEVEL >= 2
			 cout << "ifeq(): " << v1->i_value << endl << flush ;
#endif			 
//end of code added 6/8/2005			 
			 
			 if(v1->i_value == 0)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 154: //ifne: branch if int comparison with zero succeeds (ne)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			 //assert(v1->value_type == value::INT) ;
#endif
//end of code added 4/2/2005

			 if(v1->i_value != 0)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 155: //iflt: branch if int comparison with zero succeeds (lt)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->i_value < 0)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 156: //ifge: branch if int comparison with zero succeeds (ge)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code adde 4/2/2005
#if DEBUG_LEVEL >= 3
			assert(v1 != NULL) ;
			assert(v1->value_type == value::INT) ;
#endif
//end of code added 4/2/2005

			 if(v1->i_value >= 0)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 157: //ifgt: branch if int comparison with zero succeeds (gt)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			 assert(v1->value_type == value::INT) ;
#endif
//end of code added 4/2/2005

			 if(v1->i_value > 0)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;


			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 158: //ifle: branch if int comparison with zero succeeds (le)
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 if(v1->i_value <= 0)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 199: //ifnonnull: branch if reference not null
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
			assert(v1 != NULL) ;
			assert(v1->value_type == value::REFERENCE) ;
			assert(v1->ref_value != NULL) ;
			//v1->ref_value->display() ;
#endif
//end of code added 4/2/2005

			 //code changed 4/2/2005
			 //WTF was I thinking??
			 //if(v1->ref_value != NULL)
			 if(v1->ref_value->blnNull == false)
			 //end of code changed 4/2/2005
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 198: //ifnull: branch if reference null
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;
//code added 12/2/2005
#if DEBUG_LEVEL >= 3
			 assert(v1->value_type == value::REFERENCE) ;
#endif
//end of code added 12/2/2005

			 if(v1->ref_value->blnNull == true)
			 {
				 int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
				 int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

				 short g = (branchbyte1 << 8) | branchbyte2 ;

				 threads.at(c)->pc += g ;
			 }
			 else
				 threads.at(c)->pc += 3 ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005
			 
			 break ;
		 }

		 case 132: //iinc: increment local variable by constant
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;
			 signed char constant = current_method->code.byte_code.at(prog_counter+2) ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
			 assert(index >= 0 && index < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

			 LocalVariable *lv = curr_frame->local_variables.at(index) ;

			 lv->i_value += constant ;

			 threads.at(c)->pc += 3 ;
			 break ;
		 }

		 case 21: //iload: load int from local variable
		 {
			 int index = current_method->code.byte_code.at(prog_counter+1) ;

			 iload(index) ;

			 threads.at(c)->pc += 2 ; //2 because the index of the
			 break ;		  //local variable is also stored
						  //in the method's code
		 }

		 case 26: //iload_0: load int from local variable
		 {
			 iload(0) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 27: //iload_1: load int from local variable
		 {
			 iload(1) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 28: //iload_2: load int from local variable
		 {
			 iload(2) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 29: //iload_3: load int from local variable
		 {
			 iload(3) ;
			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 104: //imul: multiply int
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;
			 value *v2 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::INT ;
			 r->i_value = v1->i_value * v2->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //delete v2 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 116: //ineg: negate int
		 {
			 value *v1 = op_st->top() ;
			 op_st->pop() ;

			 value *r = createValue() ;
			 r->value_type = value::INT ;
			 r->i_value = (-1) * v1->i_value ;

			 op_st->push(r) ;

			 //code changed 9/2/2005
			 //delete v1 ;
			 //end of code changed 9/2/2005

			 threads.at(c)->pc++ ;
			 break ;
		 }

		 case 193: //instanceof: determine if object is of given type
		 {
			 int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			 int g = index1 * 256 + index2 ;

			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

//code added 13/2/2005
#if DEBUG_LEVEL >= 3
			 assert(obj_val->value_type == value::REFERENCE) ;
			 assert(obj_val->ref_value != NULL) ;
			 assert(curr_frame->rcp->at(g-1)->valid_flag == true) ;
#endif
//end of code added 13/2/005

			 Class *T;

			 try
			 {
				 if(curr_frame->rcp->at(g-1)->blnResolved == true)
					 T = curr_frame->rcp->at(g-1)->resolved_class ;
				 else
				 {
					 T = resolve_class(this_class,
							   curr_frame->rcp->at(g-1)->class_interface_name) ;

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
					 assert(T != NULL) ;
#endif
//end of code added 28/2/2005

					 curr_frame->rcp->at(g-1)->blnResolved = true ;
					 curr_frame->rcp->at(g-1)->resolved_class = T ;
				 }
			 }
			 catch(LinkageError)
			 {
				 delete obj_val ;
				 throw_exception("java/lang/LinkageError") ;
				 return ;
			 }
			 catch(NoClassDefFound)
			 {
				 delete obj_val ;
				 throw_exception("java/lang/NoClassDefFoundError") ;
				 return ;
			 }
			 catch(FormatError)
			 {
				 delete obj_val ;
				 throw_exception("java/lang/FormatError") ;
				 return ;
			 }
			 catch(ClassCircularityError)
			 {
				 delete obj_val ;
				 throw_exception("java/lang/ClassCircularityError") ;
				 return ;
			 }
			 catch(IncompatibleClassChangeError)
			 {
				 delete obj_val ;
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }
			 catch(IllegalAccessError)
			 {
				 delete obj_val ;
				 throw_exception("java/lang/IllegalAccessError") ;
				 return ;
			 }

//code added 31/7/2005
#if DEBUG_LEVEL >= 2
			 cout << "instanceof(): " << obj_val->ref_value->type->name << endl << flush ;
			 cout << "instanceof(): " << T->name << endl << flush ;
#endif			 
//end of code added 31/7/2005			 
			 value *r = createValue() ;
			 r->value_type = value::INT ;

			 //we have nested ifs instead of a single 
			 //if with an && because if ref_value is NULL,
			 //second condition will abort
			 //code changed 13/2/2005
			 //if(obj_val->ref_value != NULL)
			 if(obj_val->ref_value->blnNull != true)			 
			 //end of code changed 13/2/2005
			 {
				 if(IsAssignmentCompatible(obj_val->ref_value->type, T) == true)
					 r->i_value = 1 ;
				 else
					 r->i_value = 0 ;
			 }
			 else
				 r->i_value = 0 ;

//code added 31/7/2005
#if DEBUG_LEVEL >= 2
			 cout << "instanceof(): result is " << r->i_value << endl << flush ;
#endif
//end of code added 31/7/2005
			 
			 op_st->push(r) ;

			 //code changed 3/2/2005
			 //delete obj_val ;
			 //end of code changed 3/2/2005

			 threads.at(c)->pc+= 3 ;
			 break ;
		 }

		 case 185: //invokeinterface: invoke interface method
		 {
			 int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			 int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			 int rcp_index = index1 * 256 + index2 ;

			 string method_parent_class_name = curr_frame->rcp->at(rcp_index-1)->method_parent_class_name ;
			 string method_name = curr_frame->rcp->at(rcp_index-1)->method_name ;
			 string method_descriptor = curr_frame->rcp->at(rcp_index-1)->method_descriptor ;


			 //interface which declares this method
			 Class *intf ;

			 try
			 {
				 intf = resolve_class(this_class,method_parent_class_name) ;

				 //code added 28/2/2005
#if DEBUG_LEVEL >= 3
				 assert(intf != NULL) ;
#endif
				 //end of code added 28/2/2005

				 //we are using a dummy to 
				 //store the resolved method
				 //because the method to be
				 //invoked will be got from objectref
				 method *dum ;

				 if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
					 dum = curr_frame->rcp->at(rcp_index-1)->resolved_method ;
				 else
				 {
					 dum = resolve_interface_method(this_class,
									method_parent_class_name,
									method_name,
									method_descriptor) ;

					//code added 28/2/2005
#if DEBUG_LEVEL >= 3
					assert(dum != NULL) ;
#endif
					//end of code added 28/2/2005

					 curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					 curr_frame->rcp->at(rcp_index-1)->resolved_method = dum ;
				 }
			 }
			 catch(LinkageError)
			 {
				 throw_exception("java/lang/LinkageError") ;
				 return ;
			 }
			 catch(NoClassDefFound)
			 {
				 throw_exception("java/lang/NoClassDefFoundError") ;
				 return ;
			 }
			 catch(FormatError)
			 {
				 throw_exception("java/lang/FormatError") ;
				 return ;
			 }
			 catch(ClassCircularityError)
			 {
				 throw_exception("java/lang/ClassCircularityError") ;
				 return ;
			 }
			 catch(IncompatibleClassChangeError)
			 {
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }
			 catch(IllegalAccessError)
			 {
				 throw_exception("java/lang/IllegalAccessError") ;
				 return ;
			 }
			 catch(NoSuchMethodError)
			 {
				 throw_exception("java/lang/NoSuchMethodError") ;
				 return ;
			 }

			 //int count = current_method->code.byte_code.at(prog_counter+3) ;

			 //this code is needed to determine the number
			 //of parameters of the method (by parsing the
			 //descriptor); we need this number to pop
			 //the correct number of values from the
			 //operand stack

			 string parameter_string = method_descriptor.substr(0,
								    method_descriptor.find(")") + 1) ;
			 vector<string> *parameters = util::parse_params(parameter_string.substr(1,parameter_string.length()-2)) ;

			 int param_count = parameters->size() ;


			 delete parameters ;

			 //temporary stack used to transfer
			 //values to the new frame's local variables
			 //array
			 stack<value *> values ;

			 int i=0 ;

//code added 15/11/2004
#if DEBUG_LEVEL >= 3
			if(param_count > 0)
				assert(!op_st->empty());
#endif
//end of code added 15/11/2004

			 while(i < param_count)
			 {
				 value *v = op_st->top() ;
				 op_st->pop() ;

				 values.push(v) ;
				 i++ ;
			 }

			 value *obj_val = op_st->top() ;
			 op_st->pop() ;

			 if(obj_val->ref_value->blnNull == true)
			 {
				 EmptyValueStack(&values) ;
				 delete obj_val ;
				 throw_exception("java/lang/NullPointerException") ;
				 return ;
			 }

			 //C is the class of objref
			 Class *C = obj_val->ref_value->type ;

			 if(ImplementsInterface(C, intf) == false)
			 {
				 EmptyValueStack(&values) ;
				 delete obj_val ;
				 throw_exception("java/lang/IncompatibleClassChangeError") ;
				 return ;
			 }

			 method *m ;

			 try
			 {
				 m = GetMethod(C,method_name,method_descriptor) ;
			 }
			 catch(AbstractMethodError)
			 {
				 EmptyValueStack(&values) ;
				 delete obj_val ;
				 throw_exception("java/lang/AbstractMethodError") ;
				 return ;
			 }

			 if(m->blnPublic == false)
			 {
				 EmptyValueStack(&values) ;
				 delete obj_val ;
				 throw_exception("java/lang/IllegalAccessError") ;
				 return ;
			 }

			 if(m->blnAbstract == true)
			 {
				 EmptyValueStack(&values) ;
				 delete obj_val ;
				 throw_exception("java/lang/AbstractMethodError") ;
				 return ;
			 }

			 if(m->blnSynchronized == true)
			 {
				 //acquire/reenter monitor associated
				 //with objectref
				 if(obj_val->ref_value->mon.blnFree == true)
				 {
					 obj_val->ref_value->mon.owner_thread_id = threads.at(c)->thread_id ;
					 obj_val->ref_value->mon.increment_count() ;
					 threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;
				 }
				 else
				 {
					 if(obj_val->ref_value->mon.owner_thread_id == threads.at(c)->thread_id)
					 {
						 obj_val->ref_value->mon.increment_count() ;
						 threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;                        
					 }
					 else
					 {
						 if(curr_thread != threads.size()-1)
							 curr_thread++ ;
						 else
							 curr_thread = 0 ;
					 }
				 }
			 }
			 
			 if(m->blnNative == true)
			 {

			   if(m->blnBound == false)
			     if(bind_native_method(m) == false)
			       return ;

                           //code changed 3/8/2005
                           //execute_native_method(m, obj_val, values) ;
			   execute_native_method(m, obj_val, values, false) ;
			   //end of code chnaged 3/8/2005

			   threads.at(c)->pc += 3 ;

			 }
			 else
			 {
			   //code changed 25/7/2005
			   //frame *fm = new frame() ;
			   //fm->rcp = &(m->type->rcp) ;
			   //fm->curr_method = m ;
			   frame *fm = new frame(m) ;
			   //end of code changed 25/7/2005
#if DEBUG_LEVEL >= 2
			   cout << fm << ":" << fm->curr_method->type->name << ":" ;
			   cout << fm->curr_method->name << endl ;
#endif
			   fm->local_variables.reserve(m->max_locals + 1) ;

			   fm->prev_pc_value = threads.at(c)->pc+4 ;

			   fm->local_variables.push_back(create_local_variable(obj_val)) ;

			   //code changed 3/2/2005
			   //delete obj_val ;
			   //end of code changed 3/2/2005

			   while(!values.empty())
			   {
			     value *v = values.top() ;
			     values.pop() ;
			     if(IsCategory1(v))
    				fm->local_variables.push_back(create_local_variable(v)) ;
			     else
			     {
		    		fm->local_variables.push_back(create_local_variable(v)) ;
			    	LocalVariable *dummy = createLocalVariable() ;
				dummy->valid_flag = false;
				fm->local_variables.push_back(dummy) ;
			     }

			     //code changed 3/2/2005
			     //delete v ;
			     //end of code changed 3/2/2005

			    }

			   for(int it=fm->local_variables.size(); it < m->max_locals+1; it++)
			     fm->local_variables.push_back(NULL) ; 

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << curr_frame << ": " << C->name << "->" << m->name << "()" << "(" << fm << "(" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 			   
			   
			   threads.at(c)->java_stack.push(fm) ;
			   threads.at(c)->pc = 0 ;
			 }
			 break ;
		}

		case 183: //invokespecial: invoke instance method; special handling 
			  //for superclass, private and instance initialization 
			  //method invocations
		{
			int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = index1 * 256 + index2 ;

			string method_parent_class_name = curr_frame->rcp->at(rcp_index-1)->method_parent_class_name ;
			string method_name = curr_frame->rcp->at(rcp_index-1)->method_name ;
			string method_descriptor = curr_frame->rcp->at(rcp_index-1)->method_descriptor ;

			method *resolved_method ;


			Class *cl ;

			try
			{
				cl = resolve_class(this_class,method_parent_class_name) ;

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
				assert(cl != NULL) ;
#endif
//end of code added 28/2/2005

				if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
				{
#if DEBUG_LEVEL >= 3
				        assert(curr_frame->rcp->at(rcp_index-1)->resolved_method != NULL) ;
				        assert(curr_frame->rcp->at(rcp_index-1)->resolved_method->type != NULL) ;
#endif
					resolved_method = curr_frame->rcp->at(rcp_index-1)->resolved_method ;
				}
				else
				{
					resolved_method = resolve_class_method(this_class,
						   		       method_parent_class_name,
								       method_name,
						         	       method_descriptor) ;
					//code added 28/2/2005
#if DEBUG_LEVEL >= 3
					assert(resolved_method != NULL) ;
#endif
					//end of code added 28/2/2005


					curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					curr_frame->rcp->at(rcp_index-1)->resolved_method = resolved_method ;
				}
			}
			catch(LinkageError)
			{
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;

			}
			catch(NoSuchMethodError)
			{
				throw_exception("java/lang/NoSuchMethodError") ;
				return ;
			}

//code added 23/11/2004
#if DEBUG_LEVEL >= 3
                        //chokes for some reason if this is uncommented
			//Logger::Log(cl->name + "::" + resolved_method->name) ;
#endif
//end of code added 23/11/2004

			if(resolved_method->name == "<init>" &&
			   resolved_method->type != cl)
			{
				throw_exception("java/lang/NoSuchMethodError") ;
				return ;
			}

			if(resolved_method->blnStatic == true)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}


			method *method_to_be_invoked ;

			if(!(this_class->blnSuper == true &&
			     IsSuperClass(resolved_method->type,this_class) == true &&
			     resolved_method->name != "<init>"))
				method_to_be_invoked = resolved_method ;
			else
			{
				Class *C = this_class->super_class ;

				try
				{
					method_to_be_invoked = GetMethod(C,method_name,method_descriptor) ;
				}
				catch(AbstractMethodError)
				{
					throw_exception("java/lang/AbstractMethodError") ;
					return ;
				}

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
				assert(method_to_be_invoked != NULL) ;
#endif
//end of code added 28/2/2005
				
			}

			if(method_to_be_invoked->blnAbstract == true)
			{
				throw_exception("java/lang/AbstractMethodError") ;
				return ;
			}

			//this code is needed to determine the number
			//of parameters of the method (by parsing the
			//descriptor); we need this number to pop
			//the correct number of values from the
			//operand stack

			string parameter_string = method_descriptor.substr(0,
								   method_descriptor.find(")") + 1) ;
			vector<string> *parameters = util::parse_params(parameter_string.substr(1,parameter_string.length()-2)) ;

			int param_count = parameters->size() ;

			delete parameters ;

			//temporary stack used to transfer
			//values to the new frame's local variables
			//array
			stack<value *> values ;

			int i=0 ;

//code added 15/11/2004
#if DEBUG_LEVEL >= 3
			if(param_count > 0)
				assert(!op_st->empty());
#endif
//end of code added 15/11/2004

			while(i < param_count)
			{
				value *v = op_st->top() ;
				op_st->pop() ;

				values.push(v) ;
				i++ ;
			}

			value *obj_val = op_st->top() ;
			op_st->pop() ;

			if(obj_val->ref_value->blnNull == true)
			{
				EmptyValueStack(&values) ;
				delete obj_val ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}

			if(method_to_be_invoked->blnSynchronized == true)
			{
				//acquire/reenter monitor associated
				//with objectref
				if(obj_val->ref_value->mon.blnFree == true)
				{
					obj_val->ref_value->mon.owner_thread_id = threads.at(c)->thread_id ;
					obj_val->ref_value->mon.increment_count() ;
					threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;
				}
				else
				{
					if(obj_val->ref_value->mon.owner_thread_id == threads.at(c)->thread_id)
					{
						obj_val->ref_value->mon.increment_count() ;
						threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;
					}
					else
					{
						if(curr_thread != threads.size()-1)
							curr_thread++ ;
						else
							curr_thread = 0 ;
					}
				}
			}
			
			if(method_to_be_invoked->blnNative == true)
			{
				
			  if(method_to_be_invoked->blnBound == false)
			    if(bind_native_method(method_to_be_invoked) == false)
			      return ;

//code added 24/7/2005
#if DEBUG_LEVEL >= 1
			 cout << curr_frame << ": " << cl->name << "->" << method_to_be_invoked->name << ":" << method_to_be_invoked->descriptor << "(native method)" << endl << flush ;
#endif			 
//end of code added 24/7/2005			 
			    
			  //code changed 3/8/2005
			  //execute_native_method(method_to_be_invoked,
			  //			obj_val,
			  //			values) ;
			  execute_native_method(method_to_be_invoked,
						obj_val,
						values,
			                        false) ;
			  //end of code changed 3/8/2005
			    
			  threads.at(c)->pc += 3 ;

			}
			else
			{

			  //code changed 25/7/2005
			  //frame *fm = new frame() ;
			  //fm->rcp = &(method_to_be_invoked->type->rcp) ;
			  //fm->curr_method = method_to_be_invoked ;
			  frame *fm = new frame(method_to_be_invoked) ;
			  //end of code changed 25/7/2005
				
				
#if DEBUG_LEVEL >= 2
			  cout << fm << ":" << fm->curr_method->type->name << ":" ;
			  cout << fm->curr_method->name << endl ;
#endif
			  fm->local_variables.reserve(method_to_be_invoked->max_locals + 1) ;

			  fm->prev_pc_value = threads.at(c)->pc + 3 ;

//code added 23/11/2004
#if DEBUG_LEVEL >= 3
			  assert(obj_val->ref_value->type != NULL) ;
#endif
//end of code added 23/11/2004
			  fm->local_variables.push_back(create_local_variable(obj_val)) ;

			  //code changed 3/2/2005
			  //delete obj_val ;
			  //end of code changed 3/2/2005

			  while(!values.empty())
			  {
			    	value *v = values.top() ;
				    values.pop() ;
    				if(IsCategory1(v))
	    				fm->local_variables.push_back(create_local_variable(v)) ;
    				else
	    			{
		    			fm->local_variables.push_back(create_local_variable(v)) ;
			    		LocalVariable *dummy = createLocalVariable() ;
				    	dummy->valid_flag = false;
					    fm->local_variables.push_back(dummy) ;
    				}

				//code changed 3/2/2005			
	    			//delete v ;
	    			//end of code changed 3/2/2005
			  }

			  for(int it=fm->local_variables.size(); it < method_to_be_invoked->max_locals+1; it++)
			    fm->local_variables.push_back(NULL) ; 
			  
//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << curr_frame << ": " << cl->name << "->" << method_to_be_invoked->name << ":" << method_to_be_invoked->descriptor << "(" << fm << ")" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 			

			  
			  threads.at(c)->java_stack.push(fm) ;
			  threads.at(c)->pc = 0 ;
			}

			break ;
		}

		case 184: //invokestatic: invoke a class (static) method
		{
			int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = index1 * 256 + index2 ;

			string method_parent_class_name = curr_frame->rcp->at(rcp_index-1)->method_parent_class_name ;
			string method_name = curr_frame->rcp->at(rcp_index-1)->method_name ;
			string method_descriptor = curr_frame->rcp->at(rcp_index-1)->method_descriptor ;
			
			method *resolved_method ;

			Class *cl ;

			try
			{
				
				cl = resolve_class(this_class,method_parent_class_name) ;

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
				assert(cl != NULL) ;
#endif
//end of code added 28/2/2005

				if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
					resolved_method = curr_frame->rcp->at(rcp_index-1)->resolved_method ;
				else
				{
					resolved_method = resolve_class_method(this_class,
						   		       method_parent_class_name,
								       method_name,
						         	       method_descriptor) ;

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
					assert(resolved_method != NULL) ;
#endif
//end of code added 28/2/2005

					curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					curr_frame->rcp->at(rcp_index-1)->resolved_method = resolved_method ;
				}
			}
			catch(LinkageError)
			{
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}
			catch(NoSuchMethodError)
			{
				throw_exception("java/lang/NoSuchMethodError") ;
				return ;
			}

//code added 15/11/2004
#if DEBUG_LEVEL >= 3
			Logger::Log(cl->name + "::" + resolved_method->name) ;
#endif
//end of code added 15/11/2004

			if(resolved_method->blnStatic == false)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}

			if(cl->blnInitialised == false)
			{
//code added 16/11/2004
#if DEBUG_LEVEL >= 3
				Logger::Log("Initialising class " + cl->name) ;
#endif
//end of code added 16/11/2004				
				InitialiseClass(cl) ;
				break ;
			}


			//this code is needed to determine the number
			//of parameters of the method (by parsing the
			//descriptor); we need this number to pop
			//the correct number of values from the
			//operand stack

			string parameter_string = method_descriptor.substr(0,
								   method_descriptor.find(")") + 1) ;
			vector<string> *parameters = util::parse_params(parameter_string.substr(1,parameter_string.length()-2)) ;

			int param_count = parameters->size() ;

			delete parameters ;

			//temporary stack used to transfer
			//values to the new frame's local variables
			//array
			stack<value *> values ;

			int i=0 ;

//code added 15/11/2004
#if DEBUG_LEVEL >= 3
			if(param_count > 0)
				assert(!op_st->empty());
#endif
//end of code added 15/11/2004

			while(i < param_count)
			{
				value *v = op_st->top() ;
				op_st->pop() ;

				values.push(v) ;
				i++ ;
			}

			if(resolved_method->blnSynchronized == true)
			{
				//acquire/reenter monitor associated
				//with the class
				if(cl->mon.blnFree == true)
				{
					cl->mon.owner_thread_id = threads.at(c)->thread_id ;
					cl->mon.increment_count() ;
					threads.at(c)->monitors.push_back(cl->mon) ;
				}
				else
				{
					if(cl->mon.owner_thread_id == threads.at(c)->thread_id)
					{
						cl->mon.increment_count() ;
						threads.at(c)->monitors.push_back(cl->mon) ;
					}
					else
					{
						if(curr_thread != threads.size()-1)
							curr_thread++ ;
						else
							curr_thread = 0 ;
					}
				}
			}

			if(resolved_method->blnNative == true)
			{

			  if(resolved_method->blnBound == false)
			    if(bind_native_method(resolved_method) == false)
			    {
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
			      //Logger::LogFunctionExit("jvm::execute_instruction()") ;
#endif
//end of code added 13/11/2004			    	
			      return ;
			    }

			  value *class_obj_val = createValue() ;
			  class_obj_val->value_type = value::REFERENCE ;

//code added 11/2/2005
#if DEBUG_LEVEL >= 3
			  assert(cl->class_object != NULL) ;	  
			  //assert(cl->class_object->type != NULL) ;
#endif
//end of code added 11/2/2005

			  class_obj_val->ref_value = cl->class_object ;

			  //code added 13/2/2005
			  curr_frame->blnExecutingNativeMethod = true ;
			  //end of code added 13/2/2005

                          //code changed 3/8/2005			    
			  //execute_native_method(resolved_method,
			  //			class_obj_val,
			  //			values) ;
                          
                          //code changed 24/9/2005
			  //execute_native_method(resolved_method,
                          //                    class_obj_val,
                          //                    values,
                          //                    true) ;
                          value* v1 = createValue() ;
                          v1->value_type = value::REFERENCE ;
                          v1->ref_value = getClassObject(cl) ;
                          execute_native_method(resolved_method,
						v1,
						values,
						true) ;
                          //end of code changed 24/9/2005
                                                
                          //end of code changed 3/8/2005

			  //code added 13/2/2005
			  curr_frame->blnExecutingNativeMethod = false ;
			  //end of code added 13/2/2005

			  threads.at(c)->pc += 3 ;
			  //throw_exception("java/lang/UnsatisfiedLinkError") ;
			  //return ;
			}
			else
			{
			  //code changed 25/7/2005
			  //frame *fm = new frame() ;
			  //fm->rcp = &(resolved_method->type->rcp) ;
			  //fm->curr_method = resolved_method ;
			  frame *fm = new frame(resolved_method) ;
			  //end of code added 25/7/2005
#if DEBUG_LEVEL >= 2
			  cout << fm << ":" << fm->curr_method->type->name << ":" ;
			  cout << fm->curr_method->name << endl ;
#endif
			  fm->local_variables.reserve(resolved_method->max_locals + 1) ;


			  //code changed 24/2/2005
			  //the class object need not be put at location
			  //0 of the frame's local variables array (Duh!)
			  /*
			  value *class_obj_val = createValue() ;
			  class_obj_val->value_type = value::REFERENCE ;

//code added 11/2/2005
#if DEBUG_LEVEL >= 3
			  //assert(cl->class_object != NULL) ;
			  //cout << resolved_method->name << endl << flush ;
			  //if(cl->class_object->blnStringObject != true)
			  //	assert(cl->class_object->type != NULL) ;
#endif
//end of code added 11/2/2005

			  class_obj_val->ref_value = cl->class_object ;

			  fm->local_variables.push_back(create_local_variable(class_obj_val)) ;
			  */
			  //end of code changed 24/2/2005

			  fm->prev_pc_value = threads.at(c)->pc + 3 ;

			  while(!values.empty())
			  {
		    		value *v = values.top() ;
	    			values.pop() ;
    				if(IsCategory1(v))
				  fm->local_variables.push_back(create_local_variable(v)) ;
				else
			    	{
				  fm->local_variables.push_back(create_local_variable(v)) ;
				  LocalVariable *dummy = createLocalVariable() ;
				  dummy->valid_flag = false;
				  fm->local_variables.push_back(dummy) ;
				}

				//code changed 3/2/2005
				//delete v ;
				//end of code changed 3/2/2005
			  }

			  for(int it=fm->local_variables.size(); it < resolved_method->max_locals+1; it++)
			    fm->local_variables.push_back(NULL) ;
			  
//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << curr_frame << ": " << cl->name << "->" << resolved_method->name << ":" << resolved_method->descriptor << "(" << fm << ")" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 			

			  
			  threads.at(c)->java_stack.push(fm) ;
			  threads.at(c)->pc = 0 ;
			}
			break ;
		}

		case 182: //invokevirtual: invoke instance method; dispatch based on class
		{
			int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = index1 * 256 + index2 ;

			string method_parent_class_name = curr_frame->rcp->at(rcp_index-1)->method_parent_class_name ;
			string method_name = curr_frame->rcp->at(rcp_index-1)->method_name ;
			string method_descriptor = curr_frame->rcp->at(rcp_index-1)->method_descriptor ;

			method *resolved_method ;

			try
			{

				if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
					resolved_method = curr_frame->rcp->at(rcp_index-1)->resolved_method ;
				else
				{
					resolved_method = resolve_class_method(this_class,
						   		       method_parent_class_name,
								       method_name,
						         	       method_descriptor) ;

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
					assert(resolved_method != NULL) ;
#endif
//end of code added 28/2/2005

					curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					curr_frame->rcp->at(rcp_index-1)->resolved_method = resolved_method ;
				}
			}
			catch(LinkageError)
			{
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}
			catch(NoSuchMethodError)
			{
				throw_exception("java/lang/NoSuchMethodError") ;
				return ;
			}

//code added 23/11/2004
#if DEBUG_LEVEL >= 3
			//chokes for some reason
			//Logger::Log(method_parent_class_name + "::" + resolved_method->name) ;
#endif
//end of code added 23/11/2004

			if(resolved_method->blnStatic == true)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}

			//this code is needed to determine the number
			//of parameters of the method (by parsing the
			//descriptor); we need this number to pop
			//the correct number of values from the
			//operand stack

			string parameter_string = method_descriptor.substr(0,
								   method_descriptor.find(")") + 1) ;

			vector<string> *parameters = util::parse_params(parameter_string.substr(1,parameter_string.length()-2)) ;

			int param_count = parameters->size() ;

			delete parameters ;

			//temporary stack used to transfer
			//values to the new frame's local variables
			//array
			stack<value *> values ;

			int i=0 ;

//code added 15/11/2004
#if DEBUG_LEVEL >= 3
			if(param_count > 0)
				assert(!op_st->empty());
#endif
//end of code added 15/11/2004

			while(i < param_count)
			{
				value *v = op_st->top() ;
				op_st->pop() ;

				values.push(v) ;
				i++ ;
			}

			value *obj_val = op_st->top() ;
			op_st->pop() ;

//code added 17/11/2004
#if DEBUG_LEVEL >= 3
			assert(obj_val->value_type == value::REFERENCE) ;
			assert(obj_val->ref_value != NULL) ;
			//if(obj_val->ref_value->blnArrayObject != true)
			//	assert(obj_val->ref_value->type != NULL) ;
			//else
			//	assert(obj_val->ref_value->array_component_type != NULL) ;
#endif
//end of code added 17/11/2004
 
			if(obj_val->ref_value->blnNull == true)
			{
				EmptyValueStack(&values) ;
				delete obj_val ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}

			method *method_to_be_invoked ;

			Class *C = obj_val->ref_value->type ;

			try
			{
				method_to_be_invoked = GetMethod(C,method_name,method_descriptor) ;
			}
			catch(AbstractMethodError)
			{
				EmptyValueStack(&values) ;
				delete obj_val ;
				throw_exception("java/lang/AbstractMethodError") ;
				return ;
			}

			if(method_to_be_invoked->blnAbstract == true)
			{
				EmptyValueStack(&values) ;
				delete obj_val ;
				throw_exception("java/lang/AbstractMethodError") ;
				return ;
			}

			if(method_to_be_invoked->blnSynchronized == true)
			{
				//acquire/reenter monitor associated
				//with objectref
				if(obj_val->ref_value->mon.blnFree == true)
				{
					obj_val->ref_value->mon.owner_thread_id = threads.at(c)->thread_id ;
					obj_val->ref_value->mon.increment_count() ;
					threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;
				}
				else
				{
					if(obj_val->ref_value->mon.owner_thread_id == threads.at(c)->thread_id)
					{
						obj_val->ref_value->mon.increment_count() ;
						threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;
					}
					else
					{
						if(curr_thread != threads.size()-1)
							curr_thread++ ;
						else
							curr_thread = 0 ;
					}
				}
			}
			
			if(method_to_be_invoked->blnNative == true)
			{

			  if(method_to_be_invoked->blnBound == false)
			    if(bind_native_method(method_to_be_invoked) == false)
			      return ;

			  //code changed 3/8/2005
			  //execute_native_method(method_to_be_invoked,
			  //			obj_val,
			  //			values) ;
			  execute_native_method(method_to_be_invoked,
						obj_val,
						values,
			                        false) ;
			  //end of code changed 3/8/2005

			  threads.at(c)->pc += 3 ;
			}
			else
			{

			  //code changed 25/7/2005				
			  //frame *fm = new frame() ;

			  //fm->rcp = &(method_to_be_invoked->type->rcp) ;
			  //fm->curr_method = method_to_be_invoked ;
			  frame *fm = new frame(method_to_be_invoked) ;				
			  //end of code changed 25/7/2005
				
#if DEBUG_LEVEL >= 2
			  cout << fm << ":" << fm->curr_method->type->name << ":" ;
			  cout << fm->curr_method->name << endl ;
#endif
				
			  fm->local_variables.reserve(method_to_be_invoked->max_locals + 1) ;

			  fm->local_variables.push_back(create_local_variable(obj_val)) ;

			  fm->prev_pc_value = threads.at(c)->pc + 3 ;

			  while(!values.empty())
			  {

	    			value *v = values.top() ;

			    //temp code
			    //barking up the wrong tree; if getProperty() were
			    //invoked as getProperty(someString, null) -- as actually
			    //happens when it is called from getProperty(String), the second
			    //parameter will obviously not have blnStringObject = true				  
			    //if(method_to_be_invoked->name == "getProperty" &&
			    //   method_to_be_invoked->descriptor == "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;")
			    //{
			    //	assert(v->value_type == value::REFERENCE) ;
			    //	v->ref_value->display() ;
			    //	//cout << v->ref_value->type->name << endl << flush ;
			    //	//cout << v->ref_value << endl << flush ;
			    //	assert(v->ref_value->blnStringObject == true) ;
			    //	//cout << "getProperty(): " << v->ref_value->string_val << endl << flush ;
			    //}
			    //end of temp code
			      
		    		values.pop() ;
			    	if(IsCategory1(v))
				    	fm->local_variables.push_back(create_local_variable(v)) ;
    				else
	    			{
		    			fm->local_variables.push_back(create_local_variable(v)) ;
			    		LocalVariable *dummy = createLocalVariable() ;
				    	dummy->valid_flag = false;
    					fm->local_variables.push_back(dummy) ;
	    			}

				//end of code changed 3/2/2005
		    		//delete v ;
		    		//end of code changed 3/2/2005

			  }

			  for(int it=fm->local_variables.size(); it < method_to_be_invoked->max_locals+1; it++)
			    fm->local_variables.push_back(NULL) ;

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << curr_frame << ": " << C->name << "->" << method_to_be_invoked->name << ":" << method_to_be_invoked->descriptor << "(" << fm << ")" <<endl << flush ;
#endif			 
//end of code added 30/4/2005			 			
			  
			  threads.at(c)->java_stack.push(fm) ;
			  threads.at(c)->pc = 0 ;
			}

			break ;
		}

		case 128: //ior: boolean OR int
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::INT ;
			r->i_value = v1->i_value | v2->i_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 112: //irem: remainder int
		{
			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *v1 = op_st->top() ;
			op_st->pop() ;

//code added 29/1/2005
#if DEBUG_LEVEL >= 3
			assert(v1 != NULL) ;
			assert(v2 != NULL) ;
			assert(v1->value_type == value::INT) ;
			assert(v2->value_type == value::INT) ;
#endif
//end of code added 29/1/2005

			if(v2->i_value == 0)
			{
				//delete v1 ;
				//delete v2 ;
				throw_exception("java/lang/ArithmeticException") ;
				return ;
			}

			value *r = createValue() ;
			r->value_type = value::INT ;
			r->i_value = v1->i_value - (v1->i_value / v2->i_value) * v2->i_value ;

			op_st->push(r) ;


			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 172: //ireturn: return int from method
		{
			value *i_val = op_st->top() ;

//code added 29/1/2005
#if DEBUG_LEVEL >= 3
			assert(i_val != NULL) ;

			//code changed 26/3/2005
			//see comment in iadd
			//assert(i_val->value_type == value::INT) ;
			//end of code changed 26/3/2005

#endif
//end of code added 29/1/2005			

			//if the method is a synchronised method,
			//the object's monitor should be owned
			//by the current thread
			
			//code added 15/1/2005
			//if(current_method->blnSynchronized == true)			
			if(current_method->blnSynchronized == true && curr_frame->mon_obj != NULL)
			//end of code added 15/1/2005
			{
				object *mon_obj = curr_frame->mon_obj ;

				if(mon_obj->mon.owner_thread_id !=
				   threads.at(c)->thread_id)
				{
					throw_exception("java/lang/IllegalMonitorStateException") ;
					return ;
				}

				mon_obj->mon.decrement_count() ;
			}

			//code added 29/1/2005
			//to preserve the value to be returned
			op_st->pop() ;
			//end of code added 29/1/2005

			//pop the operand stack of the
			//current frame
			while(!op_st->empty())
			{
				delete op_st->top() ;
				op_st->pop() ;
			}

			//pop the current frame from the
			//currrent thread's stack
			threads.at(c)->java_stack.pop() ;

			//get the next instruction to 
			//execute (this was stored when
			//the frame was created)
			threads.at(c)->pc = curr_frame->prev_pc_value ;

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << "ireturn " << "(" << curr_frame << ")" ;
			 if(i_val->value_type == value::INT)
				 cout << i_val->i_value << endl << flush ;
			 else
			 {
				 cout << "(incorrect type) -- " << i_val->value_type << endl << flush ;
				 if(i_val->value_type == value::BOOLEAN)
				 {
					 if(i_val->bool_value == true)
					 	cout << "the incorrect value is true -- " ;
					 else
						cout << "the incorrect value is false -- " ;
					 cout  << i_val->bool_value << endl << flush ;
				 }
			 }
#endif			 
//end of code added 30/4/2005			 			   
			 
			//discard the current frame
			delete curr_frame ;

			//push the object reference to the 
			//invoking frame
			frame *invoking_frame = threads.at(c)->java_stack.top() ;
			 
			//code added 6/8/2005
			if(i_val->value_type == value::BOOLEAN)
				i_val->i_value = i_val->bool_value ;
			//end of code added 6/8/2005
			 
			invoking_frame->operand_stack.push(i_val) ;
			

			
			break ;
		}

		case 120: //ishl: shift left int
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::INT ;

			//code changed 19/2/2005
			//r->i_value = v1->i_value * pow((double)2, (double)(v2->i_value & 0x1f)) ;
			r->i_value = (long)(v1->i_value * pow((double)2, (double)(v2->i_value & 0x1f))) ;
			//end of code changed 19/2/2005

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 122: //ishr: arithmetic shift right int
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::INT ;

			//code changed 19/2/2005
			//r->i_value = v1->i_value / pow((double)2, (double)(v2->i_value & 0x1f)) ;
			r->i_value = (long)(v1->i_value / pow((double)2, (double)(v2->i_value & 0x1f))) ;
			//end of changed 19/2/2005

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 54: //istore: store int into local variable
		{
			int index = current_method->code.byte_code.at(prog_counter+1) ;

			istore(index) ;

			threads.at(c)->pc += 2 ; //2 because the index of the
			break ;			 //local variable is also stored
						 //in the method's code
		}

		case 59: //istore_0: store int into local variable
		{
			istore(0) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 60: //istore_1: store int into local variable
		{
			istore(1) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 61: //istore_2: store int into local variable
		{
			istore(2) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 62: //istore_3: store int into local variable
		{
			istore(3) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 100: //isub: subtract int
		{
			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *v1 = op_st->top() ;
			op_st->pop() ;

//code added 12/2/2005
#if DEBUG_LEVEL >= 3
			assert(v1->value_type == value::INT) ;
			assert(v2->value_type == value::INT) ;
#endif
//end of code added 12/2/2005

			value *r = createValue() ;
			r->value_type = value::INT ;
			r->i_value = v1->i_value - v2->i_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 124: //iushr: logical shift right int
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::INT ;

			int s = v2->i_value & 0x1f ;

			if(v1->i_value > 0)
				r->i_value = v1->i_value >> s ;
			else
				r->i_value = (v1->i_value >> s) + (2 << ~s) ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 130: //ixor: boolean XOR int
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::INT ;

			r->i_value = v1->i_value ^ v2->i_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 168: //jsr: jump subroutine
		{
			int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
			int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

			value *address = createValue() ;
			address->value_type = value::RETURN_ADDRESS ;

			address->ret_add_value = prog_counter+3 ;

			op_st->push(address) ;

			int offset = branchbyte1 * 256 + branchbyte2 ;

			threads.at(c)->pc += offset ;
			break ;
		}

		case 201: //jsr_w: jump subroutine (wide index)
		{
			int branchbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
			int branchbyte2 = current_method->code.byte_code.at(prog_counter+2) ;
			int branchbyte3 = current_method->code.byte_code.at(prog_counter+3) ;
			int branchbyte4 = current_method->code.byte_code.at(prog_counter+4) ;

			value *address = createValue() ;
			address->value_type = value::RETURN_ADDRESS ;

			address->ret_add_value = prog_counter+5 ;

			op_st->push(address) ;

			int offset = branchbyte1 * 256*256 + 
						 branchbyte2 * 256*16 +
						 branchbyte3 * 256 +
						 branchbyte4 ;

			threads.at(c)->pc += offset ;
			break ;
		}

		case 138: //l2d: convert long to double
		{
			value *v = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::DOUBLE ;
			r->double_value = v->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v ;	
			//end of code changed 9/2/2005
			
			threads.at(c)->pc++ ;
			break ;
		}

		case 137: //l2f: convert long to float
		{
			value *v = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::FLOAT ;
			r->float_value = v->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 136: //l2i: convert long to int
		{
			value *v = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::INT ;
			r->i_value = v->long_value & 0xFFFFFFFF ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 97: //ladd: add long
		{
			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *v1 = op_st->top() ;
			op_st->pop() ;

#if DEBUG_LEVEL >= 3
			 assert(v2->value_type == value::LONG) ;
			 assert(v1->value_type == value::LONG) ;
#endif

			value *r = createValue() ;
			r->value_type = value::LONG ;
			r->long_value = v1->long_value + v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 47: //laload: load long from array
		{
			value *index = op_st->top() ;
			op_st->pop() ;

			value *arrayref = op_st->top() ;
			if(arrayref->ref_value->blnNull == true)
			{
				delete index ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}
			op_st->pop() ;

			if(index->i_value < 0 ||
			   //code changed 6/3/2005
			   //index->i_value >= arrayref->ref_value->items.size())
			   index->i_value >= arrayref->ref_value->primitives.size())
			   //end of code changed 6/3/2005
			{
				delete index ;
				delete arrayref ;
				throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				return ;
			}

			value *obj_val = createValue() ;

			obj_val->value_type = value::LONG ;
			obj_val->long_value = arrayref->ref_value->primitives.at(index->i_value)->long_value ;

			op_st->push(obj_val) ;

			//code changed 3/2/2005
			//delete index ;
			//delete arrayref ;
			//end of code changed 3/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 127: //land: boolean AND long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue();
			r->value_type = value::LONG ;
			r->long_value = v1->long_value & v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 80: //lastore: store into long array
		{
			value *obj_val = op_st->top() ;
			op_st->pop() ;

			value *index = op_st->top() ;
			op_st->pop() ;

			value *arrayref = op_st->top() ;
			if(arrayref->ref_value->blnNull == true)
			{
				delete obj_val ;
				delete index ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}
			op_st->pop() ;

			if(index->i_value < 0 ||
			   //code changed 6/3/2005
			   //index->i_value >= arrayref->ref_value->items.size())
			   index->i_value >= arrayref->ref_value->primitives.size())
			   //end of code changed 6/3/2005
			{
				delete obj_val ;
				delete index ;
				delete arrayref ;
				throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				return ;
			}

			primitive_value *pv = createPrimitiveValue() ;

			pv->value_type = primitive_value::LONG ;
			pv->long_value = obj_val->long_value ;

			//delete arrayref->ref_value->primitives.at(index->i_value) ;
			arrayref->ref_value->primitives.at(index->i_value) = pv;

			//code changed 3/2/2005
			//delete obj_val ;
			//delete index ;
			//delete arrayref ;
			//end of code changed 3/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 148: //lcmp: compare long
		{
			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *i_val = createValue() ;
			i_val->value_type = value::INT ;

			if(v1->long_value > v2->long_value)
				i_val->i_value = 1 ;
			else if(v1->long_value == v2->long_value)
				i_val->i_value = 0 ;
			else
				i_val->i_value = -1 ;

			op_st->push(i_val) ;


			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 9: //lconst_0: push long constant (0)
		{
			value *v = createValue() ;
			v->value_type = value::LONG ;
			v->long_value = 0 ;

			op_st->push(v) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 10: //lconst_1: push long constant (1)
		{
			value *v = createValue() ;
			v->value_type = value::LONG ;
			v->long_value = 1 ;

			op_st->push(v) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 18: //ldc: push item from runtime constant pool
		{

			int index = current_method->code.byte_code.at(prog_counter+1) ;

			value *v = createValue() ;

			if(this_class->rcp.at(index-1)->flag == runtime_constant_pool::FLOAT_CONSTANT)
			{
				v->value_type = value::FLOAT ;
				v->float_value = this_class->rcp.at(index-1)->double_value ;
			}
			else if(this_class->rcp.at(index-1)->flag == runtime_constant_pool::INTEGER_CONSTANT)
			{
				v->value_type = value::INT ;
				v->i_value = this_class->rcp.at(index-1)->int_value ;
			}
			else //it has to be a string
			{

//code added 21/2/2005
#if DEBUG_LEVEL >= 3
			  assert(this_class->rcp.at(index-1)->flag == runtime_constant_pool::STRING_LITERAL) ;
#endif
//end of code added 21/2/2005

				string s = this_class->rcp.at(index-1)->str_value ;

				v->value_type = value::REFERENCE ;

				if(strings.find(s) != strings.end())
				{
//code added 11/2/2005
#if DEBUG_LEVEL >= 3
					assert(strings[s] != NULL);
#endif
//end of code added 11/2/2005					

					v->ref_value = strings[s] ;

                                        //code added 26/2/2005
				        typedef map<key, Class *>::const_iterator CI ;

					CI ci = classes.find(key(NULL,"java/lang/String")) ;

				        if(blnStringClassInitialised == false)
				        {
				            blnStringClassInitialised = true ;
					    InitialiseClass(classes[ci->first]) ;	
					    break ;
					}
					//end of code added 26/2/2005

				}
				else
				{

					object *string_obj = create_string_object(s) ;

					//code added 25/2/2005
					setup_string_init(string_obj, s) ;
					break ;
					//end of code added 25/2/2005

                                        //code added 20/2/2005
				        typedef map<key, Class *>::const_iterator CI ;

					CI ci = classes.find(key(NULL,"java/lang/String")) ;

				        if(blnStringClassInitialised == false)
				        {
				            blnStringClassInitialised = true ;
					    InitialiseClass(classes[ci->first]) ;	
					    break ;
					}
					//end of code added 20/2/2005

					v->ref_value = string_obj ;

					strings[s] = string_obj ;

					//code added 26/2/2005
					break ;
					//end of code added 26/2/2005
				}

			}

			op_st->push(v) ;

			threads.at(c)->pc += 2 ;

			break ;
		}

		case 19: //ldc_w: push item from runtime constant pool (wide index)
		{
			int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = index1 * 256 + index2 ;

			value *v = createValue() ;

			if(this_class->rcp.at(rcp_index-1)->flag == runtime_constant_pool::FLOAT_CONSTANT)
			{
				v->value_type = value::FLOAT ;
				v->float_value = this_class->rcp.at(rcp_index-1)->double_value ;
			}
			else if(this_class->rcp.at(rcp_index-1)->flag == runtime_constant_pool::INTEGER_CONSTANT)
			{
				v->value_type = value::INT ;
				v->i_value = this_class->rcp.at(rcp_index-1)->int_value ;
			}
			else //it has to be a string
			{
 
//code added 23/2/2005
#if DEBUG_LEVEL >= 3
			  assert(this_class->rcp.at(rcp_index-1)->flag == runtime_constant_pool::STRING_LITERAL) ;
#endif
//end of code added 23/2/2005

				string s = this_class->rcp.at(rcp_index-1)->str_value ;

				v->value_type = value::REFERENCE ;

				if(strings.find(s) != strings.end())
				{
//code added 11/2/2005
#if DEBUG_LEVEL >= 3
					assert(strings[s] != NULL);
#endif
//end of code added 11/2/2005					
					v->ref_value = strings[s] ;

                                        //code added 26/2/2005
				        typedef map<key, Class *>::const_iterator CI ;

					CI ci = classes.find(key(NULL,"java/lang/String")) ;

				        if(blnStringClassInitialised == false)
				        {
				            blnStringClassInitialised = true ;
					    InitialiseClass(classes[ci->first]) ;	
					    break ;
					}
					//end of code added 26/2/2005

				}					
				else
				{

					object *string_obj = create_string_object(s) ;

					//code added 25/2/2005
					setup_string_init(string_obj, s) ;
					break ;
					//end of code added 25/2/2005

                                        //code added 20/2/2005
				        typedef map<key, Class *>::const_iterator CI ;

					CI ci = classes.find(key(NULL,"java/lang/String")) ;

				        if(blnStringClassInitialised == false)
				        {
				            blnStringClassInitialised = true ;
					    InitialiseClass(classes[ci->first]) ;	
					    break ;
					}
					//end of code added 20/2/2005

					v->ref_value = string_obj ;

					strings[s] = string_obj ;

					//code added 26/2/2005
					break ;
					//end of code added 26/2/2005
				}
			}

			op_st->push(v) ;

			threads.at(c)->pc += 3 ;
			break ;
		}

		case 20: //ldc2_w: push long or double from runtime constant pool (wide index)
		{
			int index1 = current_method->code.byte_code.at(prog_counter+1) ;
			int index2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = index1 * 256 + index2 ;

			value *v = createValue() ;

			if(this_class->rcp.at(rcp_index-1)->flag == runtime_constant_pool::LONG_CONSTANT)
			{
				v->value_type = value::LONG ;
				v->long_value = this_class->rcp.at(rcp_index-1)->int_value ;
			}
			else //it has to be double
			{
				v->value_type = value::DOUBLE ;
				v->double_value = this_class->rcp.at(rcp_index-1)->double_value ;
			}

			op_st->push(v) ;

			threads.at(c)->pc += 3 ;
			break ;
		}

		case 109: //ldiv: divide long
		{
			value *v2 = op_st->top() ;
			op_st->pop() ;
			value *v1 = op_st->top() ;
			op_st->pop() ;

			if(v2->long_value == 0)
			{
				//delete v1 ;
				//delete v2 ;
				throw_exception("java/lang/ArithmeticException") ;
				return ;
			}

			value *r = createValue() ;
			r->value_type = value::LONG ;
			r->long_value = v1->long_value / v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005
			
			threads.at(c)->pc++ ;
			break ;
		}

		case 22: //lload: load long from local variable
		{
			int index = current_method->code.byte_code.at(prog_counter+1) ;

			lload(index) ;

			threads.at(c)->pc += 2 ; //2 because the index of the
			break ;			 //local variable is also stored
						 //in the method's code
		}

		case 30: //lload_0: load long from local variable
		{
			lload(0) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 31: //lload_1: load long from local variable
		{
			lload(1) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 32: //lload_2: load long from local variable
		{
			lload(2) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 33: //lload_3: load long from local variable
		{
			lload(3) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 105: //lmul: multiply long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;
			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;
			r->long_value = v1->long_value * v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 117: //lneg: negate long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;
			r->long_value = (-1) * v1->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 171: //lookupswitch: access jump table by key match and jump
		{

			//find the nearest opcode that is a 
			//multiple of 4 (this is used to
			//calculate the number of padding bytes)
			int pc = threads.at(c)->pc ;

			int padbytes = 4 - ((pc+1) % 4) ;
			
			int def1 = current_method->code.byte_code.at(prog_counter+padbytes) ;
			int def2 = current_method->code.byte_code.at(prog_counter+padbytes+1) ;
			int def3 = current_method->code.byte_code.at(prog_counter+padbytes+2) ;
			int def4 = current_method->code.byte_code.at(prog_counter+padbytes+3) ;

			int defaddress = def1 * 256*256 +
							 def2 * 256*16  +
							 def3 * 256     +
							 def4 ;

			int npairs1 = current_method->code.byte_code.at(prog_counter+padbytes+4) ;
			int npairs2 = current_method->code.byte_code.at(prog_counter+padbytes+5) ;
			int npairs3 = current_method->code.byte_code.at(prog_counter+padbytes+6) ;
			int npairs4 = current_method->code.byte_code.at(prog_counter+padbytes+7) ;

			int npairs = npairs1 * 256*256 +
				     npairs2 * 256*16  +
				     npairs3 * 256     +
				     npairs4 ;

			vector<match_offset *> match_offset1 ;

			pc += padbytes + 8 ; //this will give the start of the match-offsets

			for(int i=0; i<npairs; i++)
			{

				//does match take up four bytes?
				int matchbyte1 = current_method->code.byte_code.at(pc + i*8) ;
				int matchbyte2 = current_method->code.byte_code.at(pc + i*8 + 1) ;
				int matchbyte3 = current_method->code.byte_code.at(pc + i*8 + 2) ;
				int matchbyte4 = current_method->code.byte_code.at(pc + i*8 + 3) ;

				int match = matchbyte1 * 256*256 +
						 	matchbyte2 * 256*16  +
							matchbyte3 * 256     +
							matchbyte4 ;
				
				int offsetbyte1 = current_method->code.byte_code.at(pc + i*8 + 4) ;
				int offsetbyte2 = current_method->code.byte_code.at(pc + i*8 + 5) ;
				int offsetbyte3 = current_method->code.byte_code.at(pc + i*8 + 6) ;
				int offsetbyte4 = current_method->code.byte_code.at(pc + i*8 + 7) ;

				int offset = offsetbyte1 * 256*256 +
							 offsetbyte2 * 256*16  +
							 offsetbyte3 * 256     +
							 offsetbyte4 ;

				match_offset *mo = new match_offset() ;
				mo->match = match ;
				mo->offset = offset ;

				match_offset1.push_back(mo) ;	

			}

			value *v = op_st->top() ;
			op_st->pop() ;

			bool blnMatchFound = false ;

			for(int i=0; i<match_offset1.size() && blnMatchFound == false; i++)
			{
				if(v->i_value == match_offset1.at(i)->match)
				{
					threads.at(c)->pc += match_offset1.at(i)->offset ;
					blnMatchFound = true ;
				}
			}

			if(blnMatchFound == false)
				threads.at(c)->pc += defaddress ;

			for(int i=0; i<match_offset1.size(); i++)
				delete match_offset1.at(i) ;

			break ;
		}

		case 129: //lor: boolean OR long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;
			r->long_value = v1->long_value | v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 113: //lrem: remainder long
		{
			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *v1 = op_st->top() ;
			op_st->pop() ;

			if(v2->long_value == 0)
			{
				//delete v1 ;
				//delete v2 ;
				throw_exception("java/lang/ArithmeticException") ;
				return ;
			}

			value *r = createValue() ;
			r->value_type = value::LONG ;
			r->long_value = v1->long_value - (v1->long_value / v2->long_value) * v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 173: //lreturn: return long from method
		{
			value *l_val = op_st->top() ;
			
			//if the method is a synchronised method,
			//the object's monitor should be owned
			//by the current thread
			
			//code added 15/1/2005
			//if(current_method->blnSynchronized == true)			
			if(current_method->blnSynchronized == true && curr_frame->mon_obj != NULL)
			//end of code added 15/1/2005
			{
				object *mon_obj = curr_frame->mon_obj ;

				if(mon_obj->mon.owner_thread_id !=
				   threads.at(c)->thread_id)
				{
					throw_exception("java/lang/IllegalMonitorStateException") ;
					return ;
				}

				mon_obj->mon.decrement_count() ;
			}

			//code added 29/1/2005
			//to preserve the value to be returned
			op_st->pop() ;
			//end of code added 29/1/2005

			//pop the operand stack of the
			//current frame
			while(!op_st->empty())
			{
				delete op_st->top() ;
				op_st->pop() ;
			}

			//pop the current frame from the
			//currrent thread's stack
			threads.at(c)->java_stack.pop() ;

			//get the next instruction to 
			//execute (this was stored when
			//the frame was created)
			threads.at(c)->pc = curr_frame->prev_pc_value ;

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << "lreturn" << "(" << curr_frame << ")" << endl << flush ;
#endif			 
//end of code added 30/4/2005				
			
			//discard the current frame
			delete curr_frame ;

			//push the object reference to the 
			//invoking frame
			frame *invoking_frame = threads.at(c)->java_stack.top() ;
			invoking_frame->operand_stack.push(l_val) ;

			break ;
		}

		case 121: //lshl: shift left long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;

			//code changed 19/2/2005
			//r->long_value = v1->long_value * pow((double)2, (double)(v2->long_value & 0x3f)) ;
			r->long_value = (__int64)(v1->long_value * pow((double)2, (double)(v2->long_value & 0x3f))) ;
			//end of code changed 19/2/2005

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 123: //lshr: arithmetic shift right long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;

			//code changed 19/2/2005
			//r->long_value = v1->long_value / pow((double)2, (double)(v2->long_value & 0x3f)) ;
			r->long_value = (__int64)(v1->long_value / pow((double)2, (double)(v2->long_value & 0x3f))) ;
			//end of code changed 19/2/2005

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 55: //lstore: store long into local variable
		{
			int index = current_method->code.byte_code.at(prog_counter+1) ;

			lstore(index) ;

			threads.at(c)->pc += 2 ; //2 because the index of the
			break ;			 //local variable is also stored
						 //in the method's code
		}

		case 63: //lstore_0: store long into local variable
		{
			lstore(0) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 64: //lstore_1: store long into local variable
		{
			lstore(1) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 65: //lstore_2: store long into local variable
		{
			lstore(2) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 66: //lstore_3: store long into local variable
		{
			lstore(3) ;
			threads.at(c)->pc++ ;
			break ;
		}

		case 101: //lsub: subtract long
		{
			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;
			r->long_value = v1->long_value - v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 125: //lushr: logical shift right long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;

			int s = v2->long_value & 0x3f ;

			if(v1->long_value > 0)
				r->long_value = v1->long_value >> s ;
			else
				r->long_value = (v1->long_value >> s) + (2L << ~s) ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 131: //lxor: boolean XOR long
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			value *r = createValue() ;
			r->value_type = value::LONG ;

			r->long_value = v1->long_value ^ v2->long_value ;

			op_st->push(r) ;

			//code changed 9/2/2005
			//delete v1 ;
			//delete v2 ;
			//end of code changed 9/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 194: //monitorenter: enter monitor for object
		{
			value *obj_val = op_st->top() ;
			op_st->pop() ;

			if(obj_val->ref_value->blnNull == true)
			{
				delete obj_val ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}

			if(obj_val->ref_value->mon.blnFree == true)
			{
				obj_val->ref_value->mon.owner_thread_id = threads.at(c)->thread_id ;
				obj_val->ref_value->mon.increment_count() ;
				threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;
			}
			else
			{
				if(obj_val->ref_value->mon.owner_thread_id == threads.at(c)->thread_id)
				{
					obj_val->ref_value->mon.increment_count() ;
					threads.at(c)->monitors.push_back(obj_val->ref_value->mon) ;
				}
				else
				{
					if(curr_thread != threads.size()-1)
						curr_thread++ ;
					else
						curr_thread = 0 ;
				}
			}

			//code changed 3/2/2005
			//delete obj_val ;
			//end of code changed 3/2/2005
			
			threads.at(c)->pc++ ;
			break ;
		}

		case 195: //monitorexit: exit monitor for object
		{
			value *obj_val = op_st->top() ;
			op_st->pop() ;

//code added 6/3/2005
#if DEBUG_LEVEL >= 3
			assert(obj_val->value_type == value::REFERENCE) ;
#endif
//end of code added 6/3/2005

			if(obj_val->ref_value->blnNull == true)
			{
				delete obj_val ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}

			if(threads.at(c)->thread_id != obj_val->ref_value->mon.owner_thread_id)
			{
				delete obj_val ;
				throw_exception("java/lang/IllegalMonitorStateException") ;
				return ;
			}

			obj_val->ref_value->mon.decrement_count() ;

			//code changed 3/2/2005
			//delete obj_val ;
			//end of code changed 3/2/2005
			
			threads.at(c)->pc++ ;
			break ;
		}

		case 197: //multinewarray: create new multidimensional array
		{
			int indexbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
			int indexbyte2 = current_method->code.byte_code.at(prog_counter+2) ;
			int dimensions = current_method->code.byte_code.at(prog_counter+3) ;

			int rcp_index = indexbyte1 * 256 + indexbyte2 ;

			Class *cl ;

			try
			{
				if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
					cl = curr_frame->rcp->at(rcp_index-1)->resolved_class ;
				else
				{
					cl = resolve_class(this_class,
							  curr_frame->rcp->at(rcp_index-1)->class_interface_name) ;

//code added 28/2/2005
#if DEBUG_LEVEL >= 3
					 assert(cl != NULL) ;
#endif
//end of code added 28/2/2005

					curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					curr_frame->rcp->at(rcp_index-1)->resolved_class = cl ;
				}
			}
			catch(LinkageError)
			{
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}

			//temporary vector used to retrieve
			//values from the operand stack
			vector<value *> values ;

			int i=0 ;

			while(i < dimensions)
			{
				value *v = op_st->top() ;
				op_st->pop() ;

				if(v->i_value < 0)
				{
					//delete v ;
					//for(int j=0; j<values.size(); j++)
					//	delete values.at(j) ;
					throw_exception("java/lang/NegativeArraySizeException") ;
					return ;
				}

				values.push_back(v) ;
				i++ ;
			}

			object *curr_obj ;

			//according to the specification, if any count value
			//is zero, no subsequent dimensions are to be allocated;
			//since we are creating the objects starting from the
			//last dimension, how are we to do this?

			for(int i= values.size()-1; i>=0; i--)
			{
				int c = values.at(i)->i_value ;

				if(i == values.size()-1)
				{
					object *obj1 = createObject() ;
					obj1->blnArrayObject = true ;
					
					//the type of the array is actually
					//not cl, but the class equivalent
					//to the subarray class
					//(if cl represents A[][][], array_class_ref
					//would be represented by A[] for the last dimension,
					//A[][] for the next-to-last, and so on
					//obj1->array_component_type = cl ;

					//create c objects of the component type
					for(int j=0; j<c; j++)
					{
						object *obj= createObject() ;
						obj->type = cl->array_class_ref ;
						obj->blnNull = true ;

						obj1->items.push_back(obj) ;
					}
					curr_obj = obj1 ;
				}
				else
				{
					object *obj1 = createObject() ;
					obj1->blnArrayObject = true ;

					//the type of the array is actually
					//not cl, but the class equivalent
					//to the subarray class
					//(if cl represents A[][][], array_class_ref
					//would be represented by A[] for the last dimension,
					//A[][] for the next-to-last, and so on
					//obj1->array_component_type = cl ;

					//replicate curr_obj c times
					for(int j=0; j<c; j++)
						obj1->items.push_back(clone_object(curr_obj)) ;

					curr_obj = obj1 ;

				}
			}

			value *obj_val = createValue() ;
			obj_val->value_type = value::REFERENCE ;

//code added 11/2/2005
#if DEBUG_LEVEL >= 3
			assert(curr_obj != NULL);
#endif
//end of code added 11/2/2005					
			
			
			obj_val->ref_value = curr_obj ;

			op_st->push(obj_val) ;

			//code changed 3/2/2005
			//for(int i=0; i<values.size(); i++)
			//	delete values.at(i) ;
			//end of code changed 3/2/2005

			threads.at(c)->pc += 4 ;
			break ;
		}

		case 187: //new: create new object
		{
			int indexbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
			int indexbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = indexbyte1 * 256 + indexbyte2 ;

			Class *cl ;

			try
			{
				if(curr_frame->rcp->at(rcp_index-1)->blnResolved == true)
					cl = curr_frame->rcp->at(rcp_index-1)->resolved_class ;
				else
				{
					cl = resolve_class(this_class,
							  curr_frame->rcp->at(rcp_index-1)->class_interface_name) ;

					 //code added 28/2/2005
#if DEBUG_LEVEL >= 3
					 assert(cl != NULL) ;
#endif
					 //end of code added 28/2/2005

					curr_frame->rcp->at(rcp_index-1)->blnResolved = true ;
					curr_frame->rcp->at(rcp_index-1)->resolved_class = cl ;
				}
			}
			catch(LinkageError)
			{
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}

//code added 27/11/2004
#if DEBUG_LEVEL >= 3
			Logger::Log(cl->name) ;
#endif
//end of code added 27/11/2004

			if(cl->blnInterface == true || cl->blnAbstract == true)
			{
				throw_exception("java/lang/InstantiationError") ;
				return ;
			}

			if(cl->blnInitialised == false)
			{
				InitialiseClass(cl) ;
				break ; //we need to break here because
				        //initialising the class will
				        //insert a new frame which will
				        //call execute_instruction
			}

			value *obj_val = createValue() ;
			obj_val->value_type = value::REFERENCE ;

			object *obj = createObject() ;
			obj->type = cl ;

			for(int i=0; i<cl->fields.size(); i++)
			{
				value *v = createValue() ;
				v->value_type = cl->fields.at(i)->value_type ;
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
					//code added 14/1/2005
					obj1->parent_object = obj ;
					//end of code added 14/1/2005
					v->ref_value = obj1 ;
				}
				//code added 16/1/2005
				else
					assert(false) ;
				//end of code added 16/1/2005

//code added 27/11/2004
#if DEBUG_LEVEL >= 3
				assert(v != NULL) ;
#endif
//end of code added 27/11/2004

				obj->fields[cl->fields.at(i)->name] = v ;
			}

			//code added 28/11/2004
			//we also need to create entries in the 'fields'
			//map for the fields of superclasses of this class
			Class *t = cl->super_class ;
			
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
						//code added 14/1/2005
						obj1->parent_object = obj ;
						//end of code added 14/1/2005
						v->ref_value = obj1 ;
					}
					//code added 16/1/2005
					else
						assert(false) ;
					//end of code added 16/1/2005					

#if DEBUG_LEVEL >= 3
					assert(v != NULL) ;
#endif

					obj->fields[t->fields.at(i)->name] = v ;
				}
				t = t->super_class ;
			}			
			//end of code added 28/11/2004

			obj_val->ref_value = obj ;

			op_st->push(obj_val) ;

			threads.at(c)->pc += 3 ;
			break ;
		}

		case 188: //newarray: create new array
		{
			int atype = current_method->code.byte_code.at(prog_counter+1) ;

			value *count = op_st->top() ;
			op_st->pop() ;

			if(count->i_value < 0)
			{
				delete count ;
				throw_exception("java/lang/NegativeArraySizeException") ;
				return ;
			}

			object *obj = createObject() ;

			obj->blnArrayObject = true ;
			obj->blnPrimitiveArrayObject = true ;

			switch(atype)
			{
				case 4:
					obj->primitive_type = primitive_value::BOOLEAN ;
					break ;
				case 5:
					obj->primitive_type = primitive_value::CHAR ;
					break ;
				case 6:
					obj->primitive_type = primitive_value::FLOAT ;
					break ;
				case 7:
					obj->primitive_type = primitive_value::DOUBLE ;
					break ;
				case 8:
					obj->primitive_type = primitive_value::_BYTE ;
					break ;
				case 9:
					obj->primitive_type = primitive_value::SHORT ;
					break ;
				case 10:
					obj->primitive_type = primitive_value::INT ;
					break ;
				case 11:
					obj->primitive_type = primitive_value::LONG ;
					break ;
			} //end swicth

			for(int i=0; i<count->i_value; i++)
			{
				primitive_value *pv = createPrimitiveValue() ;
				pv->value_type = obj->primitive_type ;

				//just initialise everything to zero;
				//(actually, we only need to intialise
				//one member (i_value, bool_value, etc)
				//depending on the type)
				pv->IsNaN = 0 ;
				pv->IsInfinity = 0 ;
				pv->IsPositive = 0 ;

				pv->bool_value = 0 ;
				pv->i_value = 0 ;
				pv->long_value = 0 ;
				pv->float_value = 0 ;
				pv->double_value = 0 ;

				obj->primitives.push_back(pv) ;
			}

			value *arrayref = createValue() ;
			arrayref->value_type = value::REFERENCE ;
			arrayref->ref_value = obj ;

			op_st->push(arrayref) ;

			//code changed 9/2/2005
			//delete count ;
			//end of code changed 9/2/2005

			threads.at(c)->pc += 2 ;
			break ;
		}

		case 0: //nop: do nothing
		{
			//do nothing

			threads.at(c)->pc++ ;
			break ;
		}

		case 87: //pop: pop the top operand stack value
		{
			//code changed 3/2/2005
			//delete op_st->top() ;
			//end of code changed 3/2/2005
			 
			op_st->pop() ;

			threads.at(c)->pc++ ;
			break ;
		}

		case 88: //pop2: pop the top one or two operand stack values
		{
			value *v1 = op_st->top() ;
			
			if(IsCategory2(v1) == true)
			{
				//code changed 3/2/2005
				//delete v1 ;
				//end of code changed 3/2/2005
				op_st->pop() ;
			}
			else
			{
				op_st->pop() ;

				value *v2 = op_st->top() ;

				if(IsCategory1(v2) == true)
				{
					op_st->pop() ;
					
					//code changed 3/2/2005
					//delete v1 ;
					//delete v2 ;
					//end of code changed 3/2/2005
				}
				else
					op_st->push(v1) ;
			}
			

			threads.at(c)->pc++ ;
			break ;
		}

		case 181: //putfield: set field in object
		{
			int indexbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
			int indexbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = indexbyte1 * 256 + indexbyte2 ;

			field *f ;

			try
			{
				f = resolve_field(this_class,
						  curr_frame->rcp->at(rcp_index-1)->field_parent_class_name,			
						  curr_frame->rcp->at(rcp_index-1)->field_name,
						  curr_frame->rcp->at(rcp_index-1)->field_descriptor,
						  true) ;
			}
			catch(LinkageError)
			{
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}
			catch(NoSuchFieldError)
			{
				throw_exception("java/lang/NoSuchFieldError") ;
				return ;
			}

//code added 25/11/2004
#if DEBUG_LEVEL >= 3
			Logger::Log(curr_frame->rcp->at(rcp_index-1)->field_parent_class_name + ":" + f->name) ;
#endif
//end of code added 25/11/2004

			if(f->blnStatic == true)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}

			if(f->blnFinal == true && f->parent_class != this_class)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}

			value *v = op_st->top() ;
			op_st->pop() ;

			value *objref = op_st->top() ;
			op_st->pop() ;
			
//code added 14/6/2005
#if DEBUG_LEVEL >= 3
			cout << "~~~~~~~~~~begin putfield display~~~~~~~~~~" << endl << flush ;
			cout << "putfield(): objref = " << objref->ref_value << endl << flush ;
			//cout << "putfield(): type = " << objref->ref_value->type->name << endl << flush ;
			//objref->ref_value->display() ;
			cout << "putfield(): field name = " << f->name << endl << flush ;
			cout << "putfield(): field descriptor = " << f->get_descriptor() << endl << flush ;
			cout << "putfield(): field: " << endl << flush ;
			//v->display() ;
			cout << "~~~~~~~~~~end putfield display~~~~~~~~~~" << endl << flush ;
#endif			
//end of code added 14/6/2005			

//code added 27/11/2004
#if DEBUG_LEVEL >= 3
			assert(v != NULL) ;
			if(v->value_type == value::REFERENCE)
				assert(v->ref_value != NULL) ;
				
			assert(objref != NULL) ;
			if(objref->value_type == value::REFERENCE)
				assert(objref->ref_value != NULL) ;			
#endif
//end of code added 27/11/2004

			if(objref->ref_value->blnNull == true)
			{
				//delete v ;
				//delete objref ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}

			//code changed 3/2/2005
			//delete objref->ref_value->fields[f->name] ;
			//end of code changed 3/2/2005


			objref->ref_value->fields[f->name] = v ;

			//code added 1/2/2005
			if(v->value_type == value::REFERENCE && f->name == "this")
			{
#if DEBUG_LEVEL >= 3
				assert(v->ref_value != NULL) ;
#endif				
				objref->ref_value->parent_object = v->ref_value ;
			}
			//end of code added 1/2/2005

			//we don't have to delete v because
			//it is directly assigned to the
			//object's field

			//code changed 3/2/2005
			//delete objref ;
			//end of code changed 3/2/2005

//code added 22/5/2005
#if DEBUG_LEVEL >= 3
			//cout << "putfield: " << objref->ref_value->type->name << endl << flush ;
			//objref->ref_value->display() ;
			//cout << "putfield: " << f->name << endl << flush ;
			//v->display() ;
#endif			
//end of cod added 22/5/2005			
			
			threads.at(c)->pc += 3 ;
			break ;
		}

		case 179: //putstatic: set static field in class
		{
			int indexbyte1 = current_method->code.byte_code.at(prog_counter+1) ;
			int indexbyte2 = current_method->code.byte_code.at(prog_counter+2) ;

			int rcp_index = indexbyte1 * 256 + indexbyte2 ;

			field *f ;

			try
			{
				f = resolve_field(this_class,
						  curr_frame->rcp->at(rcp_index-1)->field_parent_class_name,			
						  curr_frame->rcp->at(rcp_index-1)->field_name,
						  curr_frame->rcp->at(rcp_index-1)->field_descriptor,
						  true) ;
			}
			catch(LinkageError)
			{
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}
			catch(NoSuchFieldError)
			{
				throw_exception("java/lang/NoSuchFieldError") ;
				return ;
			}

			if(f->blnStatic == false)
			{
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}

			if(f->blnFinal == true && f->parent_class != this_class)
			{
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}

			if(f->parent_class->blnInitialised == false)
			{
				InitialiseClass(f->parent_class) ;

				break ;
			}

			value *v = op_st->top() ;
			op_st->pop() ;

			//code changed 3/2/2005
			//delete f->parent_class->static_fields[f->name] ;
			//end of code changed 3/2/2005

			static_value *sv = new static_value() ;

			sv->value_type = v->value_type;

			sv->IsNaN = v->IsNaN;		
			sv->IsInfinity = v->IsInfinity ;	
			sv->IsPositive = v->IsPositive ;	

			sv->bool_value = v->bool_value;
			sv->i_value = v->i_value ;
			sv->long_value = v->long_value ;
			sv->float_value = v->float_value ;
			sv->double_value = v->double_value ;
			sv->ret_add_value = v->ret_add_value ;
			sv->ref_value = v->ref_value ;

			sv->type = f->parent_class ;
			sv->descriptor = f->get_descriptor() ;

			f->parent_class->static_fields[f->name] = sv ;

			//we don't have to delete v because
			//it is directly assigned to the
			//object's field

			threads.at(c)->pc += 3 ;
			break ;
		}

		case 169: //ret: return from subroutine
		{
			int index = current_method->code.byte_code.at(prog_counter+1) ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
			assert(index >= 0 && index < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

			threads.at(c)->pc += curr_frame->local_variables.at(index)->ret_add_value ;

//code added 30/4/2005
#if DEBUG_LEVEL >= 3
			 cout << "ret" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 			   			
			break ;
		}

		case 177: //return: void from method
		{
			//if the method is a synchronised method,
			//the object's monitor should be owned
			//by the current thread

			//code added 15/1/2005
			//if(current_method->blnSynchronized == true)			
			if(current_method->blnSynchronized == true && curr_frame->mon_obj != NULL)
			//end of code added 15/1/2005
			{
				object *mon_obj = curr_frame->mon_obj ;

				if(mon_obj->mon.owner_thread_id !=
				   threads.at(c)->thread_id)
				{
					throw_exception("java/lang/IllegalMonitorStateException") ;
					return ;
				}

				mon_obj->mon.decrement_count() ;
			}

			//pop the operand stack of the
			//current frame
			while(!op_st->empty())
			{
				//code changed 3/2/2005
				//delete op_st->top() ;
				//end of code changed 3/2/2005
				
				op_st->pop() ;
			}

			//pop the current frame from the
			//currrent thread's stack
			threads.at(c)->java_stack.pop() ;

			//get the next instruction to 
			//execute (this was stored when
			//the frame was created)
			threads.at(c)->pc = curr_frame->prev_pc_value ;

			//code added 2/5/2005
			if(current_method->name == "<init>" && 
			   current_method->type->name == "java/lang/String")
			{
				object *string_object = curr_frame->local_variables.at(0)->ref_value ;
				
				string_object->blnStringObject = true ;
				string_object->string_val = "" ;
				string_object->data.clear() ;

				for(int i = 0; i < string_object->fields["value"]->ref_value->primitives.size(); i++)
				{
					char ccc = string_object->fields["value"]->ref_value->primitives.at(i)->i_value ;
					string_object->string_val += ccc ;
					string_object->data.push_back(ccc) ;
				}

				strings[string_object->string_val] = string_object ;
			}

			//TO DO:we are assuming that the newly constructed string object
			//refers to a new string that is not currently in existence
			//and are adding this object to the 'strings' map.
			//we neeed to make this check, and if the string is already
			//in existence, delete the newly created object (this is
			//actually an inefficient way of doing it, since the whole
			//point of the <init> is wasted) and reuse
			//the object already present in the 'strings' map
			//end of code added 2/5/2005

//code added 30/4/2005
#if DEBUG_LEVEL >= 1
			 cout << "return" << "(" << curr_frame << ")" << endl << flush ;
#endif			 
//end of code added 30/4/2005			 			   						
			
			//discard the current frame
#if DEBUG_LEVEL >= 2
			cout << "deleting frame " << curr_frame << endl ;
#endif
			delete curr_frame ;


			
			break ;
		}

		case 53: //saload: load short from array
		{
			value *index = op_st->top() ;
			op_st->pop() ;

			value *arrayref = op_st->top() ;
			if(arrayref->ref_value->blnNull == true)
			{
				delete index ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}
			op_st->pop() ;

			if(index->i_value < 0 ||
			   //code changed 6/3/2005
			   //index->i_value >= arrayref->ref_value->items.size())
			   index->i_value >= arrayref->ref_value->primitives.size())
			   //end of code changed 6/3/2005
			{
				delete index ;
				delete arrayref ;
				throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				return ;
			}

			value *obj_val = createValue() ;

			obj_val->value_type = value::SHORT ;
			obj_val->i_value = arrayref->ref_value->primitives.at(index->i_value)->i_value ;

			op_st->push(obj_val) ;

			//code changed 3/2/2005
			//delete index ;
			//delete arrayref ;
			//end of code changed 3/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 86: //sastore: store into short array
		{
			value *obj_val = op_st->top() ;
			op_st->pop() ;

			value *index = op_st->top() ;
			op_st->pop() ;

			value *arrayref = op_st->top() ;
			if(arrayref->ref_value->blnNull == true)
			{
				delete obj_val ;
				delete index ;
				throw_exception("java/lang/NullPointerException") ;
				return ;
			}
			op_st->pop() ;

			if(index->i_value < 0 ||
			   //code changed 6/3/2005
			   //index->i_value >= arrayref->ref_value->items.size())
			   index->i_value >= arrayref->ref_value->primitives.size())
			   //end of code changed 6/3/2005
			{
				delete obj_val ;
				delete index ;
				delete arrayref ;
				throw_exception("java/lang/ArrayIndexOutOfBoundsException") ;
				return ;
			}

			primitive_value *pv = createPrimitiveValue() ;

			pv->value_type = primitive_value::SHORT ;
			pv->i_value = obj_val->i_value ;

			//delete arrayref->ref_value->primitives.at(index->i_value) ;
			arrayref->ref_value->primitives.at(index->i_value) = pv;

			//code changed 3/2/2005
			//delete obj_val ;
			//delete index ;
			//delete arrayref ;
			//end of code changed 3/2/2005

			threads.at(c)->pc++ ;
			break ;
		}

		case 17: //sipush: push short
		{
			int byte1 = current_method->code.byte_code.at(prog_counter+1) ;
			int byte2 = current_method->code.byte_code.at(prog_counter+2) ;

			int short_val = byte1 *256 + byte2 ;

			value *v = createValue() ;
			v->value_type = value::INT ;
			v->i_value = short_val ;

			op_st->push(v) ;

			threads.at(c)->pc += 3 ;
			break ;
		}

		case 95: //swap: swap the top two operand stack values
		{
			value *v1 = op_st->top() ;
			op_st->pop() ;

			value *v2 = op_st->top() ;
			op_st->pop() ;

			op_st->push(v1) ;
			op_st->push(v2) ;

			threads.at(c)->pc++ ;
			break ;
		}

		case 170: //tableswitch: access jump table by index and jump
		{

			//find the nearest opcode that is a 
			//multiple of 4 (this is used to
			//calculate the number of padding bytes)
			int pc = threads.at(c)->pc ;

			int padbytes = 4 - ((pc+1) % 4) ;

			int def1 = current_method->code.byte_code.at(prog_counter+padbytes+1) ;
			int def2 = current_method->code.byte_code.at(prog_counter+padbytes+2) ;
			int def3 = current_method->code.byte_code.at(prog_counter+padbytes+3) ;
			int def4 = current_method->code.byte_code.at(prog_counter+padbytes+4) ;

			int defaddress = def1 * 256*256 +
					 def2 * 256*16  +
					 def3 * 256     +
					 def4 ;

			int lowbyte1 = current_method->code.byte_code.at(prog_counter+padbytes+5) ;
			int lowbyte2 = current_method->code.byte_code.at(prog_counter+padbytes+6) ;
			int lowbyte3 = current_method->code.byte_code.at(prog_counter+padbytes+7) ;
			int lowbyte4 = current_method->code.byte_code.at(prog_counter+padbytes+8) ;

			int low = lowbyte1 * 256*256 +
				  lowbyte2 * 256*16  +
				  lowbyte3 * 256     +
				  lowbyte4 ;

			int highbyte1 = current_method->code.byte_code.at(prog_counter+padbytes+9) ;
			int highbyte2 = current_method->code.byte_code.at(prog_counter+padbytes+10) ;
			int highbyte3 = current_method->code.byte_code.at(prog_counter+padbytes+11) ;
			int highbyte4 = current_method->code.byte_code.at(prog_counter+padbytes+12) ;

			int high = highbyte1 * 256*256 +
				   highbyte2 * 256*16  +
				   highbyte3 * 256     +
				   highbyte4 ;

			vector<int> jump_table ;

			//TO DO: check whether the '+ 1' goes here
			//or at the computation of padbytes
			pc += padbytes + 12 + 1; //this will give the start of the match-offsets

			for(int i=0; i<(high - low + 1); i++)
			{

				//does match take up four bytes?
				int jumpbyte1 = current_method->code.byte_code.at(pc + i*4) ;
				int jumpbyte2 = current_method->code.byte_code.at(pc + i*4 + 1) ;
				int jumpbyte3 = current_method->code.byte_code.at(pc + i*4 + 2) ;
				int jumpbyte4 = current_method->code.byte_code.at(pc + i*4 + 3) ;

				int jump_val = jumpbyte1 * 256*256 +
					       jumpbyte2 * 256*16  +
					       jumpbyte3 * 256     +
					       jumpbyte4 ;

				jump_table.push_back(jump_val) ;	

			}

			value *v = op_st->top() ;
			op_st->pop() ;

			if(v->i_value < low || v->i_value > high)
				threads.at(c)->pc += defaddress ;
			else
				threads.at(c)->pc += jump_table.at(v->i_value - low) ;

			break ;
		}

		case 196: //wide: extend local variable index by additional bytes
		{
			int opcode = current_method->code.byte_code.at(prog_counter+1) ;

			if((opcode >= 21 && opcode <= 25) ||		//load
			   (opcode >= 54 && opcode <= 58) ||		//store
			   opcode == 169)				//ret
			{
				int indexbyte1 = current_method->code.byte_code.at(prog_counter+2) ;
				int indexbyte2 = current_method->code.byte_code.at(prog_counter+3) ;

				int index = indexbyte1 * 256 + indexbyte2 ;

				if(opcode == 21)
				{
					iload(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 23)
				{
					fload(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 25)
				{
					aload(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 22)
				{
					aload(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 24)
				{
					dload(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 54)
				{
					istore(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 56)
				{
					fstore(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 58)
				{
					astore(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 55)
				{
					astore(index) ;
					threads.at(c)->pc += 4 ;
				}
				else if(opcode == 57)
				{
					dstore(index) ;
					threads.at(c)->pc += 4 ;
				}
				else
				{

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
				  	assert(index >= 0 && index < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005
					threads.at(c)->pc += curr_frame->local_variables.at(index)->ret_add_value ;
				}
			}
			else //it is iinc
			{
				int indexbyte1 = current_method->code.byte_code.at(prog_counter+2) ;
				int indexbyte2 = current_method->code.byte_code.at(prog_counter+3) ;

				int index = indexbyte1 * 256 + indexbyte2 ;

				int constbyte1 = current_method->code.byte_code.at(prog_counter+4) ;
				int constbyte2 = current_method->code.byte_code.at(prog_counter+5) ;

				int constant = constbyte1 * 256 + constbyte2 ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
				assert(index >= 0 && index < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

				LocalVariable *lv = curr_frame->local_variables.at(index) ;

				lv->i_value += constant ;

				threads.at(c)->pc += 6 ;
			}

			break ;

		}
        default:
        {
            //invalid instruction
            assert(false) ;
        }

	} //end of switch

 #if DEBUG_LEVEL >= 3
 	//Logger::LogFunctionExit("jvm::execute_instruction(): " + message) ;
 #endif 	

	return ;
}

void jvm::aload(int n)
{

	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

//code added 20/11/2004
#if DEBUG_LEVEL >= 3
	//commented out to circumvent the problem with
	//loading the VMRuntime.so
	//assert(curr_frame->local_variables.at(n)->value_type == value::REFERENCE) ;
#endif
//end of code added 20/11/2004

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	LocalVariable *lv = curr_frame->local_variables.at(n) ;

//code added 15/1/2005
#if DEBUG_LEVEL >= 3
	//commented out for the same reason related to dlopen() problem
	//assert(lv->value_type == value::REFERENCE) ;
	assert(lv->ref_value != NULL) ;

	//if(lv->ref_value->blnArrayObject != true)
	//	assert(lv->ref_value->type != NULL) ;
	//else
	//	assert(lv->ref_value->array_component_type != NULL) ;
#endif
//end of code added 15/1/2005

	value *obj_val = createValue() ;

	obj_val->value_type = value::REFERENCE ;

//code added 11/2/2005
#if DEBUG_LEVEL >= 3
	assert(lv->ref_value != NULL) ;
#endif
//end of code added 11/2/2005

	obj_val->ref_value = lv->ref_value ;

	op_st->push(obj_val) ;

#if DEBUG_LEVEL >= 3
	//if(obj_val->ref_value->blnStringObject != true)
	//	assert(obj_val->ref_value->type != NULL) ;
#endif

//code added 28/5/2005
#if DEBUG_LEVEL >= 3
	cout << "aload(): value: " << obj_val << endl << flush ;
	cout << "object: " << obj_val->ref_value << "; " ;
	//obj_val->display() ;
	cout << "index: " << n << endl << flush ;
#endif
//end of code added 28/5/2005

	return ;

}

void jvm::astore(int n)
{

	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

	value *obj_val = op_st->top() ;
	op_st->pop() ;

	LocalVariable *lv = create_local_variable(obj_val) ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
    assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

        curr_frame->local_variables.at(n) = lv ;

	//code changed 3/2/2005
	//delete obj_val ;
	//end of code changed 3/2/2005

//code added 28/5/2005
#if DEBUG_LEVEL >= 3
	cout << "astore(): value: " << obj_val << endl << flush ;
	obj_val->display() ;
	cout << "index: " << n << endl << flush ;
#endif
//end of code added 28/5/2005

	return ;

}

void jvm::athrow()
{

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionEntry("jvm::athrow()") ;
        FunctionCallLogger fcl("jvm::athrow()") ;
#endif
//end of code added 13/11/2004	
	
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;
	method *current_method = curr_frame->curr_method ;
	Class *this_class = current_method->type ;

	value *obj_val = op_st->top() ;

	string exception_name ;

	if(threads.at(c)->blnExceptionBeingThrown == false)
		threads.at(c)->blnExceptionBeingThrown = true ;

	//if the current method is a native method,
	//pop the current frame and rethrow the exception
	if(current_method->blnNative == true)
	{
		//pop the operand stack of the
		//current frame
		while(!op_st->empty())
		{
			//code changed 3/2/2005
			//delete op_st->top() ;
			//end of code changed 3/2/2005
			
			op_st->pop() ;
		}

		//pop the current frame from the
		//currrent thread's stack
		threads.at(c)->java_stack.pop() ;

		//get the next instruction to 
		//execute (this was stored when
		//the frame was created)
		//if the previous method was a native method,
		//this will not make sense and is not actually needed
		threads.at(c)->pc = curr_frame->prev_pc_value ;

		//discard the current frame
		delete curr_frame ;

		//if an invoking frame exists
		if(!threads.at(c)->java_stack.empty())
		{
			//push the object reference onto
			//the operand stack of the invoking frame
			frame *inv_fr = threads.at(c)->java_stack.top() ;
			inv_fr->operand_stack.push(obj_val) ;

			//rethrow the exception
			athrow() ;
		}
		else //no invoking frame exists
		{
                        string tweakedExceptionName = exception_name ;
                        util::replace_substring(tweakedExceptionName,"/",".") ;
			cout << "Exception in thread '" << threads.at(c)->name << "': " << tweakedExceptionName << endl ;
			threads.erase(threads.begin()+c) ;
		}

	} //end of if(current_method->blnNative == true)

	//code changed 14/5/2005
	//if(current_method->name != "<clinit>" &&
	//	current_method->name != "<init>")
	//{
	//end of code changed 14/5/2005

		string trace_string ;		

		char temp_string1[10] ;

		sprintf(temp_string1,"%d",current_method->get_line_number(threads.at(c)->pc)) ;

		string temp_string2(temp_string1) ;

                string tweakedClassName = this_class->name ;
                util::replace_substring(tweakedClassName, "/", ".") ;


		if(this_class->source_file_name.length() > 0) 
		{
			trace_string = string("        at " + tweakedClassName + "." + 
							current_method->name + "(" +
							this_class->source_file_name + ":" + temp_string2 + ")") ;
		}
		else
		{
			trace_string = string("        at " + tweakedClassName + "." + 
							current_method->name + "(" +
							"compiled code:" + temp_string2 + ")") ;
		}

		threads.at(c)->exception_trace.push(trace_string) ;
		
	//code changed 14/5/2005
	//}
	//end of code changed 14/5/2005

	op_st->pop() ;

	exception_name = obj_val->ref_value->type->name ;

	if(obj_val->ref_value->blnNull == true)
	{
		delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::athrow() - java/lang/NullPointerException") ;
#endif
//end of code added 13/11/2004	
		throw_exception("java/lang/NullPointerException") ;
		return ;
	}

	bool blnHandlerFound = false ;
	threads.at(c)->exception_object = obj_val->ref_value ;

	//search the current method's exception handler
	//to check if this object's type is handled
	for(int i=0; i <current_method->code.ex_tbl.size(); i++)
	{
		if(threads.at(c)->pc >= current_method->code.ex_tbl.at(i)->start_pc &&
		   threads.at(c)->pc < current_method->code.ex_tbl.at(i)->end_pc)
		{
			int ct = current_method->code.ex_tbl.at(i)->catch_type ;

			try
			{
				Class *excp ;
				if(curr_frame->rcp->at(ct-1)->blnResolved == true)
					excp = curr_frame->rcp->at(ct-1)->resolved_class ;
				else
				{
					excp = resolve_class(this_class,
							     this_class->rcp.at(ct-1)->class_interface_name) ;

					 //code added 28/2/2005
#if DEBUG_LEVEL >= 3
					 assert(excp != NULL) ;
#endif
					 //end of code added 28/2/2005

					curr_frame->rcp->at(ct-1)->blnResolved = true ;
					curr_frame->rcp->at(ct-1)->resolved_class = excp ;
				}

				if(obj_val->ref_value->type == excp ||
			   	IsSuperClass(excp, obj_val->ref_value->type) == true)
				{

					//set current thread's PC register
					//to new location
					threads.at(c)->pc = current_method->code.ex_tbl.at(i)->handler_pc ;

					//clear the operand stack
					while(!op_st->empty())
					{
						//code changed 3/2/2005
						//delete op_st->top() ;
						//end of code changed 3/2/2005
						
						op_st->pop() ;
					}

					//push object reference onto the
					//operand stack
					op_st->push(obj_val) ;

					blnHandlerFound = true ;
					threads.at(c)->blnExceptionBeingThrown = false ;
					break ; //break out of for loop
				} //end if

			}
			catch(LinkageError)
			{
				delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
				//Logger::LogFunctionExit("jvm::athrow() - java/lang/LinkageError") ;
#endif
//end of code added 13/11/2004				
				throw_exception("java/lang/LinkageError") ;
				return ;
			}
			catch(NoClassDefFound)
			{
				delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
				//Logger::LogFunctionExit("jvm::athrow() - java/lang/NoClassDefFoundError") ;
#endif
//end of code added 13/11/2004				
				throw_exception("java/lang/NoClassDefFoundError") ;
				return ;
			}
			catch(FormatError)
			{
				delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
				//Logger::LogFunctionExit("jvm::athrow() - java/lang/FormatError") ;
#endif
//end of code added 13/11/2004
				throw_exception("java/lang/FormatError") ;
				return ;
			}
			catch(ClassCircularityError)
			{
				delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
				//Logger::LogFunctionExit("jvm::athrow() - java/lang/ClassCircularityError") ;
#endif
//end of code added 13/11/2004
				throw_exception("java/lang/ClassCircularityError") ;
				return ;
			}
			catch(IncompatibleClassChangeError)
			{
				delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
				//Logger::LogFunctionExit("jvm::athrow() - java/lang/IncompatibleClassChangeError") ;
#endif
//end of code added 13/11/2004
				throw_exception("java/lang/IncompatibleClassChangeError") ;
				return ;
			}
			catch(IllegalAccessError)
			{
				delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
				//Logger::LogFunctionExit("jvm::athrow() - java/lang/IllegalAccesError") ;
#endif
//end of code added 13/11/2004
				throw_exception("java/lang/IllegalAccessError") ;
				return ;
			}

		} //end if
	} //end for

	//no matching exception handler found
	if(blnHandlerFound == false)
	{

		//if the method is a synchronised method,
		//the object's monitor should be owned
		//by the current thread
		if(current_method->blnSynchronized == true)
		{
			//code changed 30/7/2005
			//if(obj_val->ref_value->mon.owner_thread_id !=
		   	//   threads.at(c)->thread_id)
			if(obj_val->ref_value->mon.owner_thread_id != -1 &&
			   obj_val->ref_value->mon.owner_thread_id != threads.at(c)->thread_id)
			//end of code added 30/7/2005
			{
				delete obj_val ;
//code added 13/11/2004
#if DEBUG_LEVEL >= 3
				//Logger::LogFunctionExit("jvm::athrow() - java/lang/IllegalMonitorStateException") ;
#endif
//end of code added 13/11/2004
				throw_exception("java/lang/IllegalMonitorStateException") ;
				return ;
			}

			obj_val->ref_value->mon.decrement_count() ;
		}

		//pop the operand stack of the
		//current frame
		while(!op_st->empty())
		{
			//code changed 3/2/2005
			//delete op_st->top() ;
			//end of code changed 3/2/2005
			op_st->pop() ;
		}

		//pop the current frame from the
		//currrent thread's stack
		threads.at(c)->java_stack.pop() ;

		//get the next instruction to 
		//execute (this was stored when
		//the frame was created)
		threads.at(c)->pc = curr_frame->prev_pc_value ;

		//discard the current frame
#if DEBUG_LEVEL >= 2
		cout << "deleting frame " << curr_frame << endl ;
#endif
		delete curr_frame ;

		//if an invoking frame exists
		if(!threads.at(c)->java_stack.empty())
		{
			
			//push the object reference onto
			//the operand stack of the invoking frame
			frame *inv_fr = threads.at(c)->java_stack.top() ;
			inv_fr->operand_stack.push(obj_val) ;

			//if the invoking method is a native method,
			//do nothing, as the native method should handle the
			//exception by JNI methods (ExceptionOccured, ExceptionClear, etc.)
			//code changed 13/2/2005
			//if(inv_fr->curr_method->blnNative == true)
			if(inv_fr->curr_method->blnNative == true || 
			   inv_fr->blnExecutingNativeMethod == true)
			//end of code changed 13/2/2005
			{
				; //do nothing
			}
			else
			{
				//rethrow the exception
				athrow() ;
			}
		}
		else //no invoking frame exists
		{
                        string tweakedExceptionName = exception_name ;
                        util::replace_substring(tweakedExceptionName,"/",".") ;
			cout << "Exception in thread \"" << threads.at(c)->name << "\": " << tweakedExceptionName << endl ;

			//new_stack is the reversed version of
			//the thread's exception trace stack
			stack<string> new_stack ;

			util::reverse_stack(&(threads.at(c)->exception_trace), &new_stack) ;

			while(!(new_stack.empty()))
			{
				cout << new_stack.top() << endl ;
				new_stack.pop() ;
			}
			threads.erase(threads.begin()+c) ;
		}
	} //end of if(blnHandlerFound == false) ;

//code added 13/11/2004
#if DEBUG_LEVEL >= 3
	//Logger::LogFunctionExit("jvm::athrow()") ;
#endif
//end of code added 13/11/2004

	return ;
}

void jvm::dload(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	LocalVariable *lv = curr_frame->local_variables.at(n) ;
	value *obj_val = createValue() ;

	obj_val->value_type = value::DOUBLE ;
	obj_val->double_value = lv->double_value ;
	obj_val->IsNaN = lv->IsNaN ;
	obj_val->IsInfinity = lv->IsInfinity ;
	obj_val->IsPositive = lv->IsPositive ;

	op_st->push(obj_val) ;

	return ;

}

void jvm::dstore(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

	value *d_val = op_st->top() ;
	op_st->pop() ;

	LocalVariable *lv = create_local_variable(d_val) ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	curr_frame->local_variables.at(n) = lv ;

	//code changed 9/2/2005
	//delete d_val ;
	//end of code changed 9/2/2005

	return ;

}

void jvm::fload(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	LocalVariable *lv = curr_frame->local_variables.at(n) ;

//code added 4/2/2005
#if DEBUG_LEVEL >= 3
	assert(lv->value_type == value::FLOAT) ;
#endif
//end of code added 4/2/2005

	value *obj_val = createValue() ;

	obj_val->value_type = value::FLOAT ;
	obj_val->float_value = lv->float_value ;
	obj_val->IsNaN = lv->IsNaN ;
	obj_val->IsInfinity = lv->IsInfinity ;
	obj_val->IsPositive = lv->IsPositive ;

	op_st->push(obj_val) ;

	return ;

}

void jvm::fstore(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

	value *f_val = op_st->top() ;
	op_st->pop() ;

	LocalVariable *lv = create_local_variable(f_val) ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	curr_frame->local_variables.at(n) = lv ;

	//code changed 9/2/2005
	//delete f_val ;
	//end of code changed 9/2/2005

	return ;

}

void jvm::iload(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	//if(n < 0 || n >= curr_frame->local_variables.size())
	//{
	//	cout << n << " " << curr_frame->local_variables.size() << endl << flush ;
	//	cout << curr_frame->curr_method->type->name << "->" << curr_frame->curr_method->name << "()" << endl << flush ;
	//}
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	LocalVariable *lv = curr_frame->local_variables.at(n) ;

//code added 4/2/2004
#if DEBUG_LEVEL >= 3
	assert(lv != NULL) ;

	//code changed 26/3/2005
	//see comment in iadd
	//assert(lv->value_type == value::INT) ;
	//end of code changed 26/3/2005
#endif
//end of code added 4/2/2004

	value *obj_val = createValue() ;

	obj_val->value_type = value::INT ;
	obj_val->i_value = lv->i_value ;

	op_st->push(obj_val) ;

	return ;

}

void jvm::istore(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

//code added 20/2/2005
#if DEBUG_LEVEL >= 3
	assert(!op_st->empty()) ;
#endif
//end of code added 20/2/2005

	value *i_val = op_st->top() ;
	op_st->pop() ;

//code added 23/11/2004
#if DEBUG_LEVEL >= 3
	assert(i_val != NULL) ;

	//code changed 26/3/2005
	//see comment in iadd
	//assert(i_val->value_type == value::INT) ;
	//end of code changed 26/3/2005

#endif
//end of code added 23/11/2004

	LocalVariable *lv = create_local_variable(i_val) ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	curr_frame->local_variables.at(n) = lv ;

	//code changed 9/2/2005
	//delete i_val ;
	//end of code changed 9/2/2005

	return ;

}

void jvm::lload(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	LocalVariable *lv = curr_frame->local_variables.at(n) ;
	value *obj_val = createValue() ;

	obj_val->value_type = value::LONG ;
	obj_val->long_value = lv->long_value ;

	op_st->push(obj_val) ;


	return ;

}

void jvm::lstore(int n)
{
	//definitions for convenience
	int c = curr_thread ;
	stack<value *> *op_st = &(threads.at(c)->java_stack.top()->operand_stack) ;
	frame *curr_frame = threads.at(c)->java_stack.top() ;

	value *l_val = op_st->top() ;
	op_st->pop() ;

	LocalVariable *lv = create_local_variable(l_val) ;

//code added 27/3/2005
#if DEBUG_LEVEL >= 3
	assert(n >= 0 && n < curr_frame->local_variables.size()) ;
#endif
//end of code added 27/3/2005

	curr_frame->local_variables.at(n) = lv ;

	//code changed 9/2/2005
	//delete l_val ;
	//end of code changed 9/2/2005

	return ;

}

//code added 25/2/2005
void jvm::setup_string_init(object *strobj, string s)
{

  typedef map<key, Class *>::const_iterator CI ;

  CI ci = classes.find(key(NULL,"java/lang/String")) ;

  assert(ci != classes.end()) ;

  Class *c = classes[ci->first] ;

  map<string, method *>::const_iterator p1 = c->methods.find("<init> %% ([C)V") ;

  method *m = c->methods[p1->first] ;

  //code changed 25/7/2005
  //frame *fm = new frame() ;
  //fm->rcp = &(c->rcp) ;
  //fm->curr_method =  m;
  frame *fm = new frame(m) ;
  //end of code changed 25/7/2005

  fm->prev_pc_value = threads.at(curr_thread)->pc ;

  fm->local_variables.reserve(m->max_locals + 1) ;

  threads.at(curr_thread)->java_stack.push(fm) ;
  threads.at(curr_thread)->pc = 0 ;	

  value *v =new value ;

  v->value_type = value::REFERENCE ;
  v->ref_value = strobj ;

  fm->local_variables.push_back(create_local_variable(v)) ;

  object *char_array = createObject() ;

  char_array->blnArrayObject = true ;

  char_array->blnPrimitiveArrayObject = true ;

  //code changed 1/5/2005 (Proletarians unite!)
  //array_component_type is NOT String!
  //char_array->array_component_type = c ;
  //end of code changed 1/5/2005

  char_array->primitive_type = primitive_value::CHAR ;

  for(int i=0; i<s.length(); i++)
    {
      primitive_value *pv = createPrimitiveValue() ;

      pv->value_type = primitive_value::CHAR ;
      pv->i_value = s.at(i) ;

      char_array->primitives.push_back(pv) ;

    }

  value *array_val = createValue() ;
  array_val->value_type = value::REFERENCE ;
  array_val->ref_value = char_array ;

  fm->local_variables.push_back(create_local_variable(array_val)) ;
  
}
//end of code added 25/2/2005
