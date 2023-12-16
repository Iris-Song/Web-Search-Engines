from openai import OpenAI
import time

# load document table
doc_table_file = open("../BIN_docTable.dt")
doc_offset = []
while True:
    line = doc_table_file.readline()
    if not line:
        break
    _,_,_,_,_,offset = line.split()
    doc_offset.append(int(offset))
doc_table_file.close()

# client = OpenAI()
client = OpenAI(
    api_key="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",
)

# read file
result_file = open("../result/fake_result20.txt")
begini = 380

query = result_file.readline()
print(query)
# ignore
result_file.readline()

line = result_file.readline()
docID_list = []
while line:
    _, _, docID = line.split()
    docID_list.append(int(docID))
    line = result_file.readline()
result_file.close()


dataset_file = open("../msmarco-docs.trec")
for i, docID in enumerate(docID_list[::-1]):
    dataset_file.seek(doc_offset[docID], 0)
    text = ""
    while True:
        line = dataset_file.readline()
        if "</TEXT>" in line:
            break
        text += line
    
    user_content = "query:" + query + "\n text:" + text
    completion = client.chat.completions.create(
        model="gpt-3.5-turbo-1106",
        messages=[
            {"role": "system", "content": "You should select snippet from a given text based on words in query\
             . The snippet should no more than 30 terms or 200 characters"},
            {"role": "user", "content": user_content[:16300]}
        ]
    )

    resfile = open("../snippet/snippet_"+str(begini+i)+".txt","a") 
    resfile.write(completion.choices[0].message.content)
    resfile.close()
    print(completion.choices[0].message.content)
    print()
    # openai limitation
    time.sleep(20)

dataset_file.close()
