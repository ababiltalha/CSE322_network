# simulator
set ns [new Simulator]


# ======================================================================
# Define options

set val(chan)         Channel/WirelessChannel       ;# channel type
set val(prop)         Propagation/TwoRayGround      ;# radio-propagation model
set val(ant)          Antenna/OmniAntenna           ;# Antenna type
set val(ll)           LL                            ;# Link layer type
set val(ifq)          Queue/DropTail/PriQueue       ;# Interface queue type
set val(ifqlen)       50                            ;# max packet in ifq
set val(netif)        Phy/WirelessPhy               ;# network interface type # Phy/WirelessPhy # Phy/WirelessPhy/802_15_4
set val(mac)          Mac/802_11                  ;# MAC type # Mac/802_11 # Mac/802_15_4
set val(rp)           DSDV                          ;# ad-hoc routing protocol 
set val(nn)           [lindex $argv 0]              ;# number of mobilenodes def 40
set val(nf)           [lindex $argv 1]              ;# number of flows def 20
set val(dim)          [lindex $argv 2]              ;# dimension of the area def 500
# =======================================================================

# trace file
set trace_file [open trace.tr w]
$ns trace-all $trace_file

# nam file
set nam_file [open animation.nam w]
$ns namtrace-all-wireless $nam_file $val(dim) $val(dim)

# topology: to keep track of node movements
set topo [new Topography]
$topo load_flatgrid $val(dim) $val(dim)


# general operation director for mobilenodes
create-god $val(nn)

# node configs
# ======================================================================

# $ns node-config -addressingType flat or hierarchical or expanded
#                  -adhocRouting   DSDV or DSR or TORA
#                  -llType	   LL
#                  -macType	   Mac/802_11
#                  -propType	   "Propagation/TwoRayGround"
#                  -ifqType	   "Queue/DropTail/PriQueue"
#                  -ifqLen	   50
#                  -phyType	   "Phy/WirelessPhy"
#                  -antType	   "Antenna/OmniAntenna"
#                  -channelType    "Channel/WirelessChannel"
#                  -topoInstance   $topo
#                  -energyModel    "EnergyModel"
#                  -initialEnergy  (in Joules)
#                  -rxPower        (in W)
#                  -txPower        (in W)
#                  -agentTrace     ON or OFF
#                  -routerTrace    ON or OFF
#                  -macTrace       ON or OFF
#                  -movementTrace  ON or OFF

# ======================================================================

$ns node-config -adhocRouting $val(rp) \
                -llType $val(ll) \
                -macType $val(mac) \
                -ifqType $val(ifq) \
                -ifqLen $val(ifqlen) \
                -antType $val(ant) \
                -propType $val(prop) \
                -phyType $val(netif) \
                -topoInstance $topo \
                -channelType $val(chan) \
                -agentTrace ON \
                -routerTrace ON \
                -macTrace OFF \
                -movementTrace OFF

# create nodes
# Node positioning : random
for {set i 0} {$i < $val(nn) } {incr i} {
    set node($i) [$ns node]
    $node($i) random-motion 0       ;# disable random motion
    $node($i) set X_ [expr (int(rand() * 1000) % $val(dim))]
    $node($i) set Y_ [expr (int(rand() * 1000) % $val(dim))]
    $node($i) set Z_ 0
    $ns initial_node_pos $node($i) 20
} 

# Traffic
# 1 Sink, Random Source
set dest [expr (int(rand() * 1000) % $val(nn))]

for {set i 0} {$i < $val(nf)} {incr i} {
    set src [expr (int(rand() * 1000) % $val(nn))]
    while {$src == $dest} {
        set src [expr (int(rand() * 1000) % $val(nn))]
    }

    # Traffic config
    # create agent
    set udp_agent [new Agent/UDP]
    # attach to nodes
    $ns attach-agent $node($src) $udp_agent
    # Traffic generator
    set exp_traffic [new Application/Traffic/Exponential objects]
    # attach to agent
    $exp_traffic attach-agent $udp_agent
    # set MSS to 536 B
    # $udp_agent set packetSize_ 536
    
    set null_agent [new Agent/Null]
    $ns attach-agent $node($dest) $null_agent
    # connect agents
    $ns connect $udp_agent $null_agent
    $udp_agent set fid_ $i

    # start traffic generation
    $ns at 1.0 "$exp_traffic start"
}



# End Simulation

# Stop nodes
for {set i 0} {$i < $val(nn)} {incr i} {
    $ns at 50.0 "$node($i) reset"
}

# call final function
proc finish {} {
    global ns trace_file nam_file
    $ns flush-trace
    close $trace_file
    close $nam_file
}

proc halt_simulation {} {
    global ns
    puts "Simulation ending"
    $ns halt
}

$ns at 50.0001 "finish"
$ns at 50.0002 "halt_simulation"




# Run simulation
puts "Simulation starting"
$ns run



