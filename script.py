import math
import sys

KEY_SIZE = int(sys.argv[1]) // 2
with open(sys.argv[2], 'r') as my_file:
    n = int(my_file.readline().split(" ")[1],16)
    e = int(my_file.readline().split(" ")[1],16)
    b=e
#KEY_SIZE = 14
#e = 17
#b = e
#n = 110176103
p = -1
q = -1
def generate_trace_output(page_trace):
    function_trace = []
    for i in range(len(page_trace)):
        if(page_trace[i]!='g'):
            function_trace.append(page_trace[i])
    branch_trace = []
    trace_index = 0
    i= 0
    while i < KEY_SIZE and  trace_index < len(function_trace):
        # for branch 1: subtract, rshift
        if function_trace[trace_index] == 's' and function_trace[trace_index+1] == 'r':
            branch_trace.append("branch1")
            trace_index = trace_index + 2
            #print(i, " branch1")
        elif function_trace[trace_index] == 'r':
            branch_trace.append("branch3")
            trace_index = trace_index + 1
            #print(i, " branch3")
        i = i + 1

    return branch_trace

def get_key(branch_trace):
    swap_iteration =  KEY_SIZE - int(math.log(e)/math.log(2))
    a_num = 1
    a_den = 1
    b_num = 0
    b_den = 1
    flag = False
    for i in range(swap_iteration+2):
        if(branch_trace[i]=="branch1"):
            a_den = a_den * 2
            b_num = b_num + b_den
            b_den = b_den * 2
        elif(branch_trace[i]=="branch3"):
            a_den = a_den*2
            if(b_num!=0):
                b_den = b_den*2
        #print("equation: ", "a/", a_den, "-", b_num, "b/", b_den)
        check_iterations = [swap_iteration-2,swap_iteration-1,swap_iteration,swap_iteration+1,swap_iteration+2]
        #print("iteration =", i)
        if i in check_iterations:
            #print("iteration_check =", i)
            #print("check equation: ", "a = ", a_den, "* num + ", (a_den/b_den)*b_num, "b")
            for num in range(1, b+1):
                a = int(a_den*num)+ int(a_den/b_den)* b_num * b
                if math.gcd(a+1, n)>1:
                    flag = True
                    p = a+1
                    q = n//p
                    if(q>p):
                        tmp=q
                        q=p
                        p=tmp
                    print("Key found: \n******\np:", hex(p).upper(), " \n******\nq:", hex(q).upper(), "\n******\n")
                    return
    if flag == False:
        print("Key is not found")


trace_file = open("trace.txt", 'r').read()
page_trace = trace_file.split(',')
branch_trace = generate_trace_output(page_trace)
get_key(branch_trace)
