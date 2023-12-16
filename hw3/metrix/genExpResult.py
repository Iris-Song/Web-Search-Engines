# caculate precision
precision_file = open("precision.txt")
p_arr = [[] for _ in range(7)]

while True:
    line = precision_file.readline()
    if not line:
        break
    p_list = [int(x) for x in line.split()]
    for i,num in enumerate(p_list):
        if num==-1:
            continue
        p_arr[i].append(num)
precision_file.close()

print('precision')
for i,lst in enumerate(p_arr):
    print(i+1, sum(lst)/len(lst), len(lst))

# caculate recall
recall_file = open("recall.txt")
r_arr = [[] for _ in range(7)]

while True:
    line = recall_file.readline()
    if not line:
        break
    r_list = [int(x) for x in line.split()]
    for i,num in enumerate(r_list):
        if num==-1:
            continue
        r_arr[i].append(num)
recall_file.close()

print('recall')
for i,lst in enumerate(r_arr):
    print(i+1, sum(lst)/len(lst), len(lst))

# 263,373,374,381,391