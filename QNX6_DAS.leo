<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet ekr_test?>
<leo_file>
<leo_header file_format="2" tnodes="0" max_tnode_index="0" clone_windows="0"/>
<globals body_outline_ratio="0.5">
	<global_window_position top="39" left="10" height="631" width="982"/>
	<global_log_window_position top="0" left="0" height="0" width="0"/>
</globals>
<preferences/>
<find_panel_settings/>
<vnodes>
<v t="ntallen.20070319130904" a="E"><vh>QNX6 Data Acquisition Architecture</vh>
<v t="ntallen.20070409132623"><vh>PR</vh>
<v t="ntallen.20070409133739" a="E"><vh>Operation</vh>
<v t="ntallen.20070409133739.1"><vh>Configurations</vh>
<v t="ntallen.20070409132623.1"><vh>Full Up with GSE</vh></v>
<v t="ntallen.20070409132623.2"><vh>Flight Mode</vh></v>
<v t="ntallen.20070409132623.3"><vh>Playback</vh></v>
<v t="ntallen.20070409132623.4"><vh>Extraction</vh></v>
<v t="ntallen.20070409132623.5"><vh>Remote GSE</vh></v>
</v>
<v t="ntallen.20070409133739.2" a="E"><vh>Display Options</vh>
<v t="ntallen.20070409132623.6"><vh>Data</vh></v>
<v t="ntallen.20070409132623.7"><vh>Command</vh></v>
</v>
<v t="ntallen.20070409132623.8" a="E"><vh>Applications</vh>
<v t="ntallen.20070409132623.9"><vh>Memo</vh></v>
</v>
</v>
<v t="ntallen.20070409133739.3"><vh>Implementation</vh></v>
</v>
<v t="ntallen.20070413134915" a="E"><vh>QNX Development Issues</vh>
<v t="ntallen.20070425144317"><vh>Handling more than one executable per project</vh></v>
<v t="ntallen.20070413134915.1" a="E"><vh>Development Strategy</vh>
<v t="ntallen.20070425144317.1"><vh>cmdgen</vh>
<v t="ntallen.20070430134037"><vh>QNX4</vh></v>
</v>
<v t="ntallen.20070502121159"><vh>Names and Devices</vh></v>
<v t="ntallen.20070425144317.2"><vh>tmlib</vh></v>
<v t="ntallen.20070425144317.3"><vh>tmphlib</vh></v>
<v t="ntallen.20070425144317.4"><vh>nortlib</vh></v>
</v>
</v>
<v t="ntallen.20070502121159.1" a="E"><vh>Implementation</vh>
<v t="ntallen.20070320121924"><vh>cmdgen</vh>
<v t="ntallen.20070501165557"><vh>Done</vh>
<v t="ntallen.20070411152334"><vh>Add Interface Definitions</vh>
<v t="ntallen.20070404111542"><vh>Initialization</vh></v>
</v>
<v t="ntallen.20070501165557.1"><vh>Figure out appropriate library flag</vh></v>
<v t="ntallen.20070502121159.2"><vh>Syntax and version checking</vh>
<v t="ntallen.20070502121159.3"><vh>cis must produce syntax error messages</vh></v>
<v t="ntallen.20070502121159.4"><vh>cic must handle version test differently</vh></v>
</v>
<v t="ntallen.20070404165230.1"><vh>Handle Quit</vh></v>
<v t="ntallen.20070501165557.2"><vh>Fix up the experiment path correctly in cis.c</vh></v>
<v t="ntallen.20070508171951"><vh>cmdgen: cis_interface nees to be bracketed by #ifdef SERVER</vh></v>
<v t="ntallen.20070508171951.1"><vh>cmdgen: cmdgen.skel needs main() changes for PHOTON</vh></v>
<v t="ntallen.20070501165557.3"><vh>Tweak cmdgen.skel</vh></v>
<v t="ntallen.20070501165557.4"><vh>Write a photon command client</vh></v>
<v t="ntallen.20070508171951.3"><vh>cmdgen: cmdgen.skel needs to be distributed</vh></v>
<v t="ntallen.20070509151554"><vh>Move oui libary to $(datadir)/oui</vh></v>
<v t="ntallen.20070508171951.4"><vh>cmdgen: cmdgen.skel atexit() needs to be investigated</vh></v>
<v t="ntallen.20070509151554.1"><vh>cmdgen: cmdgen.skel SERVER DISPLAY_EOL</vh></v>
<v t="ntallen.20070508171951.5"><vh>Handle quit and exit in cmdclient</vh></v>
<v t="ntallen.20070509151554.2"><vh>Move cmdclient.c into tmphlib</vh></v>
</v>
<v t="ntallen.20070508171951.2" a="E"><vh>Todo</vh>
<v t="ntallen.20070508171951.6"><vh>Draw polygon</vh></v>
<v t="ntallen.20070508171951.7"><vh>Get a cursor in cmd_text somehow</vh></v>
<v t="ntallen.20070509151554.3"><vh>Add default .cmd files as appropriate</vh></v>
</v>
<v t="ntallen.20070502121159.5"><vh>Memo interface can wait</vh></v>
</v>
<v t="ntallen.20070509151554.4" a="E"><vh>appgen</vh>
<v t="ntallen.20070509151554.5"><vh>command server</vh></v>
</v>
</v>
<v t="ntallen.20070502121159.6" a="E"><vh>Design</vh>
<v t="ntallen.20070319130904.1" a="E"><vh>Data</vh>
<v t="ntallen.20070524163205" a="E"><vh>Data Issues</vh>
<v t="ntallen.20070524163205.1"><vh>What is the difference between the telemetry stream and the TM stream as returned from TMbfr?</vh>
<v t="ntallen.20070524163205.2"><vh>TM has no timestamps</vh></v>
<v t="ntallen.20070524163205.3"><vh>Does not include TM info struct or identifiers</vh></v>
</v>
<v t="ntallen.20070524163205.4"><vh>What form should Inetin/Inetout use?</vh></v>
<v t="ntallen.20070524163205.5"><vh>What form should lgr/rdr use?</vh></v>
<v t="ntallen.20070524163205.6"><vh>What about other DGs? Collection?</vh></v>
<v t="ntallen.20070524163205.7"><vh>What sort of data transformations should TMbfr perform?</vh></v>
<v t="ntallen.20070524163205.8" a="E"><vh>TMbfr Operation</vh>
<v t="ntallen.20070525114357"><vh>Register Names</vh></v>
<v t="ntallen.20070525114357.1"><vh>Delay allocating buffers</vh></v>
<v t="ntallen.20070525114357.2"><vh>DG initialization</vh></v>
<v t="ntallen.20070525114357.3"><vh>Store incoming data in circular queue</vh></v>
<v t="ntallen.20070525114357.4"><vh>Store metadata in linked list</vh></v>
<v t="ntallen.20070525114357.5"><vh>Store TimeStamps in linked list</vh></v>
<v t="ntallen.20070529132018"><vh>Extend Attribute Structure</vh></v>
<v t="ntallen.20070529132018.1"><vh>Provide iofunc_open_handler for DG</vh></v>
<v t="ntallen.20070525114357.6" a="E"><vh>Store context in each OCB</vh>
<v t="ntallen.20070525132754"><vh>Partial frames</vh></v>
</v>
<v t="ntallen.20070529132018.2" a="E"><vh>io_write</vh>
<v t="ntallen.20070531163245"><vh>States</vh>
<v t="ntallen.20070529133623"><vh>Partial Header</vh></v>
<v t="ntallen.20070531162708"><vh>TM_Info</vh></v>
<v t="ntallen.20070529133623.1"><vh>Partial Data</vh></v>
</v>
<v t="ntallen.20070611133034" a="E"><vh>Data Handling</vh>
<v t="ntallen.20070611133034.1"><vh>T1-&gt;T1</vh></v>
<v t="ntallen.20070611133034.2"><vh>T1-&gt;T2</vh></v>
<v t="ntallen.20070611133034.3"><vh>T1-&gt;T3</vh></v>
<v t="ntallen.20070611133034.4"><vh>T2-&gt;T1 x</vh></v>
<v t="ntallen.20070611133034.5"><vh>T2-&gt;T2</vh></v>
<v t="ntallen.20070611133034.6"><vh>T2-&gt;T3 x</vh></v>
<v t="ntallen.20070611133034.7"><vh>T3-&gt;T1 x</vh></v>
<v t="ntallen.20070611133034.8"><vh>T3-&gt;T2 x</vh></v>
<v t="ntallen.20070611133034.9" a="TV"><vh>T3-&gt;T3</vh></v>
</v>
</v>
<v t="ntallen.20070531154309" a="E"><vh>io_read</vh>
<v t="ntallen.20070531163245.1" a="E"><vh>States</vh>
<v t="ntallen.20070531163245.2"><vh>Partial Header</vh></v>
<v t="ntallen.20070531163245.3"><vh>TM_Info</vh></v>
<v t="ntallen.20070531163245.4"><vh>Partial Data</vh></v>
</v>
</v>
<v t="ntallen.20070601102213"><vh>Commandability</vh></v>
</v>
<v t="ntallen.20070524163205.9" a="E"><vh>Client Options</vh>
<v t="ntallen.20070524163205.10"><vh>Regulated Output</vh></v>
<v t="ntallen.20070524163205.11"><vh>Incomplete Data</vh></v>
</v>
</v>
<v t="ntallen.20070319130904.2"><vh>TMbfr</vh></v>
<v t="ntallen.20070319130904.3"><vh>Data Generators</vh>
<v t="ntallen.20070319130904.4"><vh>Collection</vh>
<v t="ntallen.20070404221945"><vh>Define data interfaces</vh></v>
</v>
<v t="ntallen.20070319130904.5"><vh>rdr</vh></v>
<v t="ntallen.20070319130904.6"><vh>Inetin</vh></v>
</v>
<v t="ntallen.20070319130904.7" a="E"><vh>Data Clients</vh>
<v t="ntallen.20070319130904.8"><vh>lgr</vh></v>
<v t="ntallen.20070319130904.9"><vh>display</vh></v>
<v t="ntallen.20070319130904.10"><vh>extractions</vh></v>
<v t="ntallen.20070319130904.11"><vh>algorithms</vh></v>
</v>
<v t="ntallen.20070404221945.1"><vh>TMC rework</vh></v>
<v t="ntallen.20070404221945.2"><vh>SNAFU/ssp library</vh></v>
</v>
<v t="ntallen.20070319130904.12"><vh>Command</vh>
<v t="ntallen.20070320110353" a="E"><vh>Command Server</vh>
<v t="ntallen.20070320135407"><vh>Reader Interface Definition</vh>
<v t="ntallen.20070320135407.1"><vh>Node Name</vh></v>
<v t="ntallen.20070320135407.2"><vh>Client Message Definition</vh>
<v t="ntallen.20070320135407.4"><vh>Message contents</vh></v>
<v t="ntallen.20070320135407.5"><vh>Queue Length</vh></v>
</v>
<v t="ntallen.20070320140803"><vh>Operation</vh>
<v t="ntallen.20070330134656.1" a="E"><vh>Command Type</vh></v>
</v>
<v t="ntallen.20070320140803.1"><vh>Errors</vh>
<v t="ntallen.20070320140803.2"><vh>Out of Memory</vh></v>
<v t="ntallen.20070320140803.3"><vh>No Readers</vh></v>
<v t="ntallen.20070320140803.4"><vh>Missed Command</vh></v>
</v>
</v>
<v t="ntallen.20070404111542.1" a="E"><vh>Write Interface Definition</vh>
<v t="ntallen.20070404111542.2"><vh>How will syntax testing work?</vh></v>
</v>
<v t="ntallen.20070320150841"><vh>Development Plan</vh>
<v t="ntallen.20070403140218"><vh>resmgr_attach()</vh>
<v t="ntallen.20070403140218.1"><vh>dispatch_t *dpp</vh></v>
<v t="ntallen.20070403140218.2"><vh>resmgr_attr_t resmgr_attr</vh></v>
<v t="ntallen.20070403140218.3"><vh>path</vh></v>
<v t="ntallen.20070403140218.4"><vh>IOFUNC_ATTR_T handle</vh>
<v t="ntallen.20070403140218.5"><vh>iofunc_mount_t mountpoint</vh></v>
</v>
</v>
<v t="ntallen.20070330134656"><vh>Define data structures for queuing command output</vh>
<v t="ntallen.20070330134656.1"><vh>Command Type</vh></v>
</v>
<v t="ntallen.20070404165230"><vh>Support SELECT</vh></v>
<v t="ntallen.20070404165230.1"><vh>Handle Quit</vh></v>
<v t="ntallen.20070404165230.2"><vh>Support Signals</vh></v>
<v t="ntallen.20070320150841.4"><vh>Incorporate cmdgen</vh></v>
</v>
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
</v>
<v t="ntallen.20070322130922"><vh>Memo</vh>
<v t="ntallen.20070322160353.1"><vh>Message formatting probably takes place in the library</vh></v>
<v t="ntallen.20070322130922.1"><vh>Adopt /dev/huarp/exp/lgr</vh></v>
<v t="ntallen.20070322130922.3"><vh>Write to log file, etc.</vh></v>
<v t="ntallen.20070330134656.3"><vh>Can eliminate ocb_funcs here</vh></v>
<v t="ntallen.20070322130922.2"><vh>Accept atomic writes from multiple sources</vh></v>
<v t="ntallen.20070322160353.2"><vh>Strategy for termination, communication with the command server</vh>
<v t="ntallen.20070322180349"><vh>Find out how to gracefully shut down a resource manager</vh></v>
<v t="ntallen.20070322160353.3"><vh>Avoiding Deadlock</vh></v>
<v t="ntallen.20070322160353.4"><vh>Terminate after at least one client has opened and all have closed</vh></v>
</v>
</v>
</v>
<v t="ntallen.20070404221945.3"><vh>Graphics</vh></v>
<v t="ntallen.20070404221945.4"><vh>Drivers</vh>
<v t="ntallen.20070404221945.5"><vh>Subbus</vh></v>
<v t="ntallen.20070404221945.6"><vh>SCDC/DCCC</vh></v>
<v t="ntallen.20070404221945.7"><vh>Indexer</vh></v>
<v t="ntallen.20070404221945.8"><vh>Thompson?</vh></v>
<v t="ntallen.20070404221945.9"><vh>dacache</vh></v>
</v>
<v t="ntallen.20070323134835"><vh>Resmgr Notes</vh>
<v t="ntallen.20070323134835.1" a="E"><vh>resmgr_attach()</vh>
<v t="ntallen.20070323134835.2"><vh>dispatch_t *dpp</vh></v>
<v t="ntallen.20070323134835.3"><vh>resmgr_attr_t resmgr_attr</vh></v>
<v t="ntallen.20070323134835.4"><vh>path</vh></v>
<v t="ntallen.20070323134835.5" a="E"><vh>iofunc_attr_t handle</vh>
<v t="ntallen.20070323134835.6"><vh>iofunc_mount_t mountpoint</vh></v>
</v>
</v>
<v t="ntallen.20070330134656.4"><vh>OCB</vh></v>
<v t="ntallen.20070330134656.5"><vh>Strategy for multiple readers</vh></v>
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
<t tx="ntallen.20070319130904.12">The command architecture consists of Command Sources, the central Command Server and Command Receivers.
Command Sources include algorithms and the keyboard client
Command Receivers are client programs that wish to receive commands.

The keyboard client and the command server are both compiled from the same source code generated by cmdgen that knows how to parse text commands and perform actions based on those commands. These are in turn linked with library functions (in tmlib or tmphlib?) to make appropriate executables</t>
<t tx="ntallen.20070320110353">Command server is experiment-specific, compiled from .cmd input by cmdgen. It accepts ASCII command sequences from clients, parses them and executes the specified code. In this version, that can include making the command available to other client programs.</t>
<t tx="ntallen.20070320121924">Defines command syntax and code to execute on command reception.
</t>
<t tx="ntallen.20070320135407"></t>
<t tx="ntallen.20070320135407.1">Will appear under /dev/huarp/exp/cmd/</t>
<t tx="ntallen.20070320135407.2">Could be combined with options.</t>
<t tx="ntallen.20070320135407.4">ASCII text, client-specific. Each message should end with a newline so it will play well with shell tools.
There will be a fixed maximum message length for all clients.
</t>
<t tx="ntallen.20070320135407.5">Will simply provide linked list with no real limit. However commands queued before a reader connects will be discarded.</t>
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
Probably want to extend to keep track of open handles for this resource.</t>
<t tx="ntallen.20070323134835.6">The mountpoint structure is where the ocb_calloc and ocb_free functions are specified. The structure is optional, so it is only needed if certain options need to be specified or if you want to extend the ocb structure. We may need to do that for TMbfr and/or the Command Server, but perhaps not for the Logger.
</t>
<t tx="ntallen.20070330134656"></t>
<t tx="ntallen.20070330134656.1">Each command receiver is a command type.
For each command type, there is a mountpoint and a command output queue.

    Don't maintain list of OCBs with each command. Just maintain a reference count
    OCBs still point to command. When command is serviced (written to reading client)
      the current command reference count is decremented
        If it is the first command and the reference count is zero, recycle
      the OCBs command link is pointed to the next command
      the next command's reference count is incremented
    The only time a client will block is when the command queue is empty, so then we
    can maintain a list of waiting OCBs, but we don't have to keep it with each command.
    When a new command is received, send replies to all waiting clients and remove them
    from the waiting list.

    In this case, OCBs need
      next_command pointer
      rcvid (to indicate blocking)
      next_ocb pointer (for blocked OCBs list)
    Command needs
      reference count
      next_command pointer
    Each mountpoint (RESMGR_HANDLE_T)
      list of blocked OCBs

    In the ocb_alloc func, point to oldest command and increment reference count
    In ocb_free, decrement reference count (and possibly release the command)</t>
<t tx="ntallen.20070330134656.3"></t>
<t tx="ntallen.20070330134656.4">Unique to each open file handle. Points to the iofunc_attr_t (so you can tell what resource you're accessing.)
May need to extend if necessary to record state for this handle.
</t>
<t tx="ntallen.20070330134656.5">In single-threaded approach:
    On write, we should queue the write, then process any pending reads on that resource
        queue the data
        reply successfully to the writer
        process pending reads
        return(_RESMGR_NOREPLY);
    On read, if data is present, reply immediately, otherwise queue:
        save ctp-&gt;rcvid
        return(_RESMGR_NOREPLY);
In multi-threaded approach:
    On read, if data is not present, block on thread-specific semaphore.
    On write, queue the write and then wake up blocked reader threads.
    This assumes readers haven't locked any structures</t>
<t tx="ntallen.20070403140218"></t>
<t tx="ntallen.20070403140218.1"></t>
<t tx="ntallen.20070403140218.2">Probably common to all mountpoints. Initialized in main()
nparts_max, msg_max_size
other_func (for handling non-resource-manager messages. Probably not necessary for now)</t>
<t tx="ntallen.20070403140218.3">Unique to each mountpoint</t>
<t tx="ntallen.20070403140218.4">unique to each mountpoint, needs to be extended to include list of blocked OCBs
#define IOFUNC_ATTR_T struct ioattr_s

Extended to include:
    blocked; // list of blocked OCBs
    commands; // list of pending commands
    next; // link to another handle

</t>
<t tx="ntallen.20070403140218.5">Defines how ocb structure is extended. Can be common to all mountpoints

The mountpoint structure is where the ocb_calloc and ocb_free functions are specified. The structure is optional, so it is only needed if certain options need to be specified or if you want to extend the ocb structure. We may need to do that for TMbfr and/or the Command Server, but perhaps not for the Logger.
</t>
<t tx="ntallen.20070404111542">Reader initializations should be combined in a function called at startup.
Calling of initialization functions is generally handled via oui. In this case, where we expect almost every command server to initialize reader interfaces, we might modify the command server oui to call a known function.

Reader initialization will look something like:
    
    IOFUNC_ATTR_T *if_indxr;
    IOFUNC_ATTR_T *if_scdc;
    
    void cis_interfaces( void ) {
      if_indxr = cmdsrvr_setup_rdr( "indxr" );
      if_scdc = cmdsrvr_setup_rdr( "scdc" );
    }
    
    { cmdsrvr_turf( if_indxr, "indexer command text" ); }

The fact that this is so formulaic suggests we could make the specification easier. Something like:
    
    %INTERFACE &lt;indxr&gt;
    %INTERFACE &lt;scdc&gt;
    
    { cmdsrvr_turf( if_indxr, "indexer command text" ); }

Allowing INTERFACE definitions on separate lines means they can be further separated into multiple files.</t>
<t tx="ntallen.20070404111542.1">/dev/huarp/Exp/cmd/server
Maximum command length arbitrary (256)
Command messages are all ASCII, terminated by newline
One command per write()
Use an optional prefix:
    [ '[' mnemonic [ ':' [T][Q] ] ']' ] command
    '[' [ mnemonic ] ':V' version ] '\n'
mnemonic is the sender's mnemonic to be passed on to the log when command is received
</t>
<t tx="ntallen.20070404111542.2">For tmcalgo, we can play all sorts of games, but for file slurping, we need realtime feedback.
Can I return an error code and log the actual text?

Let's try for returning an error code. Server should produce the verbose error message to the log, but the client really only needs to know whether or not the command is correct.
</t>
<t tx="ntallen.20070404165230"></t>
<t tx="ntallen.20070404165230.1">When Quit is received, run queues on all clients returning zero bytes and return zero bytes to anyone about to block. Whenever an OCB is closed, check to see how many remain open. When the number drops to zero, we can quit.</t>
<t tx="ntallen.20070404165230.2"></t>
<t tx="ntallen.20070404221945"></t>
<t tx="ntallen.20070404221945.1"></t>
<t tx="ntallen.20070404221945.2"></t>
<t tx="ntallen.20070404221945.3"></t>
<t tx="ntallen.20070404221945.4"></t>
<t tx="ntallen.20070404221945.5"></t>
<t tx="ntallen.20070404221945.6"></t>
<t tx="ntallen.20070404221945.7"></t>
<t tx="ntallen.20070404221945.8"></t>
<t tx="ntallen.20070404221945.9"></t>
<t tx="ntallen.20070409132623">Flash app to display different modes of operation</t>
<t tx="ntallen.20070409132623.1"></t>
<t tx="ntallen.20070409132623.2"></t>
<t tx="ntallen.20070409132623.3"></t>
<t tx="ntallen.20070409132623.4"></t>
<t tx="ntallen.20070409132623.5"></t>
<t tx="ntallen.20070409132623.6">Data should highlight the data flow</t>
<t tx="ntallen.20070409132623.7">Command highlights the command flow</t>
<t tx="ntallen.20070409132623.8">Selecting an application displays a synopsis of its function and pointers to further information</t>
<t tx="ntallen.20070409132623.9">Memo is a special case.
Within any of the modes, selecting Memo should highlight the message links from active apps to Memo
</t>
<t tx="ntallen.20070409133739"></t>
<t tx="ntallen.20070409133739.1"></t>
<t tx="ntallen.20070409133739.2"></t>
<t tx="ntallen.20070409133739.3">Apps
DataLinks
CmdLinks
MemoLinks

For Each App and Link, need to know which configurations apply
For Apps, simply need to know which configurations
For Links, they are present if both their source and destination apps are present
Could be calculated or pre-calculated
Simplest implementation I can think of encodes object name and bit-mapped object type and configuration modes

Simpler approach is to use a different frame-range for each configuration and use layers for:
    Applications
    DataLinks
    CommandLinks
    MemoLinks

This requires no encoding</t>
<t tx="ntallen.20070411152334"></t>
<t tx="ntallen.20070413134915"></t>
<t tx="ntallen.20070413134915.1">How shall I organize the source code? I'd like to have some sort of hierarchy, but that's bucking the IDE's approach.
Let me try to lay out a hierarchy, and see if it can't work.

Each node below here will be an IDE project. If there are sub-nodes, they will be extra executables

Avoid Recursive Makes wherever possible
Use automake/autoconf
  subdir-objects option, etc.</t>
<t tx="ntallen.20070425144317">The frustrating thing is that the documentation on the Makefile conventions used clearly documents more layers than are implemented by the IDE. For any but the simplest projects, understanding the Makefile conventions is very useful, so having the two systems out of agreement makes life more complicated.

That said, I think I have figured out how to create a QNX C Project with a SECTION layer.

1: Create the Project in the IDE
2: Open a shell and delete the target processor subdirectory(ies)
3: Edit Makefile and change LIST=CPU to LIST=OS
4: run: addvariant section1 x86 o
   [ replace section1 with the desired section name(s) ]
   [ replace x86 with the appropriate processor(s) ]
   [ replace o with the appropriate variant(s) ]
5: Edit Makefile again and change LIST=OS to LIST=SECTION
   [ this step may not be necessary, but I had some problem under
     neutrino when I didn't do this... ]
6: Edit common.mk and change:

   include $(MKFILES_ROOT)/qmacros.mk

  to

   EXTRA_SILENT_VARIANTS=$(SECTION)
   include $(MKFILES_ROOT)/qmacros.mk
   NAME=$(SECTION)

7: Refresh the project in the IDE

NOTES: addvariant does nothing unless it finds LIST=OS, so if you want to add variants later, you have to make that change in the root Makefile. addvariant clearly doesn't understand sections, but it is still useful for adding the right Makefiles at each level. I'm taking advantage of the fact that addvariant *does* support the OS layer, which I don't need. I'm pretending that the section layer is an OS layer. For deeper nesting, manual intervention would be the thing to do.
</t>
<t tx="ntallen.20070425144317.1"></t>
<t tx="ntallen.20070425144317.2">Functions that pertain directly to the data acquisition system. Probably depends on nortlib.</t>
<t tx="ntallen.20070425144317.3">Data acquisition functions that depend directly on Photon. If I adopt a server-based approach, I should be able to limit the number of applications that depend directly on this library.</t>
<t tx="ntallen.20070425144317.4">General utility functions</t>
<t tx="ntallen.20070430134037">Added autoconf/automake support, but turns out it isn't backwards compatible for a couple reasons:
    1. -lnort needs to be -lnortlib on QNX4. Could check for either library and take whatever is found.
    2. Lib check for compiler_init_options() returns undefined symbols on QNX4. I've added yyin.c and optstr.c to nortlib2, and perhaps they need to be back-ported to QNX4.
Fixed those two, but now running into a sed incompatability.

Trying to port recent GNU sed
need to edit bootstrap.sh
  need to add HAVE_LIMITS_H
  need to define HAVE_SYS_FILE_H
  need to add '1' to blank defines
  need to add -lunix -N1M to link step
  need to #define inline

When running configure, probably need to pass in -lunix -N1M
./configure LDFLAGS="-lunix -N1M" LD=wlink
Getting lots of "Memory exhausted" complaints from sed
</t>
<t tx="ntallen.20070501165557"></t>
<t tx="ntallen.20070501165557.1">Some flag passed to the linker to embed the library path into the executable
ld flag -rpath /usr/local/lib
LDFLAGS=-Wl,-rpath /usr/local/lib -L/usr/local/lib -ltm -lnort
CFLAGS=-I/usr/local/include
</t>
<t tx="ntallen.20070501165557.2">/dev/huarp/&lt;Exp&gt;/cmd/&lt;server&gt;</t>
<t tx="ntallen.20070501165557.3">My guess is that when CLIENT is not defined, we don't need any interface functions.
Right now, server is getting some output because DISPLAY_EOL is being invoked.</t>
<t tx="ntallen.20070501165557.4">Modify this to skip Phab. It's pretty basic:
    PtInit(NULL)
    PtWidget_t *w = tbl_window("name", w,h); // may want to select some other attributes
    // attach keyboard event to window
    PtAddFilterCallback(w, ph_EV_KEY, my_key_event, NULL);
    PtWidget_t *cmd = tbl_field( w, "cmd", x, y, w, h );
    PtWidget_t *prompt = tbl_field( w, "prompt", x, y, w, h );
    // figure out how to draw my funky prompt symbol
    // PgDrawPolygon()
    PtRealizeWidget(w);
    // phinitfunc();  // This is what did all the drawing in CRphdisp.c
    PtMainLoop();

Link with -ltmph -ltm? -lph</t>
<t tx="ntallen.20070502121159">QNX4 nortlib had nl_make_name() and nl_find_name() for building names that were then searched for.
A version of nl_make_name makes good sense. nl_find_name() actually opened a connection. Since the connection is now an fd, the corresponding function would have to take open flags (O_RDONLY, etc.)</t>
<t tx="ntallen.20070502121159.1"></t>
<t tx="ntallen.20070502121159.2"></t>
<t tx="ntallen.20070502121159.3"></t>
<t tx="ntallen.20070502121159.4"></t>
<t tx="ntallen.20070502121159.5">Just because it should be easy to add via oui. Use nl_error() now, and that will map to memo seamlessly.</t>
<t tx="ntallen.20070502121159.6"></t>
<t tx="ntallen.20070508171951"></t>
<t tx="ntallen.20070508171951.1"></t>
<t tx="ntallen.20070508171951.2"></t>
<t tx="ntallen.20070508171951.3"></t>
<t tx="ntallen.20070508171951.4"></t>
<t tx="ntallen.20070508171951.5"></t>
<t tx="ntallen.20070508171951.6"></t>
<t tx="ntallen.20070508171951.7"></t>
<t tx="ntallen.20070509151554">given package name 'foo', oui currently looks for
  foo.oui
  $path/foo.oui
  oui/foo.oui
  $path/oui/foo.oui

where $path is a searchenv() call on INCLUDE
I'm inclined to go with:
    foo.oui
    oui/foo.oui
    $(datadir)/foo.oui
    $(datadir)/oui/foo.oui</t>
<t tx="ntallen.20070509151554.1">Do not require an interface definition if SERVER alone is defined</t>
<t tx="ntallen.20070509151554.2"></t>
<t tx="ntallen.20070509151554.3">root.cmd at least, but it needs editing</t>
<t tx="ntallen.20070509151554.4">Let's avoid including appgen.mk. Include it directly into the Makefile instead.
autotools gives explicit link instructions:
    cmdgen$(EXEEXT): $(cmdgen_OBJECTS) $(cmdgen_DEPENDENCIES)
        @rm -f cmdgen$(EXEEXT)
        $(LINK) $(cmdgen_OBJECTS) $(cmdgen_LDADD) $(LIBS)
Hence it wouldn't be unreasonable to add the appropriate usemsg and/or promote instructions
</t>
<t tx="ntallen.20070509151554.5">LDFLAGS needs -Wl,-rpath -Wl,/usr/local/lib
Need to promote, add usage</t>
<t tx="ntallen.20070524163205">What is the difference between the telemetry stream and the TM stream as returned from TMbfr?
</t>
<t tx="ntallen.20070524163205.1"></t>
<t tx="ntallen.20070524163205.2">Timestamps allow date/time to high precision without recording YYMMDDHHMMSS in every frame.</t>
<t tx="ntallen.20070524163205.3">App is assumed to match local definition. Can only check that MFCtr/Synch make sense releative to TM info.</t>
<t tx="ntallen.20070524163205.4">Might as well use a format similar to TMbfr's output</t>
<t tx="ntallen.20070524163205.5">Same here, although we might choose to add some sort of checksuming</t>
<t tx="ntallen.20070524163205.6">If Inetin and rdr use the TMbfr format, all DGs should</t>
<t tx="ntallen.20070524163205.7">TMbfr presumably will understand at least the four basic data formats, TMTYPE_DATA_T[1-4]
Should it:
    Decide based on the format which output format to use, or
    Respond to client requests in order to provide alternate formats

At the moment, I don't know that there is any motivation to provide a less compact format, except for the purpose of breaking up a long record into shorter records. A telemetry output program might need to add MFCtr and Synch to a T3 record, but that is going to be pretty rare. TMbfr should be able to promote a stream from T1 to T3 or T4 if it is capable. If it receives a T1 or T2 stream, it should perform sanity checks to make sure MFCtr and Synch agree with the spec (although these should be redundant). In T4 streams, checksum should be checked (or might be optionally checked.)</t>
<t tx="ntallen.20070524163205.8"></t>
<t tx="ntallen.20070524163205.9">There are three basic modes of operation worth considering:
    fast realtime response
    optimal batch performance
    playback regulated output
Fast realtime repsonse is desired by display programs and algorithms that need to respond to current conditions in a timely manner
Optimal batch performance delivers larger chunks of data to programs that don't need realtime response such as lgr and ext
Regulated output is used for playback or remote display to emulate realtime operation, but also support DVR controls

lgr often runs at the same time as algo and/or display programs, so it would be nice to be able to distinguish between the fast and optimal modes on a client basis.

lgr and ext rarely run in playback mode.

    /dev/huarp/Exp/TM/DG writer interface
    /dev/huarp/Exp/TM/DCf fast reader interface
    /dev/huarp/Exp/TM/DCo optimal reader interface

Provide a playback option to TMbfr, or simply recognize DVR controls when DG does not have O_NONBLOCK set?

Still need to know whether or not to regulate. rdr is used for both playback and extraction, and for extraction we usually don't want to regulate the output. There's still enough info if lgr and ext read from the TM/DCo interface and there is no one reading from DCf, then no regulation is required and we can charge full speed ahead. If there is someone reading from DCf, then we do need to regulate, and if we really want the display program or algo reading from DCf to run at full speed, then we need to access the DVR controls and ask for fast forward. If there was some reason this happened regularly, it would be easy to provide a command-line option to have a DC use the DCo interface instaed of the DCf interface.

In fast forward, DCf and DCo are treated the same.

Unless we are running in realtime, don't drop any data packets.</t>
<t tx="ntallen.20070524163205.10">-oR
All clients selecting -oR are synchronized. Pause, fast forward, etc. are all handled in concert by TMbfr.
-oR doesn't make a lot of sense in realtime (O_NONBLOCK) since the data should be arriving at the regulated rate anyway. In this case, the usual playback controls should not be in effect (pause, ff, slower, etc.) although it is possible to imagine a scenario where rewind over a short distance would be possible for instant replay (ala DVR)

If we are in playback mode, we could be lenient if individual clients can't keep up, or we might choose to drop data to maintain the rate.

For fast forward operation</t>
<t tx="ntallen.20070524163205.11">This is an idea to facilitate more playback options. Clients that deem incomplete data to be OK could receive only occassional updates during fast forward or search operations. Certain clients are apt to be more sensitive to the complete data set, specifically algorithms that maintain state information. These would be set to receive the complete data set and would also be prevented from seeing non-monotonic data.</t>
<t tx="ntallen.20070525114357">/dev/huarp/$Experiment/TM/DG write only
/dev/huarp/$Experiment/TM/DCf read only
/dev/huarp/$Experiment/TM/DCo read only
</t>
<t tx="ntallen.20070525114357.1">We can initialized the MD list immediately, and each client OCB can refer to the one active empty MD and indicated that 0 TimeStamps have been processed.
Until we hear from the DG, we don't know how much buffer space to allocate in the data queue.</t>
<t tx="ntallen.20070525114357.2">When the initial configuration arrives, we can make decisions about what data format to use for output and hence what buffer dimensions to use.

If nrowminf == 1 &amp;&amp; mfc_lsb == 0 &amp;&amp; mfc_msb == 1, use T3
else if nrowminf &gt; 1, use T2
else use T1

Incoming data may always come in in T1 or T2, so we'll need to be able to translate between formats

DG initialization also includes the starting TimeStamp, so we can initialize the TS list and kick all of the client OCBs</t>
<t tx="ntallen.20070525114357.3">Data is stored in rows (i.e. smallest storage unit is a row) Actual storage size depends on TM format and choice of output format. T3  rows are shorter by 4 bytes. Data is identified by Qrow, which runs from 0 to max_Qrows-1.</t>
<t tx="ntallen.20070525114357.4">Metadata records aka MD aka dq_descriptor

Starting coordinate {
  metadata_serial_number: [might not be necessary]
  starting_Qrow: Which Qrow contains the first row of data for this block
  ref_count: How many OCBs are pointing to this record
  TS_serial_number: starts at 1
  n_Qrows: How many Qrows in this block. May wrap in queue
  MFCtr, Row: What is the MFCtr and Row index for the starting Qrow of data
  Qrows_expired: How many Qrows have been expired from this metatdata block
}

TimeStamps don't expire until all the data they pertain to expire. That way new clients can always receive the TimeStamp for the oldest data on record

A single metatdata record can be used for an extended period of time. The Qrows_expired field is used to note that rows have been expired. In each client OCB, a record is kept of which metatdata block is current and how many rows within that block have been delivered.
A Metadata record is active if it is the most recent metatdata record and hence might receive additional data

A metadata record is empty if it is inactive and all of its data records have expired
  A MD can only be empty if all preceeding MD records are empty as well

TimeStamp Metadata records can be expired when no longer referenced by active or non-empty MD
  or perhaps we just merge old MD records so new clients can note that they did not see the
  start of data.

An empty, inactive, unreferenced MD can expire. Will only actually expire it if it is first in DQD_Queue, but will skip over empty inactive dqd's on read. Could theoretically be merged into the preceeding empty MD record by adding its number of expired rows to the preceeding MD's, but the only point would be to count rows and let incoming clients know that they have missed some data. (Note that no current clients would know what to do with that information.) Could instead maintain a global expired rows count.

When a MD becomes empty (as data rows are expired) reduce ref count on associated TS. TS will expire if unreferenced.
When OCB points to new MD, update ref counts. If old MD is now empty and unreferenced, do merge check.

Before the first actual TM data arrives, there will be no queued timestamps or data records.
When the tm_info_t arrives, we can create the first timestamp record and the first MD record which references it. The MD will have n_Qrows of zero indicating that MFC, rownum, etc are not yet defined.

[We could either create a starting dq_descriptor with a flag indicating it is uninitialized, or we could wait to create it until it can be initialized (when the first data arrives) The former approach means initialization takes place in two places for no particular gain.
Well, one gain might be that we can immediately reference the current TS through the current MD, and we can immediately transmit tm_info_t. Actually, we need the 'uninitialized MD' flag whenever a new timestamp arrives, since we don't know what MFC, etc. will arrive. If n_Qrows is zero, we can modify the MFC, rownum, etc.]

When first data arrives, initialize first MD, visit any queued OCBs and link them into this record
</t>
<t tx="ntallen.20070525114357.5">next
TS_serial_number
unixtime
MFCtr
</t>
<t tx="ntallen.20070525114357.6">Which Metadata block: store as pointer
How many rows have already been processed?
Next pointer for list of pending requests

We don't advance to another metadata block until there *is* another metadata block to advance to. Even if we've processed all of the current block's data, it might be augmented in the future.

When we first start, there might not be even one metadata record, so no OCBs will point to anything.
</t>
<t tx="ntallen.20070525132754">Need provision for handling partial frame requests
Need to be able to buffer the header and at least one row so a data request for 1 byte can buffer the output data
These allocations cannot occur before the DG is initialized. But of course the DG interface needs to be ready to accept the tm_info_t structure, potentially as a partial frame. First record must be tm_info_t, so we could read it directly into a tm_hdr_t and then into tm_info.</t>
<t tx="ntallen.20070529132018">Need to differentiate between nodes
Need to maintain a list of pending reads. Should I do it here, or globally? Ideally should handle highest priority first.</t>
<t tx="ntallen.20070529132018.1">Just need to make sure only one open is allowed.</t>
<t tx="ntallen.20070529132018.2">On the writer, we need a scratch buffer for interpreting incoming messages, and we may as well use the partial buffer for this purpose.

I don't necessarily need to buffer an entire tm_msg_t. For example, if the tm_msg_t actually exceeds the size of the data queue, that wouldn't make sense. I expect that the underlying OS functionality will break a big message up into smaller messages. We need to use the partial buffer to handle this.

Partial buffer should be big enough to hold the tm_hdr_t, the largest data header (T2), and one row of data.

The smallest tm_msg_t is 10 bytes (Tstamp).
The first 6 bytes include all the information we need to determine the total message size[, but rather than waste time on multiple syscalls, read in up to 64 bytes (tm_info_t), since that will include all the info we need to decide how to proceed. Might want to modify that number upwards depending on nbrow. If it turns out most frames arrive in, say, 65 bytes, we might want to go ahead and get the whole thing.] Only need to read tm_info_t once at the beginning, so it is foolish to optimize this operation for that case. Instead, optimize for incoming data. Could adjust based on the data type actually being used (i.e. DG is probably going to use the same data type throughout, so once we've received one data block, we can guess what the future data blocks will hold.)

Rego: tm_info_t must be the first type read and will never recur, so don't need to use it as a benchmark
12 bytes will include all header data without trying to retrieve any data. Data will go elsewhere, so there is little advantage in reading it here.

Read in up to 12 bytes. If at least 4 have been read, we can check if we have all the header data we need:
    TM_TYPE_INIT: 64
    TM_TYPE_TSTAMP: 10
    TM_TYPE_DATA_T1: 6
    TM_TYPE_DATA_T2: 10
    TM_TYPE_DATA_T3: 8
    TM_TYPE_DATA_T4: 12
If we don't, note the partial frame. If we do, proceed with processing
INIT and TSTAMP processing is straightforward, since all the data is present in the message.
For Data messages, determine the actual message size and compare it to the write size


</t>
<t tx="ntallen.20070529133623">In this state, we are waiting to receive a complete header. The state information includes how many bytes are in the partial header (and may be zero) We can read up to 10 bytes if they are available. Some decisions can be made with fewer bytes read, but there is very little advantage in doing so, since no message will be less than 10 bytes.</t>
<t tx="ntallen.20070529133623.1">In this state, we have received a complete header, and may have received some data or not. All complete rows have already been entered into the data queue. Any partial row is stored in the partial buffer. The size and destination of the initial read is calculated based on the current status.

If partial_data + write_size &gt;= nbrow, prepare to write data into the data queue. Copy partial data into beginning of target and read remaining fraction plus whole rows from the message.</t>
<t tx="ntallen.20070531154309">When read request arrives, determine whether it can be handled immediately. If not, block.
Whenever data arrives, check queued read requests to see if they can be satisfied. If so,
process them.

If tm_info has not been written yet, OCB will not have a dq_descriptor
</t>
<t tx="ntallen.20070531162708">Reading data into tm_info. This should only be used if we're getting tm_info in tiny bits.

process_tm_info();
As soon as tm_info has been received, we can decide what data format to output, how much buffer space to allocate and in what configuration. We can then create the first timestamp record (with the TS in the tm_info) and the first dq_descriptor, albeit with no Qrows, but refrencing the the first timestamp. Then we can check to see if any readers are waiting, and initialize them.

T1: Complete minor frames
T2: Complete rows
T3: Truncated rows

if ( mfc_lsb == 0 &amp;&amp; mfc_msb == 1 &amp;&amp; nrowminf == 1 ) use T3
else if ( nrowminf == 1 ) use T1
else use T2</t>
<t tx="ntallen.20070531163245"></t>
<t tx="ntallen.20070531163245.1">Actually may not need states per se.

If there is partial data present ( ocb-&gt;part.nbdata ) then send it
else If dqd is not set, we need to transmit the TMTYPE_INIT record
else If current dqd is done and next dqd has a new TS, transmit the new TS
else transmit from the current dqd

</t>
<t tx="ntallen.20070531163245.2">Store the size of the header we are transmitting (nbhdr) and the current offset. Use the OCB's tm_hdrs_t struct. Also buffer the data portion of the record in the buffer and set nbdata.

If offset is zero, we're just starting {
  Initialize the tm_hdrs_t and set nbhdr and nbdata
  If the entire request can be satisfied immediately {
    set up the buffers to send the data, and leave offset at zero and state at Partial Header
    return
  } else {
    copy the data into the partial buffer (and initialize it if necessary)
  }
}
set up buffers to send partial data and possibly switch to Partial Data state
</t>
<t tx="ntallen.20070531163245.3">I think this state will only be used for writer, or maybe it will be eliminated altogether.

Used when transmitting the initial tm_info struct(? how is the tstamp handled?)
</t>
<t tx="ntallen.20070531163245.4">Only used when transmitting a single row. All larger requests will be processed in integral numbers of rows.
</t>
<t tx="ntallen.20070601102213">As discussed elsewhere, in realtime operation TMbfr will not handle any commands. It will forward all data that is written, and it will shutdown gracefully when the write stream is closed. By 'gracefully' I mean it will signal EOF to all readers, and won't actually shut down until all of the reader streams are also closed.

In playback or remote operation, it will monitor commands from the command server for data regulation, but it will not accept a QUIT command via that route because it is difficult to signal back to the DG that the channel should be closed. Instead, the DG should monitor its own command channel for QUIT.

What if the DG never starts up? cat /dev/null &gt;/dev/huarp/Exp/TM/DG
</t>
<t tx="ntallen.20070611133034"></t>
<t tx="ntallen.20070611133034.1">Copy straight in. T1 output does not include calculation of MFCtr, so there are no requirements for consecutive frames.</t>
<t tx="ntallen.20070611133034.2">Should be a straight copy, but need to verify MFCtr, Row_num
T2 output does require that frames be consecutive, so we need to read in the rows and then check them to make sure they belong with the previous records. Before we copy, we can check the header, of course.</t>
<t tx="ntallen.20070611133034.3">Need to go row-by-row, verify MFCtr</t>
<t tx="ntallen.20070611133034.4">Not likely. Could be illegal.</t>
<t tx="ntallen.20070611133034.5">Copy straight in, then verify continuity with previous records</t>
<t tx="ntallen.20070611133034.6">Not likely, could be illegal (i.e. T2 only applies when nrowminf &gt; 1)</t>
<t tx="ntallen.20070611133034.7">Won't happen</t>
<t tx="ntallen.20070611133034.8">Won't happen</t>
<t tx="ntallen.20070611133034.9">Copy straight in. Verify consecutive, etc.</t>
</tnodes>
</leo_file>
