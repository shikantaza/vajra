import java.io.File ;
import java.io.FileOutputStream ;
import java.io.PrintStream ;

public class FileWriter
{
	public static void main(String[] args)
	{

		try
		{
			PrintStream out = new PrintStream(new FileOutputStream(new File(args[0]))) ;
			out.println(args[1]) ;
			out.close() ;
			out = null ;
		}
		catch(Exception e)
		{
			System.out.println(e.getMessage()) ;
		}
	}
}
