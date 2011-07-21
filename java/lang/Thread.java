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

package java.lang ;

public class Thread
{

    public static int MAX_PRIORITY ;

    public static int MIN_PRIORITY ;

    public static int NORM_PRIORITY ;

    private ClassLoader contextClassLoader ;

    private Thread theThread ;

    private ThreadGroup theThreadGroup = null ;

    private Runnable theTarget = null ;

    //commented out because the name will be taken from the 'thread' class
    //defined in jvm.cpp
    //private String theName = null ;

    private long theStackSize = 0;

    private static native Thread CreateThread() ;

    public Thread()
    {
	theThread = CreateThread() ;
    }

    public Thread(Runnable target)
    {
	theTarget = target ;
	theThread = CreateThread() ;
    }

    public Thread(Runnable target, String name)
    {
	theTarget = target ;
	setName(name) ;
	theThread = CreateThread() ;
    }

    public Thread(String name)
    {
	setName(name) ;
	theThread = CreateThread() ;
    }

    public Thread(ThreadGroup group, Runnable target)
    {
	theThreadGroup = group ;
	theTarget = target ;
	theThread = CreateThread() ;

	theThreadGroup.addThread(theThread) ;
    }

    public Thread(ThreadGroup group, Runnable target, String name)
    {
	theThreadGroup = group ;
	theTarget = target ;
	setName(name) ;
	theThread = CreateThread() ;

	theThreadGroup.addThread(theThread) ;
    }

    public Thread(ThreadGroup group, Runnable target, String name, long stackSize)
    {
	theThreadGroup = group ;
	theTarget = target ;
	setName(name) ;
	theStackSize = stackSize ;
	theThread = CreateThread() ;

	theThreadGroup.addThread(theThread) ;
    }

    public Thread(ThreadGroup group, String name)
    {
	theThreadGroup = group ;
	setName(name) ;
	theThread = CreateThread() ;

	theThreadGroup.addThread(theThread) ;
    }

    public static int activeCount()
    {
	return currentThread().getThreadGroup().activeCount() ;
    }

    public final native void checkAccess() ;

    public native int countStackFrames() ;

    public native static Thread currentThread() ;

    public native void destroy() ;

    public native static void dumpStack() ;

    public static int enumerate(Thread[] tarray)
    {
	return currentThread().getThreadGroup().enumerate(tarray, true) ;
    }

    public ClassLoader getContextClassLoader()
    {
	return contextClassLoader ;
    }

    public final native String getName() ;

    public native int getPriority() ;

    public final ThreadGroup getThreadGroup()
    {
	return theThreadGroup ;
    }

    public native static boolean holdsLock(Object obj) ;

    public native void interrupt() ;

    public static boolean interrupted()
    {
	return currentThread().isInterrupted() ;
    }

    public native boolean isAlive() ;

    public final native boolean isDaemon() ;

    public native boolean isInterrupted() ;

    public final  native void join() ;

    public final native void join(long millis) ;

    public final native void join(long millis, int nanos) ;

    public native void resume() ;

    public void run()
    {
	if(theTarget != null)
	    theTarget.run() ;
    }

    public void setContextClassLoader(ClassLoader cl)
    {
	contextClassLoader = cl ;
    }

    public final native void setDaemon(boolean on) ;

    public final native void setName(String name) ;

    public native void setPriority(int newPriority) ;

    public native static void sleep(long millis) ;

    public native static void sleep(long millis, int nanos) ;

    public native void start() ;

    public void stop()
    {
	stop_internal() ;
	if(theThreadGroup != null)
	    theThreadGroup.removeThread(theThread) ;
    }

    private native void stop_internal() ;

    public void stop(Throwable obj)
    {
	stop_internal(obj) ;
	if(theThreadGroup != null)
	    theThreadGroup.removeThread(theThread) ;
    }

    private native void stop_internal(Throwable obj) ;

    public native void suspend() ;

    public String toString()
    {
	return getName() + ":" + getPriority() + ":" + getThreadGroup().toString() ;
    }

    public native static void yield() ;

}
