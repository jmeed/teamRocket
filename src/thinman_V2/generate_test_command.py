import random
import binascii

TEST_DATA_SIZE = 512

data_set = bytearray()

for i in range(TEST_DATA_SIZE):
    data_set.append(random.randrange(0, 256))

print binascii.b2a_hex(str(data_set))
print "------"

print "writw 0"

for i in range(0, len(data_set), 5):
    print ' '.join('%02x' % (b,) for b in data_set[i:i+5])

print 'z'