import matplotlib.pyplot as plt

outfiles = ["area.out", "nodes.out", "flows.out"]
dimlist = [250, 500, 750, 1000, 1250]
nnlist = [20, 40, 60, 80, 100]
nflist = [10, 20, 30, 40, 50]
throughput = []
delays = []
delivery = []
drop = []


labelylist = ["Throughput", "Delay", "Delivery", "Drop"]
labelxlist = ["Area Side Size (m)", "Number of Nodes", "Number of Flows"]


def saveGraph(paramlist, labelx):
    plt.plot(paramlist, throughput, marker="o", color="b")
    plt.ylabel("Network Throughput (bits/sec)")
    plt.xlabel(labelx)
    plt.grid(True)
    plt.savefig("graphs/"+labelylist[0]+" vs "+labelx+".png")
    plt.close()

    plt.plot(paramlist, delays, marker="o", color="b")
    plt.ylabel("End-to-end Delay (sec)")
    plt.xlabel(labelx)
    plt.grid(True)
    plt.savefig("graphs/"+labelylist[1]+" vs "+labelx+".png")
    plt.close()

    plt.plot(paramlist, delivery, marker="o", color="b")
    plt.ylabel("Packet Delivery Ratio")
    plt.xlabel(labelx)
    plt.grid(True)
    plt.savefig("graphs/"+labelylist[2]+" vs "+labelx+".png")
    plt.close()

    plt.plot(paramlist, drop, marker="o", color="b")
    plt.ylabel("Packet Drop Ratio")
    plt.xlabel(labelx)
    plt.grid(True)
    plt.savefig("graphs/"+labelylist[3]+" vs "+labelx+".png")
    plt.close()


filename = (outfiles[0])
throughput = []
delays = []
delivery = []
drop = []
outfile = open(filename, "r")
for line in outfile:
    tokens = line.split()
    throughput.append(float(tokens[0]))
    if (tokens[1] != "-nan"):
        delays.append(float(tokens[1]))
    else:
        delays.append(0)
    delivery.append(float(tokens[2]))
    drop.append(float(tokens[3]))
outfile.close()
saveGraph(dimlist, labelxlist[0])

filename = (outfiles[1])
throughput = []
delays = []
delivery = []
drop = []
outfile = open(filename, "r")
for line in outfile:
    tokens = line.split()
    throughput.append(float(tokens[0]))
    if (tokens[1] != "-nan"):
        delays.append(float(tokens[1]))
    else:
        delays.append(0)
    delivery.append(float(tokens[2]))
    drop.append(float(tokens[3]))
outfile.close()
saveGraph(nnlist, labelxlist[1])

filename = (outfiles[2])
throughput = []
delays = []
delivery = []
drop = []
outfile = open(filename, "r")
for line in outfile:
    tokens = line.split()
    throughput.append(float(tokens[0]))
    if (tokens[1] != "-nan"):
        delays.append(float(tokens[1]))
    else:
        delays.append(0)
    delivery.append(float(tokens[2]))
    drop.append(float(tokens[3]))
outfile.close()
saveGraph(nflist, labelxlist[2])





