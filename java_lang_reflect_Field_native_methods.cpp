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

#include "java_lang_reflect_Field.h"

#include "jvm.h"

#include "util.h"

#include <cassert>

#define theJVM ((MyJNINativeInterface_ *)(env->functions))->jvm_ptr

typedef map<string, static_value *>::const_iterator CI ;

typedef map<string, value *>::const_iterator CI1 ;

using namespace std ;

JNIEXPORT jboolean JNICALL Java_java_lang_reflect_Field_equals(JNIEnv *env, jobject this_obj, jobject that_obj)
{
  field *this_field = static_cast<field *>(this_obj) ;

  field *that_field = static_cast<field *>(that_obj) ;

  if(this_field->parent_class == that_field->parent_class &&
     this_field->name == that_field->name &&
     this_field->get_descriptor() == that_field->get_descriptor())
    return JNI_TRUE ;
  else
    return JNI_FALSE ;

}

JNIEXPORT jobject JNICALL Java_java_lang_reflect_Field_get(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: wrapping primitives into Objects; currently
  //only objects will be returned

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jobject>(cls->static_fields[ci->first]->ref_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return NULL ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return NULL ;
    }

  //TO DO: verify access control

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jobject>(obj1->fields[ci1->first]->ref_value) ;

}

JNIEXPORT jboolean JNICALL Java_java_lang_reflect_Field_getBoolean(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jboolean>(cls->static_fields[ci->first]->bool_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jboolean>(obj1->fields[ci1->first]->bool_value) ;

}

JNIEXPORT jbyte JNICALL Java_java_lang_reflect_Field_getByte(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jbyte>(cls->static_fields[ci->first]->i_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jbyte>(obj1->fields[ci1->first]->i_value) ;

}

JNIEXPORT jchar JNICALL Java_java_lang_reflect_Field_getChar(JNIEnv *env, jobject  fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jchar>(cls->static_fields[ci->first]->i_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jchar>(obj1->fields[ci1->first]->i_value) ;

}

JNIEXPORT jclass JNICALL Java_java_lang_reflect_Field_getDeclaringClass(JNIEnv *env, jobject fldobj)
{

  field *fld = static_cast<field *>(fldobj) ;

  return static_cast<jclass>(fld->parent_class) ;

}

JNIEXPORT jdouble JNICALL Java_java_lang_reflect_Field_getDouble(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jdouble>(cls->static_fields[ci->first]->double_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jdouble>(obj1->fields[ci1->first]->double_value) ;

}

JNIEXPORT jfloat JNICALL Java_java_lang_reflect_Field_getFloat(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jfloat>(cls->static_fields[ci->first]->float_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jfloat>(obj1->fields[ci1->first]->float_value) ;

}

JNIEXPORT jint JNICALL Java_java_lang_reflect_Field_getInt(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jint>(cls->static_fields[ci->first]->i_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jint>(obj1->fields[ci1->first]->i_value) ;

}

JNIEXPORT jlong JNICALL Java_java_lang_reflect_Field_getLong(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jlong>(cls->static_fields[ci->first]->long_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jlong>(obj1->fields[ci1->first]->long_value) ;

}

JNIEXPORT jint JNICALL Java_java_lang_reflect_Field_getModifiers(JNIEnv *env, jobject fldobj)
{

  field *fld = static_cast<field *>(fldobj) ;

  int modifier = 0 ;

  if(fld->blnPublic == true)
    modifier += 0x0001 ;	

  if(fld->blnPrivate == true)
    modifier += 0x0002 ;

  if(fld->blnProtected == true)
    modifier += 0x0004 ;

  if(fld->blnStatic == true)
    modifier += 0x0008 ;

  if(fld->blnFinal == true)
    modifier += 0x0010 ;

  if(fld->blnVolatile == true)
    modifier += 0x0040 ;

  if(fld->blnTransient == true)
    modifier += 0x0080 ;

  return modifier ;

}

JNIEXPORT jstring JNICALL Java_java_lang_reflect_Field_getName(JNIEnv *env, jobject fldobj)
{

  field *fld = static_cast<field *>(fldobj) ;

  string s1(fld->name) ;

  return env->NewString(s1.data(), s1.length()) ;

}

JNIEXPORT jshort JNICALL Java_java_lang_reflect_Field_getShort(JNIEnv *env, jobject fldobj, jobject obj)
{

  //TO DO: verify access control

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      return static_cast<jlong>(cls->static_fields[ci->first]->long_value) ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return 0 ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return 0 ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  return static_cast<jshort>(obj1->fields[ci1->first]->i_value) ;

}

JNIEXPORT jclass JNICALL Java_java_lang_reflect_Field_getType(JNIEnv *env, jobject fldobj)
{

  field *fld = static_cast<field *>(fldobj) ;

  //the 'descriptor' member of the 'field' class contains
  //this information (4.3.2 in the VM Specs). this has to 
  //be converted to a 'Class' object by referring to the 
  //'Classes' member of the 'theJVM' object

  return util::getClassFromName(theJVM, 
				fld->get_descriptor(),
				fld->parent_class->defining_loader) ;

}

JNIEXPORT jint JNICALL Java_java_lang_reflect_Field_hashCode(JNIEnv *env, jobject fldobj)
{
  //we simply return the pointer value of the object
  return (int)(fldobj) ;
}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_set(JNIEnv *env, jobject fldobj, jobject obj, jobject val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  object *objval = static_cast<object *>(val) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      delete cls->static_fields[ci->first]->ref_value ;

      cls->static_fields[ci->first]->ref_value = objval ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  //TO DO: handle primitives (unwrapping, etc.)

  //we are not handling identity/widening conversion

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  delete obj1->fields[ci1->first]->ref_value ;

  obj1->fields[ci1->first]->ref_value = objval ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setBoolean(JNIEnv *env, jobject fldobj, jobject obj, jboolean val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->bool_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->bool_value = val ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setByte(JNIEnv *env, jobject fldobj, jobject obj, jbyte val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->i_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->i_value = val ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setChar(JNIEnv *env, jobject fldobj, jobject obj, jchar val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->i_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->i_value = val ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setDouble(JNIEnv *env, jobject fldobj, jobject obj, jdouble val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->double_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->double_value = val ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setFloat(JNIEnv *env, jobject fldobj, jobject obj, jfloat val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->float_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->float_value = val ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setInt(JNIEnv *env, jobject fldobj, jobject obj, jint val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->i_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->i_value = val ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setLong(JNIEnv *env, jobject fldobj, jobject obj, jlong val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->long_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->long_value = val ;

  return ;

}

JNIEXPORT void JNICALL Java_java_lang_reflect_Field_setShort(JNIEnv *env, jobject fldobj, jobject obj, jshort val)
{

  field *fld = static_cast<field *>(fldobj) ;

  object *obj1 = static_cast<object *>(obj) ;

  if(fld->blnStatic == true)
    {
      Class *cls = fld->parent_class ;

      if(cls->blnInitialised == false)
	{
	  theJVM->InitialiseClass(cls) ;
	}

      CI ci = cls->static_fields.find(fld->name) ;  

      cls->static_fields[ci->first]->i_value = val ;

      return ;

    }

  if(obj1->blnNull == true)
    {
      theJVM->throw_exception("java/lang/NullPointerException") ;
      return ;
    }

  if(obj1->type != fld->parent_class)
    {
      theJVM->throw_exception("java/lang/IllegalArgumentException") ;
      return ;
    }

  //TO DO: verify access control
  if(fld->blnFinal == true)
    {
      theJVM->throw_exception("java/lang/IllegalAccessException") ;
      return ;
    }

  CI1 ci1 = obj1->fields.find(fld->name) ;  

  obj1->fields[ci1->first]->i_value = val ;

  return ;

}

JNIEXPORT jstring JNICALL Java_java_lang_reflect_Field_toString(JNIEnv *env, jobject fldobj)
{

  field *fld = static_cast<field *>(fldobj) ;

  string str_val ;

  if(fld->blnPublic == true)
    str_val = "public ";	
  else if(fld->blnPrivate == true)
    str_val = "private " ;
  else if(fld->blnProtected == true)
    str_val = "protected " ;

  if(fld->blnStatic == true)
    str_val += "static ";
  else if(fld->blnFinal == true)
    str_val += "final ";
  else if(fld->blnVolatile == true)
    str_val += "volatile ";
  else if(fld->blnTransient == true)
    str_val += "transient ";

  string fld_name = fld->name ;

  util::replace_substring(fld_name, "/", ".") ;

  str_val += fld_name ;

  string temp_str = fld->get_descriptor() ;

  if(temp_str == "B")
    str_val += "byte" ;
  else if(temp_str == "C")
    str_val += "char" ;
  else if(temp_str == "D")
    str_val += "double" ;
  else if(temp_str == "F")
    str_val += "float" ;
  else if(temp_str == "I")
    str_val += "int" ;
  else if(temp_str == "J")
    str_val += "long" ;
  else if(temp_str == "S")
    str_val += "short" ;
  else if(temp_str == "Z")
    str_val += "boolean" ;
  else if(temp_str.at(0) == 'L') //i.e. it is a reference type
    {

      string ref_type = temp_str.substr(1,temp_str.length()-2) ; //we need to omit the ';' at the end
	  
      util::replace_substring(ref_type, "/", ".") ;

      str_val += ref_type ;

    }
  else if(temp_str.at(0) == '[') // array
    {
      string array_type ;

      //count the dimensions
      int dims = 1 ;

      for(int j=1; j<temp_str.length(); j++)
	{
	  if(temp_str.at(j) == '[')
	    dims++ ;
	  else
	    break ;
	}

      array_type = temp_str.substr(dims,temp_str.length()-1) ;

      if(array_type == "B")
	str_val += "byte" ;
      else if(array_type == "C")
	str_val += "char" ;
      else if(array_type == "D")
	str_val += "double" ;
      else if(array_type == "F")
	str_val += "float" ;
      else if(array_type == "I")
	str_val += "int" ;
      else if(array_type == "J")
	str_val += "long" ;
      else if(array_type == "S")
	str_val += "short" ;
      else if(array_type == "Z")
	str_val += "boolean" ;
      else if(array_type.at(0) == 'L') //i.e. it is a reference type
	{

	  //again omitting the ';' at the end	      
	  string array_type2 = array_type.substr(1,array_type.length()-2) ;

	  util::replace_substring(array_type2,"/",".") ;

	  str_val += array_type2 ;

	}

      for(int j=0; j<dims; j++)
	str_val += "[]" ;
      
    }

  //TO DO: check if the class name needs to be tweaked
  //(to remove "/", for example)
  str_val += fld->parent_class->name ;

  str_val += "." + fld->name ;
	
  return env->NewString(str_val.data(), str_val.length()) ;

}
