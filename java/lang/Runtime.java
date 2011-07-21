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

import java.io.* ;
import java.util.* ;

public class Runtime
{

    public native void addShutdownHook(Thread hook) ;

    public native int availableProcessors() ;

    public Process exec(String command) throws IOException
    {
	//split command into a String array
	StringTokenizer tokenizer = new StringTokenizer(command) ;

	int nof_tokens = tokenizer.countTokens() ;

	String[] cmdarray = new String[nof_tokens] ;

	for(int i=0; i<nof_tokens; i++)
	    {
		cmdarray[i] = tokenizer.nextToken() ;
	    }

	return VMProcess.exec(cmdarray, null, null) ;
    }


    public Process exec(String[] cmdarray) throws IOException
    {
	return VMProcess.exec(cmdarray, null, null) ;
    }

    public Process exec(String[] cmdarray, String[] envp) throws IOException
    {
	return VMProcess.exec(cmdarray, envp, null) ;
    }

    public Process exec(String[] cmdarray, String[] envp, File dir) throws IOException
    {
	return VMProcess.exec(cmdarray, envp, dir) ;
    }

    public Process exec(String cmd, String[] envp) throws IOException
    {
	//split command into a String array
	StringTokenizer tokenizer = new StringTokenizer(cmd) ;

	int nof_tokens = tokenizer.countTokens() ;

	String[] cmdarray = new String[nof_tokens] ;

	for(int i=0; i<nof_tokens; i++)
	    {
		cmdarray[i] = tokenizer.nextToken() ;
	    }

	return VMProcess.exec(cmdarray, envp, null) ;
    }

    public Process exec(String cmd, String[] envp, File dir) throws IOException
    {
	//split command into a String array
	StringTokenizer tokenizer = new StringTokenizer(cmd) ;

	int nof_tokens = tokenizer.countTokens() ;

	String[] cmdarray = new String[nof_tokens] ;

	for(int i=0; i<nof_tokens; i++)
	    {
		cmdarray[i] = tokenizer.nextToken() ;
	    }

	return VMProcess.exec(cmdarray, envp, dir) ;
    }

    public native void exit (int status) ;

    public native long freeMemomry() ;

    public native void gc() ;

    public native InputStream getLocalizedInputStream(InputStream in) ;

    public native OutputStream getLocalizedOutputStream(OutputStream out) ;

    public static native Runtime getRuntime() ;

    public native void halt(int status) ;

    public native void load(String filename) ;

    public native void loadLibrary(String libname) ;

    public native long maxMemory() ;

    public native boolean removeShutdownHook(Thread hook) ;

    public native void runFinalization() ;

    public static native void runFinalizersOnExit(boolean value) ;

    public native long totalMemory() ;

    public native void traceInstructions(boolean on) ;

    public native void traceMethodCalls(boolean on) ;

}