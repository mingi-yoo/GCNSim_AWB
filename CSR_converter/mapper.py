# 인덱스가 숫자가 아니고
# adj list 형태로 되어있는 경우

# 테스트용
# graph_path = "./"
# graph_name = "reddit-adjlist"
# comment_num = 3

# reddit
graph_path = "/nfs/home/seonbinara/dataset/GRIP_Reddit (GraphSAGE)/reddit/"
graph_name = "reddit-adjlist"
comment_num = 3

file_name = graph_path + graph_name + ".txt"
opt_file_name = graph_path + graph_name + "_index.txt"

reader = open(file_name, encoding='utf8')
writer = open(opt_file_name, 'w', encoding='utf8')

for comment in range(comment_num):
    reader.readline()

node_num = 0
node_dict = dict()
for line in reader:
    for node in line.split():
        if node not in node_dict:
            node_dict[node] = node_num
            node_num += 1

reader.close() # 닫고 다시 열자

reader = open(file_name, encoding='utf8')

for comment in range(comment_num):
    reader.readline()

for line in reader:
    frm = int(node_dict[line.split()[0]])
    idx = 0
    for node in line.split():
        if idx != 0:
            writer.write(str(frm))
            writer.write(" ")
            writer.write(str(node_dict[node]))
            writer.write("\n")
            idx += 1
        else:
            idx += 1

reader.close()
writer.close()