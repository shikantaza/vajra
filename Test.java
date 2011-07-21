import java.io.* ;

public class Test
{
	
	public static void main(String[] args)
	{


		String s = "Vajra has been integrated with Classpath!" ;
		
		System.out.println(s) ;
		System.out.println("Vajra has been integrated with Classpath!") ;
                System.out.println("Hello, World!") ;

		
		java.util.Properties p = new java.util.Properties() ;
		
		p.put("java.version", "1.1") ;
		
		String s1 = (String)p.get("java.version") ;
		
		if(s1 == null)
			System.out.println("s1 is null") ;
		else
			System.out.println(s1) ;
		
	}

/*	
	public void someMethod() throws Exception
	{
		try
		{
			int a, b, c ;
			a = 1 ;
			b = 0 ;
			c = a / b ;
		}
		catch(Exception e)
		{
			throw e ;
		}
	}

	private final class InnerCls
	{
		int yy = i ;

	}
*/	
}
