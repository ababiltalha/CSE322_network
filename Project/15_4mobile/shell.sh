filename="wireless_802_15_4_mobile"

# ns filename.tcl modified nodes flows speed packets
ns $filename.tcl 1 40 20 5 200
awk -f $filename.awk trace.tr > temp.out
