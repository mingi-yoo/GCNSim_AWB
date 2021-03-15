# 정렬된 from - to 파일용!!

# ogb-product
graph_path = "/nfs/home/seonbinara/dataset/OGB_product/"
graph_name = "ogb_product_sorted"
comment_num = 0

file_name = graph_path + graph_name + ".txt"
opt_file_name = graph_path + graph_name + "_csr.txt"

row = []
row.append(0)
col = []

reader = open(file_name, encoding='utf8')
writer = open(opt_file_name, 'w', encoding='utf8')

for comment in range(comment_num):
    reader.readline()

row_identifier = 0
row_num = 0
past_row_num = 0
max_val = 0 # 최대 인덱스 얻는 용
total_row = 1
for line in reader:
    frm, to = (int(s) for s in line.split())
    if(max_val <= frm):
        max_val = frm
    if(max_val <= to):
        max_val = to
    if(frm == row_identifier):
        row_num += 1
    else:
        if (frm - row_identifier) >= 2:
            for i in range(frm - row_identifier - 1):
                row.append(row_num)
                total_row += 1
        # 첫번째 경우에는 row에 0으로 시작
        row.append(row_num)
        row_num += 1
        row_identifier = frm
        total_row += 1
    col.append(to) # for column

# 마지막 row에 대한 처리가 안된 상태이므로
row.append(row_num)
for i in range(max_val - total_row):
    row.append(row_num)

# 파일 출력
for el in row:
    writer.write(str(el))
    writer.write(" ")

writer.write("\n")

for el in col:
    writer.write(str(el))
    writer.write(" ")

reader.close()
writer.close()

print("max_index is... ", max_val)
