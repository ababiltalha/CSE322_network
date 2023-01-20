#Create a simulator object
set ns [new Simulator]

#Define different colors for data flows (for NAM)
$ns color 1 Blue
$ns color 2 Red

#Open the NAM file and trace file
set nam_file [open animation.nam w]
set trace_file [open trace.tr w]
$ns namtrace-all $nam_file
$ns trace-all $trace_file

#Define a 'finish' procedure
proc finish {} {
    global ns nam_file trace_file
    $ns flush-trace 
    #Close the NAM trace file
    close $nam_file
    close $trace_file
    #Execute NAM on the trace file
    # exec nam out.nam &
    exit 0
}

#Create four nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]

#Create links between the nodes
# ns <link-type> <node1> <node2> <bandwidht> <delay> <queue-type-of-node2>
$ns duplex-link $n0 $n2 2Mb 10ms DropTail
$ns duplex-link $n1 $n2 2Mb 10ms DropTail
$ns duplex-link $n2 $n3 2Mb 10ms DropTail
$ns duplex-link $n4 $n3 2Mb 10ms DropTail
$ns duplex-link $n2 $n4 2Mb 10ms DropTail

#Set Queue Size of link (n2-n3) to 10
# $ns queue-limit $n2 $n3 20

#Give node position (for NAM)
#$ns duplex-link-op $n0 $n2 orient right-down
#$ns duplex-link-op $n1 $n2 orient right-up
#$ns duplex-link-op $n2 $n3 orient right

#Monitor the queue for link (n2-n3). (for NAM)
$ns duplex-link-op $n2 $n3 queuePos 0.5


#Setup a TCP connection
#Setup a flow
set tcp1 [new Agent/TCP]
$ns attach-agent $n0 $tcp1
set sink [new Agent/TCPSink]
$ns attach-agent $n3 $sink
$ns connect $tcp1 $sink
$tcp1 set fid_ 1



#Setup a FTP Application over TCP connection
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1
$ftp1 set type_ FTP

#Setup another TCP connection
#Setup a flow
set tcp2 [new Agent/TCP]
$ns attach-agent $n4 $tcp2
set sink [new Agent/TCPSink]
$ns attach-agent $n1 $sink
$ns connect $tcp2 $sink
$tcp2 set fid_ 2



#Setup a FTP Application over TCP connection
set ftp2 [new Application/FTP]
$ftp2 attach-agent $tcp2
$ftp2 set type_ FTP



# #Setup a UDP connection
# #Setup another flow
# set udp [new Agent/UDP]
# $udp set class_ 2
# $ns attach-agent $n4 $udp
# set null [new Agent/Null]
# $ns attach-agent $n1 $null
# $ns connect $udp $null
# $udp set fid_ 2

# #Setup a CBR over UDP connection
# set cbr [new Application/Traffic/CBR]
# $cbr attach-agent $udp
# $cbr set type_ CBR
# $cbr set packet_size_ 1000
# $cbr set rate_ 1mb
# $cbr set random_ false

#Schedule events for the CBR and FTP agents
$ns at 0.1 "$ftp2 start"
$ns at 0.1 "$ftp1 start"
$ns at 4.5 "$ftp1 stop"
$ns at 4.5 "$ftp2 stop"

#Detach tcp and sink agents (not really necessary)
$ns at 4.5 "$ns detach-agent $n0 $tcp1 ; $ns detach-agent $n3 $sink"
$ns at 4.5 "$ns detach-agent $n4 $tcp2 ; $ns detach-agent $n1 $sink"


#Call the finish procedure after 5 seconds of simulation time
$ns at 5.0 "finish"


#Run the simulation
$ns run