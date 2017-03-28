import sqlite3
import sys
import xml.etree.ElementTree as ET

if len(sys.argv) < 2:
    print "You must provide xml file's name as first argument, and database file's name as second argument"
    sys.exit(-1)

conn = sqlite3.connect(sys.argv[2])
c = conn.cursor()
c.execute("CREATE TABLE clusters (clustid int,template text,goodness real,AvgLen real)")
c.execute("CREATE TABLE syslog (clustid int,msg text)")
file_contents = "<file>\n"
with open(sys.argv[1], 'r') as file_obj:
    file_contents += file_obj.read()
file_contents += "</file>"
root = ET.fromstring(file)
for ActClust in root:
    ActTemplate = ""
    for ActToken in ActClust:
        ActTemplate = ActTemplate + ActToken.get('value')+" "

    values = "(%s, %s, %s, %s)" % (ActClust.get('id'), ActTemplate, ActClust.get('goodness').replace(",", "."),
            ActClust.get('AvgLen'))
    c.execute("INSERT INTO clusters VALUES " + values)

conn.commit()
conn.close()
