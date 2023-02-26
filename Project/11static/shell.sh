filename="wireless_802_11_static"
ns $filename.tcl 1 80 20 2 200
awk -f $filename.awk trace.tr > temp.out
