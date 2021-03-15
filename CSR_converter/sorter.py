# 테스트용
# graph_path = "./"
# graph_name = "reddit-adjlist_index"
# comment_num = 0

# reddit
# graph_path = "/nfs/home/seonbinara/dataset/GRIP_Reddit (GraphSAGE)/reddit/"
# graph_name = "reddit-adjlist_index"
# comment_num = 0

# livejournal
# graph_path = "/nfs/home/seonbinara/dataset/GRIP_Livejournal/"
# graph_name = "com-lj.ungraph"
# comment_num = 1

# youtube
# graph_path = "/nfs/home/seonbinara/dataset/GRIP_Youtube/"
# graph_name = "com-youtube.ungraph"
# comment_num = 1

# pokec
# graph_path = "/nfs/home/seonbinara/dataset/GRIP_Pokec/"
# graph_name = "soc-pokec-relationships"
# comment_num = 0

# epinion
# graph_path = "./"
# graph_name = "soc-Epinions1"
# comment_num = 4

# drug_drug
# graph_path = "/nfs/home/seonbinara/codes/GCN_cache_test_multiway/data/"
# graph_name = "drug_drug"
# comment_num = 4

# protein_protein
# graph_path = "/nfs/home/seonbinara/dataset/OGN_proteinprotein/"
# graph_name = "protein_protein"
# comment_num = 4

# wiki-topcats
graph_path = "/nfs/home/seonbinara/dataset/wiki-topcats/"
graph_name = "wiki-topcats"
comment_num = 0

file_name = graph_path + graph_name + ".txt"
opt_file_name = graph_path + graph_name + "_sorted.txt"

reader = open(file_name, encoding='utf8')
writer = open(opt_file_name, 'w', encoding='utf8')

for comment in range(comment_num):
    reader.readline()

adj_dict = dict()
for line in reader:
    frm, to = (int(s) for s in line.split())
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
