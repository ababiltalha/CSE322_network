#!/bin/bash
node_default=$((40))
flow_default=$((20))
packet_default=$((200))
coverage_default=$((1))
# speed_default=$((10))
unmodified=$((0))
modified=$((1))

filename="wireless_802_11_static"

node_result_file="results/node.out"
flow_result_file="results/flow.out"
packet_result_file="results/packet.out"
coverage_result_file="results/coverage.out"

node_result_file_mod="results/node-modified.out"
flow_result_file_mod="results/flow-modified.out"
packet_result_file_mod="results/packet-modified.out"
coverage_result_file_mod="results/coverage-modified.out"

# $node_default $flow_default $coverage_default $packet_default

# # Varying nodes
>$node_result_file
>$node_result_file_mod
for i in 20 40 60 80 100
do
    echo "node $i " >> $node_result_file
    echo "node $i " >> $node_result_file_mod
    ns "$filename.tcl" $unmodified $i $flow_default $coverage_default $packet_default 
    awk -f $filename.awk trace.tr >> $node_result_file
    ns "$filename.tcl" $modified $i $flow_default $coverage_default $packet_default 
    awk -f $filename.awk trace.tr >> $node_result_file_mod
done

# # Varying flows
>$flow_result_file
>$flow_result_file_mod
for i in 10 20 30 40 50
do
    echo "flow $i " >> $flow_result_file
    echo "flow $i " >> $flow_result_file_mod 
    ns "$filename.tcl" $unmodified $node_default $i $coverage_default $packet_default 
    awk -f $filename.awk trace.tr >> $flow_result_file
    ns "$filename.tcl" $modified $node_default $i $coverage_default $packet_default 
    awk -f $filename.awk trace.tr >> $flow_result_file_mod
done



# # Varying coverage area
>$coverage_result_file
>$coverage_result_file_mod
for i in 1 2 3 4 5
do
    echo "coverage $i " >> $coverage_result_file
    echo "coverage $i " >> $coverage_result_file_mod
    ns "$filename.tcl" $unmodified $node_default $flow_default $i $packet_default 
    awk -f $filename.awk trace.tr >> $coverage_result_file
    ns "$filename.tcl" $modified $node_default $flow_default $i $packet_default 
    awk -f $filename.awk trace.tr >> $coverage_result_file_mod
done


# # Varying packet rate
>$packet_result_file
>$packet_result_file_mod
for i in 100 200 300 400 500
do
    echo "packet $i " >> $packet_result_file
    echo "packet $i " >> $packet_result_file_mod
    ns "$filename.tcl" $unmodified $node_default $flow_default $coverage_default $i  
    awk -f $filename.awk trace.tr >> $packet_result_file
    ns "$filename.tcl" $modified $node_default $flow_default $coverage_default $i  
    awk -f $filename.awk trace.tr >> $packet_result_file_mod
done

python3 $filename.py $node_result_file $node_result_file_mod "node"
python3 $filename.py $flow_result_file $flow_result_file_mod "flow"
python3 $filename.py $coverage_result_file $coverage_result_file_mod "coverage"
python3 $filename.py $packet_result_file $packet_result_file_mod "packet"