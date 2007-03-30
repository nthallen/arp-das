<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet ekr_test?>
<leo_file>
<leo_header file_format="2" tnodes="0" max_tnode_index="0" clone_windows="0"/>
<globals body_outline_ratio="0.5">
	<global_window_position top="63" left="10" height="652" width="1014"/>
	<global_log_window_position top="0" left="0" height="0" width="0"/>
</globals>
<preferences/>
<find_panel_settings/>
<vnodes>
<v t="ntallen.20070319130904" a="E"><vh>QNX6 Data Acquisition Architecture</vh>
<v t="ntallen.20070319130904.1"><vh>Data</vh>
<v t="ntallen.20070319130904.2"><vh>TMbfr</vh></v>
<v t="ntallen.20070319130904.3"><vh>Data Generators</vh>
<v t="ntallen.20070319130904.4"><vh>Collection</vh></v>
<v t="ntallen.20070319130904.5"><vh>rdr</vh></v>
<v t="ntallen.20070319130904.6"><vh>Inetin</vh></v>
</v>
<v t="ntallen.20070319130904.7"><vh>Data Clients</vh>
<v t="ntallen.20070319130904.8"><vh>lgr</vh></v>
<v t="ntallen.20070319130904.9"><vh>display</vh></v>
<v t="ntallen.20070319130904.10"><vh>extractions</vh></v>
<v t="ntallen.20070319130904.11"><vh>algorithms</vh></v>
</v>
</v>
<v t="ntallen.20070319130904.12" a="E"><vh>Command</vh>
<v t="ntallen.20070320110353" a="E"><vh>Command Server</vh>
<v t="ntallen.20070320121924"><vh>cmdgen</vh>
<v t="ntallen.20070320135407"><vh>Interface Definition</vh>
<v t="ntallen.20070320135407.1"><vh>Node Name</vh></v>
<v t="ntallen.20070320135407.2"><vh>Message Definition</vh>
<v t="ntallen.20070320135407.3"><vh>Message size</vh></v>
<v t="ntallen.20070320135407.4"><vh>Message contents</vh></v>
<v t="ntallen.20070320135407.5"><vh>Queue Length</vh></v>
</v>
<v t="ntallen.20070320135407.6"><vh>Options</vh></v>
<v t="ntallen.20070320140803"><vh>Operation</vh></v>
<v t="ntallen.20070320140803.1"><vh>Errors</vh>
<v t="ntallen.20070320140803.2"><vh>Queue Full</vh></v>
<v t="ntallen.20070320140803.3"><vh>No Readers</vh></v>
<v t="ntallen.20070320140803.4"><vh>Missed Command</vh></v>
</v>
<v t="ntallen.20070320150524"><vh>Do I need to allow multiple readers?</vh></v>
</v>
</v>
<v t="ntallen.20070320110353.1"><vh>Commands written to /dev/huarp/exp/Cmd</vh></v>
<v t="ntallen.20070320110353.2"><vh>Commands read from /dev/huarp/exp/Cmd/*</vh></v>
<v t="ntallen.20070320111823" a="TV"><vh>Develop IDL for Command Readers</vh></v>
</v>
<v t="ntallen.20070320150524.1"><vh>Command Sources</vh>
<v t="ntallen.20070320150524.2"><vh>Keyboard</vh></v>
<v t="ntallen.20070320150524.3"><vh>Algorithm</vh></v>
</v>
<v t="ntallen.20070320150524.4"><vh>Command Receivers</vh>
<v t="ntallen.20070320150524.5"><vh>DG</vh></v>
<v t="ntallen.20070320150524.6"><vh>lgr</vh></v>
<v t="ntallen.20070320150524.7"><vh>TMbfr</vh></v>
<v t="ntallen.20070320150524.8"><vh>Indexer</vh></v>
<v t="ntallen.20070320150524.9"><vh>SCDC/DCCC</vh></v>
<v t="ntallen.20070322160353"><vh>Quit</vh></v>
</v>
<v t="ntallen.20070320150841"><vh>Development Plan</vh>
<v t="ntallen.20070320150841.1"><vh>Start with TMbfr source</vh></v>
<v t="ntallen.20070320150841.2"><vh>Implement writing</vh></v>
<v t="ntallen.20070320150841.3"><vh>Implement Interface definitions and reading</vh></v>
<v t="ntallen.20070320150841.4"><vh>Incorporate cmdgen</vh></v>
</v>
</v>
<v t="ntallen.20070322130922" a="E"><vh>Logger</vh>
<v t="ntallen.20070322130922.1"><vh>Adopt /dev/huarp/exp/lgr</vh></v>
<v t="ntallen.20070322130922.2"><vh>Accept atomic writes from multiple sources</vh></v>
<v t="ntallen.20070322160353.1"><vh>Message formatting probably takes place in the library</vh></v>
<v t="ntallen.20070322160353.2"><vh>Strategy for termination, communication with the command server</vh>
<v t="ntallen.20070322180349"><vh>Find out how to gracefully shut down a resource manager</vh></v>
<v t="ntallen.20070322160353.3"><vh>Avoiding Deadlock</vh></v>
<v t="ntallen.20070322160353.4"><vh>Terminate after at least one client has opened and all have closed</vh></v>
</v>
<v t="ntallen.20070322130922.3"><vh>Write to log file, etc.</vh></v>
</v>
<v t="ntallen.20070323134835" a="E"><vh>Resmgr Notes</vh>
<v t="ntallen.20070323134835.1" a="E"><vh>resmgr_attach()</vh>
<v t="ntallen.20070323134835.2"><vh>dispatch_t *dpp</vh></v>
<v t="ntallen.20070323134835.3"><vh>resmgr_attr_t resmgr_attr</vh></v>
<v t="ntallen.20070323134835.4"><vh>path</vh></v>
<v t="ntallen.20070323134835.5" a="E"><vh>iofunc_attr_t handle</vh>
<v t="ntallen.20070323134835.6"><vh>iofunc_mount_t mountpoint</vh></v>
</v>
</v>
</v>
</v>
</vnodes>
<tnodes>
<t tx="ntallen.20070319130904">@nocolor
</t>
<t tx="ntallen.20070319130904.1"></t>
<t tx="ntallen.20070319130904.2">Only a single writer (Data Generator) is allowed. Writer can specify O_NONBLOCK to signal realtime (i.e. Collection)
</t>
<t tx="ntallen.20070319130904.3">Data Generators (DGs) are where telemetry data enters the system. These programs open the TMbfr for write. TMbfr is limited to a single writer, so only one DG is allowed per instrument.

DGs also will read from the command server to receive commands.
</t>
<t tx="ntallen.20070319130904.4">Collection is the realtime DG which collects data from the instrument. Each Collection program is instrument-specific, and is compiled from TMC code.</t>
<t tx="ntallen.20070319130904.5">The Reader reads telemetry data from log files written by lgr. rdr is not instrument-specific.
</t>
<t tx="ntallen.20070319130904.6">Inetin retrieves telemetry data from a remote system via TCP/IP.
</t>
<t tx="ntallen.20070319130904.7">Data Clients (DCs) read telemetry data from TMbfr and process it in various ways.</t>
<t tx="ntallen.20070319130904.8">The Logger (lgr) writes telemetry data to disk. It reads commands from the Command Server.
</t>
<t tx="ntallen.20070319130904.9">Display programs read telemetry data from TMbfr and display it on screen as text or graphics.
</t>
<t tx="ntallen.20070319130904.10"></t>
<t tx="ntallen.20070319130904.11"></t>
<t tx="ntallen.20070319130904.12">Centralize command processing</t>
<t tx="ntallen.20070320110353">Command server is experiment-specific, compiled from .cmd input by cmdgen. It accepts ASCII command sequences from clients, parses them and executes the specified code. In this version, that can include making the command available to other client programs.</t>
<t tx="ntallen.20070320110353.1">Using existing ASCII syntax parsed by cmdgen-generated parser
Executes specified C-code
May enqueue commands to external clients</t>
<t tx="ntallen.20070320110353.2">e.g.
/dev/huarp/exp/Cmdr/TM
/dev/huarp/exp/Cmdr/Idx
/dev/huarp/exp/Cmdr/scdc
May have multiple readers (or not?)
Syntax for outbound commands is debatable
</t>
<t tx="ntallen.20070320111823">The point of this is to simplify the enqueuing of commands and the reinterpretation of them on the client side. This could be very simple and is not strictly necessary, since we're not looking at wide open interfaces. The important thing is that it is straightforward to enqueue a command, dequeue a command and interpret the results.

Right now, I would like to see both interfaces be totally ASCII readable.
</t>
<t tx="ntallen.20070320121924">Defines command syntax and code to execute on command reception.
Should also define client interfaces</t>
<t tx="ntallen.20070320135407"></t>
<t tx="ntallen.20070320135407.1">Will appear under /dev/huarp/exp/Cmdr/</t>
<t tx="ntallen.20070320135407.2">Could be combined with options.</t>
<t tx="ntallen.20070320135407.3"></t>
<t tx="ntallen.20070320135407.4"></t>
<t tx="ntallen.20070320135407.5">Actually, since we're a resource manager, we must be able to queue commands, at least one deep.
If there is no reader, then reporting an error is appropriate
If there is a reader, but it is not currently blocked, we could report BUSY
Certain services would benefit from allowing queuing of commands, e.g. the Indexer. Other services probably don't need it.</t>
<t tx="ntallen.20070320135407.6">Queue Length
Command Length/Format
Allow/support multiple readers
</t>
<t tx="ntallen.20070320140803">Each interface contains a queue
If no readers exist, no commands will be enqueued
Commands are dequeued when all current readers have received them or when the queue overflows
The first command written to the queue is numbered 1.
Queue records oldest command number and newest command number.
Each reader has a record of last command read.
</t>
<t tx="ntallen.20070320140803.1"></t>
<t tx="ntallen.20070320140803.2">Logged. Not reported back to the writer. Causes oldest command to be deleted.</t>
<t tx="ntallen.20070320140803.3">Logged. Could be reported back to writer. Command is not enqueued.</t>
<t tx="ntallen.20070320140803.4">To be reported to a reader if the queue overflowed while it was processing.</t>
<t tx="ntallen.20070320150524">Let me try to figure out what command clients I will need. Offhand, it looks like a single reader should suffice in most cases. If we tried to run more than one lgr, or daisy chain TMbfrs or something exotic like that, it would get more complicated.</t>
<t tx="ntallen.20070320150524.1"></t>
<t tx="ntallen.20070320150524.2">Also compiled from cmdgen sources</t>
<t tx="ntallen.20070320150524.3">Does not see cmdgen source, but invokes the command server in test mode to test command syntax
</t>
<t tx="ntallen.20070320150524.4"></t>
<t tx="ntallen.20070320150524.5">Only one of these
</t>
<t tx="ntallen.20070320150524.6">Should be only one of these
</t>
<t tx="ntallen.20070320150524.7">Playback controls: Assuming only monotonic, no rewind. If we support rewind, then the rdr (DG) needs to get those commands too.</t>
<t tx="ntallen.20070320150524.8">One should be enouch
</t>
<t tx="ntallen.20070320150524.9">One
</t>
<t tx="ntallen.20070320150841"></t>
<t tx="ntallen.20070320150841.1"></t>
<t tx="ntallen.20070320150841.2"></t>
<t tx="ntallen.20070320150841.3"></t>
<t tx="ntallen.20070320150841.4"></t>
<t tx="ntallen.20070322130922"></t>
<t tx="ntallen.20070322130922.1"></t>
<t tx="ntallen.20070322130922.2"></t>
<t tx="ntallen.20070322130922.3"></t>
<t tx="ntallen.20070322160353">Everyone wants to get the Quit command. This one almost certainly requires multiple support, or else it is written to every interface.</t>
<t tx="ntallen.20070322160353.1">Examine current library for options
Usual message format includes:
    timestamp: hdr: severity: message

timestamp: Some sort of hook should be available for tweaking this. In playback, we want this to reflect the playback time, as opposed to the realtime, though we might want to mix the two. Perhaps playback time should be prefixed with something: &lt;00:00:00&gt;:

Ideally, lgr will check incoming messages to determine whether they have been properly formatted. If they don't have a timestamp, one should be added. If they don't have a header, maybe it can make one up?

Perhaps Timestamps should include the date. lgr could strip it out, but add a date stamp whenever it changes.

[[mm/dd/yyyy ]hh:mm:ss:][&lt;mm/dd/yyyy hh:mm:ss&gt;][hdr[.SEVERITY]:]message</t>
<t tx="ntallen.20070322160353.2"></t>
<t tx="ntallen.20070322160353.3">lgr cannot read commands from the command server and also accept log messages from the command server without risking a deadlock. The only command we're interested in is 'Quit.'</t>
<t tx="ntallen.20070322160353.4">Suppose I adopt the simple strategy of having lgr begin termination after at least one client has opened a connection and all clients have closed. Startup procedure would be to start lgr, then the command server, and then everything else.</t>
<t tx="ntallen.20070322180349">Monitor messages received. Figure out what the '0' command really is.
No idea, but simply terminating seems to work fine.
</t>
<t tx="ntallen.20070323134835"></t>
<t tx="ntallen.20070323134835.1"></t>
<t tx="ntallen.20070323134835.2"></t>
<t tx="ntallen.20070323134835.3">Probably common to all mountpoints
nparts_max, msg_max_size
other_func (for handling non-resource-manager messages. Probably not necessary for now)</t>
<t tx="ntallen.20070323134835.4">Unique to each mountpoint</t>
<t tx="ntallen.20070323134835.5">unique to each mountpoint
count records the number of OCBs using this attribute. zero should mean all clear
</t>
<t tx="ntallen.20070323134835.6">The mountpoint structure is where the ocb_calloc and ocb_free functions are specified. The structure is optional, so it is only needed if certain options need to be specified or if you want to extend the ocb structure. We may need to do that for TMbfr and/or the Command Server, but perhaps not for the Logger.
</t>
</tnodes>
</leo_file>
