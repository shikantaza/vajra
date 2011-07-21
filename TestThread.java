public class TestThread implements Runnable
{

	private int id ;

	public TestThread(int i)
	{
		id = i ;
	}
	
	public void run()
	{
		System.out.println("Running thread " + id) ;
	}

	public static void main(String[] args)
	{
		TestThread tt1 = new TestThread(1) ;
		TestThread tt2 = new TestThread(2) ;
	
		Thread t1 = new Thread(tt1) ;
		Thread t2 = new Thread(tt2) ;

		t1.start() ;
		t2.start() ;
	}
}

		

