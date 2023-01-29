echo "shell file running"
touch area.out
touch nodes.out
touch flows.out
touch temp.txt
for i in {250..1250..250}
do
    echo "Area size: $i" >> area.out
    echo " ### Varied parameters are nodes: 40 flows: 20 area size: $i x $i ###"
    ns 1805077.tcl 40 20 $i >> temp.txt
    awk -f parse.awk trace.tr >> area.out
done
for i in {20..100..20}
do
    echo "Nodes: $i" >> nodes.out
    echo " ### Varied parameters are nodes: $i flows: 20 area size: 500 x 500 ###"
    ns 1805077.tcl $i 20 500 >> temp.txt
    awk -f parse.awk trace.tr >> nodes.out
done
for i in {10..50..10}
do
    echo "Flows: $i" >> flows.out
    echo " ### Varied parameters are nodes: 40 flows: $i area size: 500 x 500 ###"
    ns 1805077.tcl 40 $i 500 >> temp.txt
    awk -f parse.awk trace.tr >> flows.out
done
# rm area.out nodes.out flows.out trace.tr animation.nam temp.txt
