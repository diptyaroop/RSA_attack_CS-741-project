import sys

lines = []
window_size = int(sys.argv[1]) // 2
with open(sys.argv[2], 'r') as my_file:
	for line in my_file:
		lines.append(line.strip().replace("\n",""))
i=0
end = len(lines)-2
i=0
count=9
flag=False
for i in reversed(range(len(lines)-2)):
	if(lines[i] != "r g" and lines[i]!= "s g"):
		if flag:
			break
		end = i-1
		count=0
		
		continue
	if(lines[i] == "s g" and lines[i+1] != "r g"):
		if flag:
			break
		end = i-1
		count=0
		continue
	if(lines[i]=="r g"):
		count+=1
	#print(i,lines[i],count)
	if(count==window_size):
		
		if(lines[i]!="r g" or lines[end]!="r g"):
			end = i-1
		else:
			flag=True
			

#i+=2
#print(i)
#j=496
output=""
#start+=1
i = i+1
while(lines[i]=="s g"):
	i+=1
count=0
while count<window_size:
	#print(lines[i])
	temp = lines[i]
	output = output + temp.replace(" ",",")+","
	i+=1	
	if(temp=="r g"):
		count+=1

f = open("trace.txt", "w")
f.write(output)
f.close()
