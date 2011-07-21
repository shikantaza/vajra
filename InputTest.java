import java.io.*;

public class InputTest
{
	public static void main(String[] args)
	{
		try {
	     		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

        		String str = "";

			while(str != "quit")
			{
				System.out.print("Enter text ('quit' to exit): ");
        	    		str = in.readLine();
				if(str != "quit")
  	          			System.out.println("You entered '" + str + "'");
			} ;

            		System.out.println("Bye");
        	}
		catch (IOException e) {
    		}
	}
}
