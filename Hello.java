public class Hello
{
	public static void main(String[] args)
	{
		//System.out.println(args[0]) ;
		Integer i = new Integer(10) ;
                
                System.out.println(i) ;
                
		Integer j = new Integer(20) ;

		int k = i.intValue() + j.intValue() ;

		Integer a = new Integer(k) ;

		System.out.println(a.toString()) ;
		System.out.println(4.55) ;
		
	}
}
