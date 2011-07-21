public class Fibonacci
{
	public static void main(String[] args)
	{
		System.out.println(f(16)) ;
	}

	private static int f(int i)
	{
		if(i == 1 || i == 2)
			return 1 ;
		else
			return f(i-2) + f(i-1) ;
	}
}
