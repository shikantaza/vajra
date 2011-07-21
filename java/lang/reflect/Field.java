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

public class Field
{
    public native boolean equals(Object obj) ;

    public native Object get(Object obj) ;

    public native boolean getBoolean(Object obj) ;

    public native byte getByte(Object obj) ;

    public native char getChar(Object obj) ;

    public native Class getDeclaringClass() ;

    public native double getDouble(Object obj) ;

    public native float getFloat(Object obj) ;

    public native int getInt(Object obj) ;

    public native long getLong(Object obj) ;

    public native int getModifiers() ;

    public native String getName() ;

    public native short getShort(Object obj) ;

    public native Class getType() ;

    public native int hashCode() ;

    public native void set(Object obj, Object value) ;

    public native void setBoolean(Object obj, boolean z) ;

    public native void setByte(Object obj, byte b) ;

    public native void setChar(Object obj, char c) ;

    public native void setDouble(Object obj, double d) ;

    public native void setFloat(Object obj, float f) ;

    public native void setInt(Object obj, int i) ;

    public native void setLong(Object obj, long l) ;

    public native void setShort(Object obj, short s) ;

    public native String toString() ;

}