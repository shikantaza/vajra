#IMPORTANT: Ensure that VAJRA_CLASSPATH has been set correctly before running 'make'

#Use Sun compiler/runtime
#VAJRA_CLASSPATH=/home/rajesh/j2sdk1.4.2_04/jre/lib/rt.jar:/home/rajesh/eclipse/workspace/vajra/vajra/vre.jar:/home/rajesh/eclipse/workspace/vajra/vajra
#JAVAC = /home/rajesh/j2sdk1.4.2_04/bin/javac -classpath ${CLASSPATH}

#Use Classpath and Jikes
VAJRA_CLASSPATH=/home/rajesh/vajra/classpath.jar:.
JAVAC = /usr/bin/jikes -classpath ${VAJRA_CLASSPATH}

CXX=/usr/bin/g++

INCLUDEDIRS=.:/usr/include/g++:/usr/include:/usr/local/include

CXXFLAGS = -O0 -g -I${INCLUDEDIRS}

OBJS = ClassFile.o Class.o java.o jni.o jvm.o Logger.o \
              method.o monitor.o object.o \
              opcodes.o util.o ziplib.o

LIBOBJS = java_lang_Thread_native_methods.o \
		java_lang_VMSystem_native_methods.o \
		java_lang_VMSecurityManager_native_methods.o \
		java_lang_reflect_Constructor_native_methods.o \
		java_lang_reflect_Method_native_methods.o \
		java_lang_VMRuntime_native_methods.o \
		java_lang_reflect_Field_native_methods.o \
		java_lang_Runtime_native_methods.o \
		java_lang_VMObject_native_methods.o \
		java_lang_VMClass_native_methods.o \
		java_lang_VMClassLoader_native_methods.o \
		java_lang_VMThread_native_methods.o

#JAR = /home/rajesh/j2sdk1.4.2_04/bin/jar 
JAR = /usr/bin/jar 

all : vajra all_libs classes

Test : all
	./vajra Test
	./vajra Hello
	./vajra HelloWorld
	./vajra InputTest
	./vajra Fibonacci
	./vajra ArgPrinter 1 2 3 4
	./vajra FileWriter 1.txt "Hello, World!"
	cat 1.txt
	rm 1.txt

vajra : ${OBJS}
	${CXX} ${CXXFLAGS} -o vajra ${OBJS} -lz -ldl

all_libs : java_lang_Runtime.so java_lang_Thread.so \
	java_lang_reflect_Constructor.so java_lang_reflect_Field.so \
	java_lang_reflect_Method.so java_lang_VMRuntime.so \
	java_lang_VMSecurityManager.so java_lang_VMSystem.so \
	java_lang_VMObject.so java_lang_VMClass.so vajra.so

classes : Test.class HelloWorld.class Hello.class InputTest.class \
        Example1.class Fibonacci.class ArgPrinter.class FileWriter.class TestThread.class

clean : clean_vajra clean_libs clean_classes

clean_vajra :
	rm -rf vajra ${OBJS} temp.out vajra_log_*.c vajra_2.log

clean_libs :
	rm -rf java_lang_Runtime.so \
		java_lang_Thread.so java_lang_reflect_Constructor.so \
		java_lang_reflect_Field.so java_lang_reflect_Method.so \
		java_lang_VMRuntime.so java_lang_VMSecurityManager.so \
		java_lang_VMSystem.so java_lang_VMObject.so \
		java_lang_VMClass.so vajra.so
	rm -rf ${LIBOBJS}
	rm -rf ${OBJS}

clean_classes : 
	rm -rf java/lang/reflect/Constructor.class \
		java/lang/reflect/Field.class \
		java/lang/reflect/Method.class \
		Test.class HelloWorld.class Hello.class \
		InputTest.class Example1.class A.class B.class \
                Fibonacci.class ArgPrinter.class \
		FileWriter.class TestThread.class

ClassFile.o : ClassFile.h util.h logger.h exceptions.h
Class.o : vajra.h util.h
java.o : jvm.h
jni.o : jvm.h util.h
jvm.o : jvm.h ClassFile.h vajra.h util.h logger.h
Logger.o : logger.h
method.o : method.h
monitor.o : monitor.h
object.o : vajra.h
opcodes.o : jvm.h util.h
util.o : util.h
ziplib.o : ziplib.h

ClassFile.h : ziplib.h
	touch ClassFile.h

vajra.h : jni.h field.h method.h ClassFile.h monitor.h
	touch vajra.h

jni.h : jni_md.h
	touch jni.h

jvm.h : jni.h vajra.h vajra.h exceptions.h
	touch jvm.h

method.h : ClassFile.h vajra.h jni.h
	touch method.h

java_lang_VMClass.so : java_lang_VMClass_native_methods.o util.o jvm.o \
		object.o Class.o Logger.o opcodes.o ClassFile.o method.o \
		monitor.o java.o ziplib.o jni.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_VMClass.so -o \
                java_lang_VMClass.so java_lang_VMClass_native_methods.o util.o \
		jvm.o object.o Class.o Logger.o opcodes.o ClassFile.o method.o \
		monitor.o java.o ziplib.o jni.o -lc

java_lang_Runtime.so : java_lang_Runtime_native_methods.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_Runtime.so -o \
                java_lang_Runtime.so java_lang_Runtime_native_methods.o -lc

java_lang_Thread.so : java_lang_Thread_native_methods.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_Thread.so -o \
                java_lang_Thread.so java_lang_Thread_native_methods.o -lc

java_lang_reflect_Constructor.so : java_lang_reflect_Constructor_native_methods.o \
	util.o Class.o ClassFile.o method.o jvm.o monitor.o Logger.o java.o \
	ziplib.o object.o opcodes.o jni.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_reflect_Constructor.so -o \
                java_lang_reflect_Constructor.so java_lang_reflect_Constructor_native_methods.o \
		util.o Class.o ClassFile.o method.o jvm.o monitor.o Logger.o java.o \
		ziplib.o object.o opcodes.o jni.o -lc

java_lang_reflect_Field.so : java_lang_reflect_Field_native_methods.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_reflect_Field.so -o \
                java_lang_reflect_Field.so java_lang_reflect_Field_native_methods.o -lc

java_lang_reflect_Method.so : java_lang_reflect_Method_native_methods.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_reflect_Method.so -o \
                java_lang_reflect_Method.so java_lang_reflect_Method_native_methods.o -lc

java_lang_VMRuntime.so : java_lang_VMRuntime_native_methods.o jvm.o ClassFile.o ziplib.o \
                         object.o Class.o java.o Logger.o monitor.o opcodes.o util.o method.o jni.o 
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_VMRuntime.so -o \
                java_lang_VMRuntime.so java_lang_VMRuntime_native_methods.o jvm.o ClassFile.o ziplib.o \
                object.o Class.o java.o Logger.o monitor.o opcodes.o util.o method.o jni.o -lc

java_lang_VMSecurityManager.so : java_lang_VMSecurityManager_native_methods.o object.o monitor.o \
                                 Class.o ClassFile.o ziplib.o java.o Logger.o util.o method.o jni.o jvm.o opcodes.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_VMSecurityManager.so -o \
                java_lang_VMSecurityManager.so java_lang_VMSecurityManager_native_methods.o \
                object.o monitor.o Class.o ClassFile.o ziplib.o java.o Logger.o util.o method.o jni.o jvm.o opcodes.o -lc

java_lang_VMSystem.so : java_lang_VMSystem_native_methods.o object.o monitor.o 
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_VMSystem.so -o \
                java_lang_VMSystem.so java_lang_VMSystem_native_methods.o object.o monitor.o -lc

java_lang_VMObject.so : java_lang_VMObject_native_methods.o object.o monitor.o 
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,java_lang_VMObject.so -o \
                java_lang_VMObject.so java_lang_VMObject_native_methods.o object.o monitor.o -lc

vajra.so : java_lang_VMClassLoader_native_methods.o \
		java_lang_VMThread_native_methods.o \
		util.o jvm.o object.o Class.o Logger.o opcodes.o \
		ClassFile.o method.o \
		monitor.o java.o ziplib.o jni.o
	${CXX} ${CXXFLAGS} -g -shared -Wl,-soname,vajra.so -o \
                vajra.so \
		java_lang_VMClassLoader_native_methods.o \
		java_lang_VMThread_native_methods.o \
		util.o jvm.o object.o Class.o Logger.o opcodes.o ClassFile.o method.o \
		monitor.o java.o ziplib.o jni.o -lc

java_lang_Class_native_methods.o : java_lang_Class_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_Class_native_methods.cpp -o java_lang_Class_native_methods.o

java_lang_VMClass_native_methods.o : java_lang_VMClass_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_VMClass_native_methods.cpp -o java_lang_VMClass_native_methods.o

java_lang_Runtime_native_methods.o : java_lang_Runtime_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_Runtime_native_methods.cpp -o java_lang_Runtime_native_methods.o

java_lang_Thread_native_methods.o : java_lang_Thread_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_Thread_native_methods.cpp -o java_lang_Thread_native_methods.o

java_lang_reflect_Constructor_native_methods.o : java_lang_reflect_Constructor_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_reflect_Constructor_native_methods.cpp -o java_lang_reflect_Constructor_native_methods.o

java_lang_reflect_Field_native_methods.o : java_lang_reflect_Field_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_reflect_Field_native_methods.cpp -o java_lang_reflect_Field_native_methods.o

java_lang_reflect_Method_native_methods.o : java_lang_reflect_Method_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_reflect_Method_native_methods.cpp -o java_lang_reflect_Method_native_methods.o

java_lang_VMRuntime_native_methods.o : java_lang_VMRuntime_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_VMRuntime_native_methods.cpp -o java_lang_VMRuntime_native_methods.o	

java_lang_VMSystem_native_methods.o : java_lang_VMSystem_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_VMSystem_native_methods.cpp -o java_lang_VMSystem_native_methods.o

java_lang_VMObject_native_methods.o : java_lang_VMObject_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_VMObject_native_methods.cpp -o java_lang_VMObject_native_methods.o

java_lang_VMClassLoader_native_methods.o : java_lang_VMClassLoader_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_VMClassLoader_native_methods.cpp -o java_lang_VMClassLoader_native_methods.o

java_lang_VMThread_native_methods.o : java_lang_VMThread_native_methods.cpp
	${CXX} ${CXXFLAGS} -c java_lang_VMThread_native_methods.cpp -o java_lang_VMThread_native_methods.o

Test.class : Test.java
	${JAVAC} Test.java

HelloWorld.class : HelloWorld.java
	${JAVAC} HelloWorld.java

Hello.class : Hello.java
	${JAVAC} Hello.java

InputTest.class : InputTest.java
	${JAVAC} InputTest.java

Example1.class : Example1.java
	${JAVAC} Example1.java

Fibonacci.class : Fibonacci.java
	${JAVAC} Fibonacci.java

ArgPrinter.class : ArgPrinter.java
	${JAVAC} ArgPrinter.java

FileWriter.class : FileWriter.java
	${JAVAC} FileWriter.java

TestThread.class : TestThread.java
	${JAVAC} TestThread.java
