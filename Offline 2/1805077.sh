echo "shell file running"
rm graphs/*
rm area.out nodes.out flows.out trace.tr animation.nam temp.txt
touch area.out
touch nodes.out
touch flows.out
touch temp.txt
for i in {250..1250..250}
do
    echo " ### Varied parameters are nodes: 40 flows: 20 area size: $i x $i ###"
    ns 1805077.tcl 40 20 $i >> temp.txt
    awk -f 1805077.awk trace.tr >> area.out
    echo "" >> temp.txt
done
# python .py
for i in {20..100..20}
do
    echo " ### Varied parameters are nodes: $i flows: 20 area size: 500 x 500 ###"
    ns 1805077.tcl $i 20 500 >> temp.txt
    awk -f 1805077.awk trace.tr >> nodes.out
    echo "" >> temp.txt
done
for i in {10..50..10}
do
    echo " ### Varied parameters are nodes: 40 flows: $i area size: 500 x 500 ###"
    ns 1805077.tcl 40 $i 500 >> temp.txt
    awk -f 1805077.awk trace.tr >> flows.out
    echo "" >> temp.txt
done
# mkdir graphs
python 1805077.py

rm area.out nodes.out flows.out trace.tr animation.nam temp.txt
