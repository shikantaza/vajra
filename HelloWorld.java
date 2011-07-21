class A
{
	static
	{
		System.out.println("static block of A executed") ;
	}
	public void method()
	{
		System.out.println("method() in A called") ;
	}
}

class B extends A
{
	static
	{
		System.out.println("static block of B executed") ;
	}
	public void method()
	{
		System.out.println("method() in B called") ;
	}
}


public class HelloWorld
{

	public static void main(String arg[])
	{
		
/*
		int i ;
		short s ;
		char c ;
		long l ;
		float f ;
		double d ;
		String str ;

		System.out.println("Enter an integer:") ;
		i = Console.readInt() ;

		System.out.println("Enter a short:") ;
		s = Console.readShort() ;

		System.out.println("Enter a character:") ;
		c = Console.readChar() ;

		System.out.println("Enter a long:") ;
		l = Console.readLong() ;

		System.out.println("Enter a float:") ;
		f = Console.readFloat() ;

		System.out.println("Enter a double:") ;
		d = Console.readDouble() ;

		System.out.println("Enter a string:") ;
		str = Console.readString() ;

		System.out.println(i) ;
		System.out.println(s) ;
		System.out.println(c) ;
		System.out.println(l) ;
		System.out.println(f) ;
		System.out.println(d) ;
		System.out.println(str) ;
		
		double i1 = 10.0 ;

		while(i1 >= 1)
		{
			System.out.println(i1) ;
			i1 = i1 - 0.5 ;
		}
*/		

		HelloWorld hw = new HelloWorld() ;

		A a = new B() ;

		hw.DriverMethod(a) ;

	}

	public void DriverMethod(A a)
	{

		a.method() ;

		int i1 = 4 , i2 = 2 ;
		int i3 ;
		i3 = i1 + i2 ;
		System.out.println(i3) ;
		i3 = i1 - i2 ;
		System.out.println(i3) ;
		i3 = i1 * i2 ;
		System.out.println(i3) ;
		i3 = i1 / i2 ;
		System.out.println(i3) ;

		short s1 = 6 , s2 = 1 ;
		short s3 ;
		s3 = (short)(s1 + s2) ;
		System.out.println(s3) ;
		s3 = (short)(s1 - s2) ;
		System.out.println(s3) ;
		s3 = (short)(s1 * s2) ;
		System.out.println(s3) ;
		s3 = (short)(s1 / s2) ;
		System.out.println(s3) ;

		long l1 = 4 , l2 = 2 ;
		long l3 ;
		l3 = l1 + l2 ;
		System.out.println(l3) ;
		l3 = l1 - l2 ;
		System.out.println(l3) ;
		l3 = l1 * l2 ;
		System.out.println(l3) ;
		l3 = l1 / l2 ;
		System.out.println(l3) ;

		float f1 = 4.88f , f2 = 2.64f ;
		float f3 ;
		f3 = f1 + f2 ;
		System.out.println(f3) ;
		f3 = f1 - f2 ;
		System.out.println(f3) ;
		f3 = f1 * f2 ;
		System.out.println(f3) ;
		f3 = f1 / f2 ;
		System.out.println(f3) ;
		
		double d1 = 7.4938 , d2 = 2.8232421 ;
		double d3 ;
		d3 = d1 + d2 ;
		System.out.println(d3) ;
		d3 = d1 - d2 ;
		System.out.println(d3) ;
		d3 = d1 * d2 ;
		System.out.println(d3) ;
		d3 = d1 / d2 ;
		System.out.println(d3) ;
		


	}
}
