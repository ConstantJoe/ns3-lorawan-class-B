import sys
from operator import itemgetter

netdevice = []
enddevice = []
ns = []
with open(sys.argv[1], 'r') as f:
	# skip the first bit
	line = f.readline()
	while (line != "RESULTS START HERE\n"):
		if line == "":
			print("Err: no results in this file.")
			sys.exit()
		line = f.readline()

	numNodes = f.readline()

	line = f.readline()
	while line != "":
		w = line.split('\t')
		l = len(w)

		if l == 5:
			# push into netdevice list
			netdevice.append(w)
		elif l == 6:
			# push into enddevice list
			enddevice.append(w)
		elif l == 11:
			# push into network server list
			ns.append(w)
		line = f.readline()

#print(netdevice)

netdevice.sort(key=lambda x: int(x[0]))
ns.sort(key=lambda x: int(x[0]))

#print(netdevice)

#sorted(netdevice, key=itemgetter(0))

#print(netdevice)



#sorted(ns, key=itemgetter(0), reverse=True)

#for x in enddevice:
#	print(x)

##for x in ns:
#	print(x) 

with open("out" + str(sys.argv[2])  + ".txt", 'a') as f:
	f.write(str(len(enddevice)) +  "\n")
	f.write("ID\tUS_A\tDS_RW1\tDS_RW2\tDS_B\tDS_Bea\t\tID\tDS_A_Gen\tDS_Sent\tDS_RW1\tDS_RW2\tDS_Retrans\tDS_Acks\tDS_Bea\tDS_B_Gen\tDS_B\tUS_A\t\tID\tTX_Busy\tTX_DC\tRX_Bea\tRX_DL\n")

	for x in range(len(enddevice)):
		for idy, y in enumerate(enddevice[x]):
			if idy == len(enddevice[x]) -1:
				y = y[:len(y)-1] #remove the newline
                                f.write("%s" % y)
                        else:   
                                f.write("%s\t" % y)
		f.write("\t\t")
		for idy, y in enumerate(ns[x]):
                        if idy == len(ns[x]) -1:
				y = y[:len(y)-1] #remove the newline
                                f.write("%s" % y)
                        else:
                                f.write("%s\t" % y)
		f.write("\t\t")
		for idy, y in enumerate(netdevice[x]):
                        if idy == len(netdevice[x]) -1:
				y = y[:len(y)-1] #remove the newline
                                f.write("%s" % y)
                        else:
                                f.write("%s\t" % y)
		f.write("\n")
	#add last netdevice line
	f.write(" \t \t \t \t \t \t\t \t \t \t \t \t \t \t \t \t \t\t\t")
	for idy, y in enumerate(netdevice[len(enddevice)]):
        	if idy == len(netdevice[len(enddevice)]) -1:
                	y = y[:len(y)-1] #remove the newline
                        f.write("%s" % y)
                else:
                       	f.write("%s\t" % y)
        f.write("\n")
	f.write("\n")
