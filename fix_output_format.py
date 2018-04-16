import sys

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

netdevice.sort(key=lambda x: x[0])
ns.sort(key=lambda x: x[0])



#for x in enddevice:
#	print(x)

##for x in ns:
#	print(x) 

with open("out" + numNodes[:len(numNodes)-1] + ".txt", 'w') as f:
	for x in netdevice:
		for idy, y in enumerate(x):
			if idy == len(x) -1:
				f.write("%s" % y)
			else:	
				f.write("%s " % y)
	f.write("\n")

	for x in enddevice:
		for idy, y in enumerate(x):
			if idy == len(x) -1:
				f.write("%s" % y)
			else:	
				f.write("%s " % y)
	f.write("\n")

	for x in ns:
		for idy, y in enumerate(x):
			if idy == len(x) -1:
				f.write("%s" % y)
			else:	
				f.write("%s " % y)

