# simulator
set ns [new Simulator]
expr { srand(47) }
set dropRate 0.025
# ======================================================================
# Define options

set val(chan)         Channel/WirelessChannel       ;# channel type
set val(prop)         Propagation/TwoRayGround      ;# radio-propagation model
set val(ant)          Antenna/OmniAntenna           ;# Antenna type
set val(ll)           LL                            ;# Link layer type
set val(ifq)          Queue/DropTail/PriQueue       ;# Interface queue type
set val(ifqlen)       50                            ;# max packet in ifq
set val(netif)        Phy/WirelessPhy               ;# network interface type # Phy/WirelessPhy # Phy/WirelessPhy/802_15_4
set val(mac)          Mac/802_11                    ;# MAC type # Mac/802_11 # Mac/802_15_4
set val(rp)           DSDV                          ;# ad-hoc routing protocol 
set val(modify)       [lindex $argv 0]
set val(nn)           [lindex $argv 1]              ;# number of mobilenodes def 40
set val(nf)           [lindex $argv 2]              ;# number of flows def 20
set val(ntx_range)    [lindex $argv 3]              ;# multiple of default tx_range (coverage area) def 1 
set val(pktrate)      [lindex $argv 4]              ;# packet rate def 200
set val(dim)          500
set val(energymodel_11)     EnergyModel             ;# Energy Model
set val(initialenergy_11)   1000                    ;# Initial energy in Joules
set val(idlepower_11)       900e-3                  ;# Stargate (802.11b)
set val(rxpower_11)         925e-3                  ;# Stargate (802.11b)
set val(txpower_11)         1425e-3                 ;# Stargate (802.11b)
set val(sleeppower_11)      300e-3                  ;# Stargate (802.11b)
# Agent/TCP/WestwoodNR set maxcwnd_ [expr {$val(pktrate) * 0.04}]
Agent/TCP/WestwoodNR set modified $val(modify)
set nowValue [Phy/WirelessPhy set Pt_]
set newValue_Pt [expr {$nowValue * $val(ntx_range) * $val(ntx_range)}]
Phy/WirelessPhy set Pt_ $newValue_Pt
# =======================================================================

# trace file
set trace_file [open trace.tr w]
$ns trace-all $trace_file
# $ns use-newtrace

# nam file
# set nam_file [open animation.nam w]
# $ns namtrace-all-wireless $nam_file $val(dim) $val(dim)

# topology: to keep track of node movements
set topo [new Topography]
$topo load_flatgrid $val(dim) $val(dim)

proc UniformErr {} {
    global dropRate
    set err [new ErrorModel/Uniform $dropRate pkt]
    return $err
}


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
                -energyModel $val(energymodel_11) \
                -idlePower $val(idlepower_11) \
                -rxPower $val(rxpower_11) \
                -txPower $val(txpower_11) \
                -sleepPower $val(sleeppower_11) \
                -initialEnergy $val(initialenergy_11)\
                -movementTrace OFF
                # -IncomingErrProc UniformErr\

# create nodes
# Node positioning : grid
for {set i 0} {$i < $val(nn)} {incr i} {
    set node($i) [$ns node]
    $node($i) random-motion 0       ;# disable random motion
    $node($i) set X_ [expr int($i / 10) * $val(dim) / 50]
    $node($i) set Y_ [expr int($i % 10) * $val(dim) / 50]
    $node($i) set Z_ 0
    $ns initial_node_pos $node($i) 20
}


# Traffic
# set src [expr (int(rand() * 1000) % $val(nn))]

for {set i 0} {$i < $val(nf)} {incr i} {
    set src [expr (int(rand() * 1000) % $val(nn))]
    set dest [expr (int(rand() * 1000) % $val(nn))]
    while {$src == $dest} {
        set dest [expr (int(rand() * 1000) % $val(nn))]
    }
    # Traffic config
    # create agent
    set tcp [new Agent/TCP/WestwoodNR]
    set tcp_sink [new Agent/TCPSink]
    # attach to nodes
    $ns attach-agent $node($src) $tcp
    $ns attach-agent $node($dest) $tcp_sink
    # connect agents
    $ns connect $tcp $tcp_sink
    $tcp set fid_ $i
    $tcp set maxseq_ $val(pktrate)

    # Traffic generator
    set ftp [new Application/FTP]
    # attach to agent
    $ftp attach-agent $tcp
    
    # start traffic generation
    $ns at 1.0 "$ftp start"

}
# End Simulation

# Stop nodes
for {set i 0} {$i < $val(nn)} {incr i} {
    $ns at 50.0 "$node($i) reset"
}

# call final function
proc finish {} {
    global ns trace_file
    #  nam_file
    $ns flush-trace
    close $trace_file
    # close $nam_file
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



