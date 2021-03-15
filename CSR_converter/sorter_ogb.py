
# 0bg_product
graph_path = "/nfs/home/seonbinara/dataset/OGB_product/"
graph_name = "ogb_product"
comment_num = 0

file_name = graph_path + "unprocessed/edge" + ".csv"
opt_file_name = graph_path + graph_name + "_sorted.txt"

reader = open(file_name, encoding='utf8')
writer = open(opt_file_name, 'w', encoding='utf8')

for comment in range(comment_num):
    reader.readline()

adj_dict = dict()
for line in reader:
    frm, to = (int(s) for s in line.split(","))
    if frm not in adj_dict:
        adj_dict[frm] = set()
        adj_dict[frm].add(to)
    else:
        adj_dict[frm].add(to)

for key in sorted(adj_dict):
    for to in sorted(list(adj_dict[key])):
        writer.write(str(key))
        writer.write(" ")
        writer.write(str(to))
        writer.write("\n")

reader.close()
writer.close()
