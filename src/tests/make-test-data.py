import random 
count = 0
with open('test.txt', 'wb') as f:
    for x in range(0, 5):
        offset = random.gauss(1000, 500)
        for x in range(count, count + 5000):
            line = "%d %d" % (x, offset + random.gauss(0, 200))
            f.write((line + "\n").encode('ascii'))
        count += 5000
