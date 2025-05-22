// See https://aka.ms/new-console-template for more information


using CommandLine;
using FireballGenerator;
using System.Net;

internal class Program
{

	public static string Get(string uri)
	{
		HttpWebRequest request = (HttpWebRequest)WebRequest.Create(uri);
		request.AutomaticDecompression = DecompressionMethods.GZip | DecompressionMethods.Deflate;

		using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
		using (Stream stream = response.GetResponseStream())
		using (StreamReader reader = new StreamReader(stream))
		{
			return reader.ReadToEnd();
		}
	}

	static void RunOptionsAndReturnExitCode(Options opts)
	{
		if (opts.generate_stationary_event)
			Console.WriteLine($"Try to generate stationary events.");

		if (opts.generate_event)
			Console.WriteLine($"Try to generate detections.");



		if (opts.start_brightness < 1 || opts.start_brightness > 255)
		{
			Console.WriteLine($"Start brightness must be between 1 and 255");
			Environment.Exit(1);
		}

		if (opts.max_brightness < 1 || opts.max_brightness > 255 || opts.max_brightness < opts.start_brightness)
		{
			Console.WriteLine($"Max brightness must be between 1 and 255 and must be greater or equals than start brithness.");
			Environment.Exit(2);
		}

		if (opts.brigthness_step < 1 || opts.brigthness_step > 255)
		{
			Console.WriteLine($"Brightness step must be between 1 and 255");
			Environment.Exit(3);
		}


		if (opts.start_event_duration < 1 || opts.start_event_duration > 20000)
		{
			Console.WriteLine($"Start event duration must be between 1 [ms] and 20000 [ms]");
			Environment.Exit(4);
		}

		if (opts.stop_event_duration < 1 || opts.stop_event_duration > 20000 || opts.stop_event_duration < opts.start_event_duration)
		{
			Console.WriteLine($"Stop event duration must be between 1 and 20000 and must be greater or equals than start event duration ");
			Environment.Exit(5);
		}

		if (opts.event_duration_step < 1 || opts.event_duration_step > 20000)
		{
			Console.WriteLine($"Event duration step must be between 1 and 20000");
			Environment.Exit(6);
		}

		if (opts.wait_time < 1 || opts.event_duration_step > 20000)
		{
			Console.WriteLine($"Wait time must be between 1 and 20000");
			Environment.Exit(7);
		}

		if (opts.fireball_number < 1 || opts.fireball_number > 100)
		{
			Console.WriteLine($"Fireball number must be between 1 and 100");
			Environment.Exit(8);
		}


		if (!(opts.generate_stationary_event || opts.generate_event || opts.generate_burst))
		{
			Console.WriteLine($"Nothing to do. Exit.");
			Environment.Exit(0);
		}

		if (opts.generate_burst)
		{
			Console.WriteLine($"Generate fireball burst using start brightness and start event duration");
		}

		if (opts.generate_burst)
		{

			Get("http://" + opts.ip_address + "/B" + opts.start_brightness);
			Thread.Sleep(opts.wait_time);

			//set time
			Get("http://" + opts.ip_address + "/E" + opts.start_event_duration);
			Thread.Sleep(opts.wait_time);

			for (int i=0;i< opts.fireball_number;i++)
			{
				if (opts.generate_event)
				{
					//generate event
					Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "; D event (B" + opts.start_brightness + ", E" + opts.start_event_duration + ")");
					Get("http://" + opts.ip_address + "/D");

					//wait event
					Thread.Sleep(opts.start_event_duration + opts.wait_time);
				}

				if (opts.generate_stationary_event)
				{
					Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "; R event (B" + opts.start_brightness + ", E" + opts.start_event_duration + ")");
					Get("http://" + opts.ip_address + "/R");
					//wait event
					Thread.Sleep(opts.start_event_duration + opts.wait_time);
				}
			}
		}
		else
		{
			for (int brightness = opts.start_brightness; brightness <= opts.max_brightness; brightness += opts.brigthness_step)
			{

				for (int event_duration = opts.start_event_duration; event_duration <= opts.stop_event_duration; event_duration += opts.event_duration_step)
				{
					//set brightness
					Get("http://" + opts.ip_address + "/B" + brightness);
					Thread.Sleep(opts.wait_time);

					//set time
					Get("http://" + opts.ip_address + "/E" + event_duration);
					Thread.Sleep(opts.wait_time);

					if (opts.generate_event)
					{
						//generate event
						Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "; D event (B" + brightness + ", E" + event_duration + ")");
						Get("http://" + opts.ip_address + "/D");

						//wait event
						Thread.Sleep(event_duration + opts.wait_time);
					}

					if (opts.generate_stationary_event)
					{
						Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "; R event (B" + brightness + ", E" + event_duration + ")");
						Get("http://" + opts.ip_address + "/R");
						//wait event
						Thread.Sleep(event_duration + opts.wait_time);
					}
				}
			}
		}
	}


	static void HandleParseError(IEnumerable<Error> errs)
	{

	}

	private static void Main(string[] args)
	{
		Parser.Default.ParseArguments<Options>(args)
			 .WithParsed<Options>(opts => RunOptionsAndReturnExitCode(opts))
			 .WithNotParsed<Options>((errs) => HandleParseError(errs));

	}
}