#!/bin/bash
node_default=$((40))
flow_default=$((20))
packet_default=$((200))
speed_default=$((10))
unmodified=$((0))
modified=$((1))

filename="wireless_802_15_4_mobile"

node_result_file="results/node.out"
flow_result_file="results/flow.out"
packet_result_file="results/packet.out"
speed_result_file="results/speed.out"

node_result_file_mod="results/node-modified.out"
flow_result_file_mod="results/flow-modified.out"
packet_result_file_mod="results/packet-modified.out"
speed_result_file_mod="results/speed-modified.out"

# $node_default $flow_default $speed_default $packet_default

# Varying nodes
>$node_result_file
>$node_result_file_mod
for i in 20 40 60 80 100
do
    echo "node $i " >> $node_result_file
    echo "node $i " >> $node_result_file_mod
    ns "$filename.tcl" $unmodified $i $flow_default $speed_default $packet_default 
    awk -f $filename.awk trace.tr >> $node_result_file
    ns "$filename.tcl" $modified $i $flow_default $speed_default $packet_default 
    awk -f $filename.awk trace.tr >> $node_result_file_mod
done

# Varying flows
>$flow_result_file
>$flow_result_file_mod
for i in 10 20 30 40 50
do
    echo "flow $i " >> $flow_result_file
    echo "flow $i " >> $flow_result_file_mod 
    ns "$filename.tcl" $unmodified $node_default $i $speed_default $packet_default 
    awk -f $filename.awk trace.tr >> $flow_result_file
    ns "$filename.tcl" $modified $node_default $i $speed_default $packet_default 
    awk -f $filename.awk trace.tr >> $flow_result_file_mod
done



# Varying speed
>$speed_result_file
>$speed_result_file_mod
for i in 5 10 15 20 25
do
    echo "speed $i " >> $speed_result_file
    echo "speed $i " >> $speed_result_file_mod
    ns "$filename.tcl" $unmodified $node_default $flow_default $i $packet_default 
    awk -f $filename.awk trace.tr >> $speed_result_file
    ns "$filename.tcl" $modified $node_default $flow_default $i $packet_default 
    awk -f $filename.awk trace.tr >> $speed_result_file_mod
done


# Varying packet rate
>$packet_result_file
>$packet_result_file_mod
for i in 100 200 300 400 500
do
    echo "packet $i " >> $packet_result_file
    echo "packet $i " >> $packet_result_file_mod
    ns "$filename.tcl" $unmodified $node_default $flow_default $speed_default $i 
    awk -f $filename.awk trace.tr >> $packet_result_file
    ns "$filename.tcl" $modified $node_default $flow_default $speed_default $i 
    awk -f $filename.awk trace.tr >> $packet_result_file_mod
done

python3 $filename.py $node_result_file $node_result_file_mod "node"
python3 $filename.py $flow_result_file $flow_result_file_mod "flow"
python3 $filename.py $speed_result_file $speed_result_file_mod "speed"
python3 $filename.py $packet_result_file $packet_result_file_mod "packet"