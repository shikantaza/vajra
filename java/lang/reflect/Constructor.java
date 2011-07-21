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

package java.lang.reflect ;

public class Constructor
{
    public boolean equals(Object obj)
    {
	Constructor other_constructor = (Constructor)obj ;

	if(this.getDeclaringClass() == other_constructor.getDeclaringClass() &&
	   this.getParameterTypes() == other_constructor.getParameterTypes())
	    return true ;
	else
	    return false ;
    }

    public native Class getDeclaringClass() ;

    public native Class[] getExceptionTypes() ;

    public native int getModifiers() ;

    public native String getName() ;

    public native Class[] getParameterTypes() ;

    public native int hashCode() ;

    public native Object newInstance(Object[] initargs) ;

    public native String toString() ;
}