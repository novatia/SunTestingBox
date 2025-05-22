using CommandLine;

namespace FireballGenerator
{
	internal class Options
	{

		[Option('X', "generate_burst", Required = false, HelpText = "Generate a trail of -n fireball (default 1) ")]
		public bool generate_burst { get; set; } = false;

		[Option('n', "fireball_number", Required = false, HelpText = "A number of generated fireball")]
		public int fireball_number { get; set; } = 1;


		[Option('s', "stationary", Required = false, HelpText = "Try to generate stationary events")]
		public bool generate_stationary_event { get; set; } = false;

		[Option('d', "detection", Required = false, HelpText = "Try to generate stationary events")]
		public bool generate_event { get; set; } = true;


		[Option('b', "start_brightness", Required = false, HelpText = "Start brightness of the events 0->255")]
		public int start_brightness { get; set; } = 64;

		[Option('t', "brigthness_step", Required = false, HelpText = "Brightness increment step for events 0->255")]
		public int brigthness_step { get; set; } = 2;

		[Option('B', "max_brightness", Required = false, HelpText = "Maximum brightness of the events 0->255")]
		public int max_brightness { get; set; } = 88;


		[Option('e', "start_event_duration", Required = false, HelpText = "Start event duration [ms]")]
		public int start_event_duration { get; set; } = 500;


		[Option('E', "stop_event_duration", Required = false, HelpText = "Stop event duration [ms]")]
		public int stop_event_duration { get; set; } = 1250;

		[Option('T', "event_duration_step", Required = false, HelpText = "Event duration increment step[ms]")]
		public int event_duration_step { get; set; } = 250;

		[Option('w', "wait_time", Required = false, HelpText = "Wait time between events")]
		public int wait_time { get; set; } = 1000;

		[Option('I', "ip_address", Required = false, HelpText = "Target ip address")]
		public string ip_address { get; set; } = "192.168.26.77";
	}
}
